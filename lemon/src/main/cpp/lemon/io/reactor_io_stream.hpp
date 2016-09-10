/**
 *
 * @file     io_object_iocp
 * @brief    Copyright (C) 2015  yayanyang All Rights Reserved
 * @author   yayanyang
 * @date     2015/12/07
 */
#ifndef LEMON_IO_REACTOR_IO_STREAM_HPP
#define LEMON_IO_REACTOR_IO_STREAM_HPP

#include <functional>

#include <lemon/io/buff.hpp>
#include <lemon/io/reactor_op.hpp>
#include <lemon/io/reactor_io_object.hpp>
#include <lemon/io/reactor_io_service.hpp>

namespace lemon{
    namespace io{

        template <typename Callback>
        class reactor_read_op : public reactor_op
        {
        public:
            reactor_read_op(int fd,buffer buff,Callback && callback)
                    :reactor_op(
                    (reactor_op::action_f)reactor_read_op::read_action,
                    (reactor_op::complete_f)reactor_read_op::read_complete)
                    ,_fd(fd)
                    ,_buff(buff)
                    ,_callback((callback))
                    ,_trans(0)
            {
               
            }

        private:

            static bool read_action (reactor_read_op* op)
            {
                ssize_t length = ::read(op->_fd,op->_buff.data,op->_buff.length);

                if(length == -1)
                {
                    if(errno == EAGAIN || errno == EWOULDBLOCK)
                    {
                        return false;
                    }

                    op->_ec = std::error_code(errno,std::system_category());

                    return true;
                }

                op->_trans = (size_t)length;

                return true;
            }

            static void read_complete (reactor_read_op* op)
            {
                op->_callback(op->_trans,op->_ec);
            }

        private:
            int                     _fd;
            buffer                  _buff;
            Callback                _callback;
            size_t                  _trans;
            std::error_code         _ec;
        };


        template <typename Callback>
        class reactor_read_complete_op : public reactor_op
        {

        public:
            reactor_read_complete_op(Callback callback,size_t length,std::error_code ec)
                    :reactor_op(
                    nullptr,
                    (reactor_op::complete_f)reactor_read_complete_op::read_complete)
                    ,_callback(callback)
                    ,_trans(length)
                    ,_ec(ec)
            {

            }

        private:
            static void read_complete (reactor_read_complete_op* op)
            {
                op->_callback(op->_trans,op->_ec);
            }

            Callback                        _callback;
            size_t                          _trans;
            std::error_code                 _ec;
        };


        class reactor_io_stream : public reactor_io_object
        {
        public:
            reactor_io_stream(reactor_io_service & service,handler fd)
                    :reactor_io_object(service,fd)
            {

            }

            template <typename Callback>
            void read(buffer buff,Callback && callback)
            {
                ssize_t length = ::read(get(),buff.data,buff.length);

                if(length == -1)
                {
                    if(errno == EAGAIN || errno == EWOULDBLOCK)
                    {
                        auto op = new reactor_read_op<Callback>(get(),buff,std::forward<Callback>(callback));

                        std::lock_guard<reactor_io_service> lock(service());

                        push_read_op(op);
                    }
                    else
                    {
                        auto op = new reactor_read_complete_op<Callback>(
                                std::forward<Callback>(callback),
                                0,
                                std::error_code(errno,std::system_category()));

                        service().complete(op);
                    }
                }
                else
                {

                    auto op = new reactor_read_complete_op<Callback>(std::forward<Callback>(callback),(size_t)length,std::error_code());

                    service().complete(op);
                }
            }

            template <typename Callback>
            void write(const_buffer buff,Callback && callback)
            {

            }
        };

        using io_stream = reactor_io_stream;
    }
}

#endif //LEMON_IO_REACTOR_IO_STREAM_HPP