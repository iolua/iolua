/**
 *
 * @file     io_operation
 * @brief    Copyright (C) 2015  yayanyang All Rights Reserved
 * @author   yayanyang
 * @date     2015/12/07
 */
#ifndef LEMON_IO_REACTOR_IO_SERVICE_HPP
#define LEMON_IO_REACTOR_IO_SERVICE_HPP

#include <mutex>
#include <chrono>
#include <thread>
#include <cstddef>
#include <functional>
#include <system_error>
#include <unordered_map>
#include <condition_variable>

#include <lemon/nocopy.hpp>
#include <lemon/io/handler.hpp>
#include <lemon/io/reactor_op.hpp>
#include <lemon/io/reactor_io_object.hpp>
#include <lemon/log/log.hpp>

namespace lemon{
    namespace io{

        struct io_event
        {
            handler             fd;

            std::error_code     ec;

            int                 ops; // indicate this a read operation
        };

        class reactor_io_service : private nocopy
        {
            friend void reactor_io_service_register(
                    reactor_io_service& service,
                    reactor_io_object *obj,
                    std::error_code & ec
            ) noexcept;

            friend void reactor_io_service_unregister(
                    reactor_io_service& service,
                    reactor_io_object *obj
            ) noexcept;
        public:

            reactor_io_service()
                    :_complete_header(nullptr)
                    ,_complete_tail(nullptr)
                    ,_logger(log::get("io_service"))
            {

            }

            virtual ~reactor_io_service()
            {
                if(_dispatcher.joinable()) _dispatcher.join();
            }

            void close()
            {
                doclose();

                _condition.notify_all();
            }

            virtual void doclose() = 0;

            void run_one(std::error_code & ec)
            {
                run_one(std::chrono::seconds(-1),ec);
            }

            template <typename _Rep, typename _Period>
            void run_one(std::chrono::duration<_Rep, _Period> timeout)
            {
                std::error_code ec;
                run_one(timeout,ec);

                if (ec&&ec != std::errc::timed_out)
                {
                    throw std::system_error(ec);
                }
            }

            template <typename _Rep, typename _Period>
            void run_one(std::chrono::duration<_Rep,_Period> timeout, std::error_code & ec)
            {
                std::unique_lock<std::mutex> lock(_mutex);

                std::cv_status status = std::cv_status::no_timeout;

                if(_complete_header ==nullptr)
                {
                    if(std::chrono::duration<_Rep,_Period>(-1) == timeout)
                    {
                        _condition.wait(lock);
                    }
                    else
                    {
                        status = _condition.wait_for(lock,timeout);
                    }
                }

                if(status == std::cv_status::timeout)
                {
                    ec = std::make_error_code(std::errc::timed_out);
                    return;
                }

                auto complete = _complete_header;

                if(complete)
                {
                    // pop the header complete handler
                    _complete_header = _complete_header->next;

                    if(_complete_tail == complete) {
                        _complete_tail = nullptr;
                    }

                    lock.unlock();

                    complete->complete();

                    delete complete;

                    return;
                }

                ec = make_error_code(errc::io_service_closed);
            }


            void push_read_op(reactor_io_object* obj, reactor_op* op, std::error_code & ec)
            {
                std::unique_lock<std::mutex> lock(_mutex);

                if( _handlers.count(obj->get()) == 0 )
                {
                    ec = make_error_code(errc::unregister_fd);
                    return;
                }

                obj->push_read_op(op);

                lemonD(_logger,"invoke io_read(%d)",obj->get());

                if(invoke_io_op(obj,io_event_op::read))
                {
                    lemonD(_logger,"invoke io_read(%d) -- success",obj->get());
                    _condition.notify_one();
                }

                lemonD(_logger,"invoke io_read(%d) -- wait",obj->get());
            }

            void push_write_op(reactor_io_object* obj, reactor_op* op, std::error_code & ec)
            {
                std::unique_lock<std::mutex> lock(_mutex);

                if( _handlers.count(obj->get()) == 0 )
                {
                    ec = make_error_code(errc::unregister_fd);
                    return;
                }

                obj->push_write_op(op);

                if(invoke_io_op(obj,io_event_op::write))
                {
                    _condition.notify_one();
                }
            }

        protected:

            void start()
            {
                _dispatcher = std::thread(std::bind(&reactor_io_service::process,this));
            }


            virtual std::size_t io_events_wait(io_event* events,std::size_t max) = 0;

            virtual void register_io_service(handler fd, std::error_code & ec) = 0;

            virtual void unregister_io_service(handler fd) = 0;

        private:


