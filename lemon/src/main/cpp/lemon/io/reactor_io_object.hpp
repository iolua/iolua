/**
 *
 * @file     reactor_io_object
 * @brief    Copyright (C) 2015  yayanyang All Rights Reserved
 * @author   yayanyang
 * @date     2015/12/07
 */
#ifndef LEMON_IO_REACTOR_IO_OBJECT_HPP
#define LEMON_IO_REACTOR_IO_OBJECT_HPP


#include <unistd.h>
#include <system_error>
#include <lemon/nocopy.hpp>
#include <lemon/io/handler.hpp>
#include <lemon/io/reactor_op.hpp>

namespace lemon{
    namespace io{

        class reactor_io_object;
        class reactor_io_service;

        enum class io_event_op
        {
            none = 0, read = 1,write = 2
        };

        void reactor_io_service_register(
                reactor_io_service& service,
                reactor_io_object *obj,
                std::error_code & ec
        ) noexcept;

        void reactor_io_service_unregister(
                reactor_io_service& service,
                reactor_io_object *obj
        ) noexcept;

        /**
         * reactor io object
         */
        class reactor_io_object : private nocopy
        {
        public:
            reactor_io_object(reactor_io_service & service,handler fd)
                    :_handler(fd)
                    ,_service(service)
                    ,_read_header(nullptr)
                    ,_read_tail(nullptr)
                    ,_write_header(nullptr)
                    ,_write_tail(nullptr)
            {
                std::error_code ec;
                reactor_io_service_register(service,this,ec);

                if (ec)
                {
                    throw std::system_error(ec);
                }
            }

            virtual ~reactor_io_object()
            {
                reactor_io_service_unregister(_service,this);

                ::close(_handler);
            }

            handler get() const
            {
                return _handler;
            }

            reactor_op* process_one_op(io_event_op event_op)
            {
                reactor_op* op = nullptr;

                if(event_op == io_event_op::read) op = front_read_op();
                else op = front_write_op();

                if(op == nullptr ) return nullptr;

                if(op->action())
                {
                    if(event_op == io_event_op::read) {
                        pop_read_op();
                    }
                    else pop_write_op();

                    return op;
                }

                return nullptr;
            }


            reactor_io_service & service()
            {
                return _service;
            }

        protected:

            void push_read_op(reactor_op* op)
            {
                push(&_read_header,&_read_tail,op);
            }

            reactor_op* front_read_op() const
            {
                return _read_header;
            }

            void pop_read_op()
            {
                pop(&_read_header,&_read_tail);
            }

            void push_write_op(reactor_op* op)
            {
                push(&_write_header,&_write_tail,op);
            }

            reactor_op* front_write_op() const
            {
                return _write_header;
            }

            void pop_write_op()
            {
                pop(&_write_header,&_write_tail);
            }


        private:

            static void push(reactor_op **header,reactor_op**tail, reactor_op *irp) noexcept
            {
                if(*header == nullptr)
                {
                    *header = *tail = irp;

                    return;
                }

                (*tail)->next = irp;

                *tail = irp;
            }

            static void pop(reactor_op ** header, reactor_op**tail) noexcept
            {

                auto irp = *header;

                if(irp ==nullptr) return;

                *header = irp->next;

                if(*header == nullptr) *tail = nullptr;

            }

        private:
            handler                 _handler;
            reactor_io_service      &_service;
            reactor_op              *_read_header;
            reactor_op              *_read_tail;
            reactor_op              *_write_header;
            reactor_op              *_write_tail;
        };

    }
}

#endif //LEMON_IO_REACTOR_IO_OBJECT_HPP