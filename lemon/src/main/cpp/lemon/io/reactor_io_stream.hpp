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
        class reactor_write_op : public reactor_op
        {
        public:
            reactor_write_op(int fd,const_buffer buff,Callback && callback)
                    :reactor_op(
                    (reactor_op::action_f)reactor_write_op::write_action,
                    (reactor_op::complete_f)reactor_write_op::write_complete)
                    ,_fd(fd)
                    ,_buff(buff)
                    ,_callback((callback))
                    ,_trans(0)
            {

            }

        private:

            static bool write_action (reactor_write_op* op)
            {
                ssize_t length = ::write(op->_fd,op->_buff.data,op->_buff.length);

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

            static void write_complete (reactor_write_op* op)
            {
                op->_callback(op->_trans,op->_ec);
            }

        private:
            int                     _fd;
            const_buffer            _buff;
            Callback                _callback;
            size_t                  _trans;
            std::error_code         _ec;
        };




        class reactor_io_stream : public reactor_io_object
        {
        public:
            reactor_io_stream(reactor_io_service & service,handler fd)
                    :reactor_io_object(service,fd)
            {

            }

            template <typename Callback>
            void read(buffer buff,Callback && callback, std::error_code & ec)
            {
                auto op = std::unique_ptr<reactor_accept_op<Callback>>(new reactor_read_op<Callback>((int)get(),buff,std::forward<Callback>(callback)));

                service().push_read_op(this,op.get(),ec);

                if(ec)
                {
                    return;
                }

                op.release();
            }

            template <typename Callback>
            void write(const_buffer buff,Callback && callback, std::error_code & ec)
            {
                auto op = std::unique_ptr<reactor_accept_op<Callback>>(new reactor_write_op<Callback>((int)get(),buff,std::forward<Callback>(callback)));

                service().push_read_op(this,op.get(),ec);

                if(ec)
                {
                    return;
                }

                op.release();
            }
        };

        using io_stream = reactor_io_stream;
    }
}

#endif //LEMON_IO_REACTOR_IO_STREAM_HPP