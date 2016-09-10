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
        public:

            reactor_io_service()
                    :_complete_header(nullptr)
                    ,_complete_tail(nullptr)
            {

            }

            virtual ~reactor_io_service()
            {
                if(_dispatcher.joinable()) _dispatcher.join();
            }

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
                }
            }

            void notify_all()
            {
                _condition.notify_all();
            }

            void complete(reactor_op * op)
            {
                std::unique_lock<std::mutex> lock(_mutex);

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

            void lock()
            {
                _mutex.lock();
            }

            void unlock()
            {
                _mutex.unlock();
            }

        protected:

            friend void reactor_io_service_register(
                    reactor_io_service& service,
                    reactor_io_object *obj,
                    std::error_code & ec
            ) noexcept;

            friend void reactor_io_service_unregister(
                    reactor_io_service& service,
                    reactor_io_object *obj
            ) noexcept;


            virtual std::size_t io_events_wait(io_event* events,std::size_t max) = 0;

            virtual void register_io_service(handler fd, std::error_code & ec) = 0;

            virtual void unregister_io_service(handler fd) = 0;


            void start()
            {
                _dispatcher = std::thread(std::bind(&reactor_io_service::process,this));
            }

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

            void process() noexcept
            {
                const static size_t max_events = 256;
                io_event events[max_events];
                std::size_t raised;

                while((raised = io_events_wait(events,max_events)) != 0)
                {
                    std::unique_lock<std::mutex> lock(_mutex);

                    size_t completes = 0;

                    for(std::size_t i = 0; i < raised; i ++)
                    {
                        auto iter = _handlers.find(events[i].fd);

                        if(iter != _handlers.end())
                        {
                            if(events[i].ops & (int)io_event_op::read)
                            {
                                if(invoke_io_op(iter->second,io_event_op::read))
                                {
                                    completes++;
                                }
                            }

                            if(events[i].ops & (int)io_event_op::write)
                            {
                                if(invoke_io_op(iter->second,io_event_op::write))
                                {
                                    completes++;
                                }
                            }
                        }
                    }

                    if(completes != 0)
                    {
                        _condition.notify_all();
                    }
                }
            }



        private:

            std::mutex                                              _mutex;

            std::condition_variable                                 _condition;

            std::thread                                             _dispatcher;

            std::unordered_map<handler,reactor_io_object*>          _handlers; // register objects

            reactor_op                                              *_complete_header; // the complete operation queue header

            reactor_op                                              *_complete_tail; // the complete operation queue tail
        };

        inline void reactor_io_service_register(
                reactor_io_service& service,
                reactor_io_object *obj,
                std::error_code & ec ) noexcept
        {

            service.register_io_service(obj->get(),ec);

            if(ec) return;

            std::lock_guard<std::mutex> lock(service._mutex);

            service._handlers[obj->get()] = obj;
        }

        inline void reactor_io_service_unregister(
                reactor_io_service& service,
                reactor_io_object *obj ) noexcept
        {

            service.unregister_io_service(obj->get());

            std::lock_guard<std::mutex> lock(service._mutex);

            service._handlers.erase(obj->get());
        }

    }
}


#endif //LEMON_IO_REACTOR_IO_SERVICE_HPP