            bool invoke_io_op(reactor_io_object * obj,io_event_op event_op)
            {
                // try invoke io operation
                auto op = obj->process_one_op(event_op);
                
                if(op)
                {
                    if(_complete_header == nullptr)
                    {
                        _complete_tail = _complete_header = op;
                    }
                    else
                    {
                        _complete_tail->next = op;
                        _complete_tail = op;
                    }

                    return true;
                }

                return false;
            }

            void invoke_all_io_op(reactor_io_object * obj)
            {
                size_t completes = 0;
                for(;;)
                {
                    reactor_op* op = obj->front_read_op();

                    if(!op) break;

                    if(op->action())
                    {
                        completes ++;
                        if(_complete_header == nullptr)
                        {
                            _complete_tail = _complete_header = op;
                        }
                        else
                        {
                            _complete_tail->next = op;
                            _complete_tail = op;
                        }
                    }

                    obj->pop_read_op();
                }

                for(;;)
                {
                    reactor_op* op = obj->front_write_op();

                    if(!op) break;

                    if(op->action())
                    {
                        completes ++;

                        if(_complete_header == nullptr)
                        {
                            _complete_tail = _complete_header = op;
                        }
                        else
                        {
                            _complete_tail->next = op;
                            _complete_tail = op;
                        }
                    }

                    obj->pop_write_op();
                }

                for(size_t i = 0; i < completes; i ++ )
                {
                    _condition.notify_one();
                }
            }

            void process() noexcept
            {
                const static size_t max_events = 256;
                io_event events[max_events];
                std::size_t raised;

                while((raised = io_events_wait(events,max_events)) != 0)
                {
                    lemonD(_logger,"reactor_io_service process(%d) ...", raised)

                    std::unique_lock<std::mutex> lock(_mutex);

                    size_t completes = 0;

                    for(std::size_t i = 0; i < raised; i ++)
                    {
                        auto iter = _handlers.find(events[i].fd);

                        if(iter != _handlers.end())
                        {
                            if(events[i].ops & (int)io_event_op::read)
                            {
                                lemonD(_logger,"fd(%d) async invoke io_event_read", events[i].fd);

                                if(invoke_io_op(iter->second,io_event_op::read))
                                {
                                    lemonD(_logger,"fd(%d) async invoke io_event_read -- success", events[i].fd);
                                    completes++;
                                }
                                else
                                {
                                    lemonD(_logger,"fd(%d) async invoke io_event_read -- wait", events[i].fd);
                                }
                            }

                            if(events[i].ops & (int)io_event_op::write)
                            {
                                lemonD(_logger,"fd(%d) async invoke io_event_write", events[i].fd);
                                if(invoke_io_op(iter->second,io_event_op::write))
                                {
                                    lemonD(_logger,"fd(%d) async invoke io_event_write -- success", events[i].fd);
                                    completes++;
                                }
                                else
                                {
                                    lemonD(_logger,"fd(%d) async invoke io_event_write -- wait", events[i].fd);
                                }
                            }
                        }
                        else
                        {
                            lemonW(_logger,"reactor_io_service un-register fd(%d) event raised(%d)", events[i].fd,events[i].ops);
                        }
                    }

                    for(size_t i = 0; i < completes; i ++ )
                    {
                        _condition.notify_one();
                    }

                    lemonD(_logger,"reactor_io_service process completes(%d)",completes);
                }

                lemonD(_logger,"reactor_io_service process -- exit")
            }



        private:

            std::mutex                                              _mutex;

            std::condition_variable                                 _condition;

            std::thread                                             _dispatcher;

            std::unordered_map<handler,reactor_io_object*>          _handlers; // register objects

            reactor_op                                              *_complete_header; // the complete operation queue header

            reactor_op                                              *_complete_tail; // the complete operation queue tail

            const lemon::log::logger                                &_logger;
        };

        inline void reactor_io_service_register(
                reactor_io_service& service,
                reactor_io_object *obj,
                std::error_code & ec ) noexcept
        {
            lemonD(service._logger,"reactor_io_service register fd(%d)",obj->get());

            service.register_io_service(obj->get(),ec);

            if(ec) {
                lemonE(service._logger,"reactor_io_service register fd(%d) error: %s",obj->get(),ec.message().c_str());
                return;
            }

            std::lock_guard<std::mutex> lock(service._mutex);

            service._handlers[obj->get()] = obj;

            lemonD(service._logger,"reactor_io_service register fd(%d) -- success",obj->get());
        }

        inline void reactor_io_service_unregister(
                reactor_io_service& service,
                reactor_io_object *obj ) noexcept
        {

            lemonD(service._logger,"reactor_io_service un-register fd(%d)",obj->get());

            service.unregister_io_service(obj->get());

            std::lock_guard<std::mutex> lock(service._mutex);

            service._handlers.erase(obj->get());

            service.invoke_all_io_op(obj);
        }

    }
}


#endif //LEMON_IO_REACTOR_IO_SERVICE_HPP