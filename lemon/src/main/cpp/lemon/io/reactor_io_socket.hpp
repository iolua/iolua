/**
 *
 * @file     iocp_io_socket
 * @brief    Copyright (C) 2016  yayanyang All Rights Reserved
 * @author   yayanyang
 * @date     2016/02/18
 */
#ifndef LEMON_IO_REACTOR_IO_SOCKET_HPP
#define LEMON_IO_REACTOR_IO_SOCKET_HPP

#include <cerrno>
#include <memory>
#include <system_error>

#include <lemon/config.h>
#include <lemon/io/sockaddr.hpp>
#include <lemon/io/reactor_io_stream.hpp>

#include <fcntl.h>
#include <unistd.h>

#ifdef __MACH__
#define MSG_NOSIGNAL 0
#endif

namespace lemon{
    namespace io{

        const static int max_addr_buffer_length = 56;

        class reactor_io_socket;

        template <typename Callback>
        class reactor_accept_op : public reactor_op
        {
        public:
            reactor_accept_op(reactor_io_service & service,int fd,Callback && callback)
                    :reactor_op(
                    (reactor_op::action_f)reactor_accept_op::accept_action,
                    (reactor_op::complete_f)reactor_accept_op::read_complete)
                    ,_service(service)
                    ,_fd(fd)
                    ,_callback((callback))
            {

            }

        private:

            static bool accept_action (reactor_accept_op* op);

            static void read_complete (reactor_accept_op* op);

        private:
            reactor_io_service                      &_service;
            int                                     _fd;
            Callback                                _callback;
            std::error_code                         _ec;
            int                                     _conn;
            address                                 _addr;
        };

        template <typename Callback>
        class reactor_connect_op : public reactor_op
        {
        public:
            reactor_connect_op(int fd, const address & addr,Callback && callback)
                    :reactor_op(
                    (reactor_op::action_f)reactor_connect_op::connect_action,
                    (reactor_op::complete_f)reactor_connect_op::read_complete)
                    ,_fd(fd)
                    ,_addr(addr)
                    ,_callback((callback))
            {

            }

        private:

            static bool connect_action (reactor_connect_op* op)
            {

                if(connect(op->_fd,op->_addr,(socklen_t)op->_addr.length()) == -1)
                {
                    if (errno == EINPROGRESS || errno == EAGAIN)
                    {
                        return false;
                    }

                    if(errno != EISCONN)
                    {
                        op->_ec = std::error_code(errno,std::system_category());

                        return true;
                    }
                }

                return true;
            }

            static void read_complete (reactor_connect_op* op)
            {
                op->_callback(op->_ec);
            }

        private:
            address                                 _addr;
            int                                     _fd;
            Callback                                _callback;
            std::error_code                         _ec;
        };


        template <typename Callback>
        class reactor_recv_op : public reactor_op
        {
        public:
            reactor_recv_op(int fd, buffer buff,int flags,Callback && callback)
                    :reactor_op(
                    (reactor_op::action_f)reactor_recv_op::recv_action,
                    (reactor_op::complete_f)reactor_recv_op::read_complete)
                    ,_fd(fd)
                    ,_buff(buff)
                    ,_flags(flags)
                    ,_callback((callback))
            {

            }

        private:

            static bool recv_action (reactor_recv_op* op)
            {

                ssize_t recvbytes = recv(op->_fd,op->_buff.data,op->_buff.length,op->_flags | MSG_NOSIGNAL);
                if(recvbytes == -1)
                {
                    if (errno == EINPROGRESS || errno == EAGAIN)
                    {
                        return false;
                    }

                    op->_ec = std::error_code(errno,std::system_category());

                    return true;
                }

                op->_trans = (size_t)recvbytes;

                return true;
            }

            static void read_complete (reactor_recv_op* op)
            {
                op->_callback(op->_trans,op->_ec);
            }

        private:
            int                                     _fd;
            buffer                                  _buff;
            int                                     _flags;
            Callback                                _callback;
            std::error_code                         _ec;
            size_t                                  _trans;
        };


        template <typename Callback>
        class reactor_send_op : public reactor_op
        {
        public:
            reactor_send_op(int fd, const_buffer buff,int flags,Callback && callback)
                    :reactor_op(
                    (reactor_op::action_f)reactor_send_op::recv_action,
                    (reactor_op::complete_f)reactor_send_op::read_complete)
                    ,_fd(fd)
                    ,_buff(buff)
                    ,_flags(flags)
                    ,_callback((callback))
            {

            }

        private:

            static bool recv_action (reactor_send_op* op)
            {

                ssize_t recvbytes = send(op->_fd,op->_buff.data,op->_buff.length,op->_flags | MSG_NOSIGNAL);
                if(recvbytes == -1)
                {
                    if (errno == EINPROGRESS || errno == EAGAIN)
                    {
                        return false;
                    }

                    op->_ec = std::error_code(errno,std::system_category());

                    return true;
                }

                op->_trans = (size_t)recvbytes;

                return true;
            }

            static void read_complete (reactor_send_op* op)
            {
                op->_callback(op->_trans,op->_ec);
            }

        private:
            int                                     _fd;
            const_buffer                            _buff;
            int                                     _flags;
            Callback                                _callback;
            std::error_code                         _ec;
            size_t                                  _trans;
        };



        class reactor_io_socket : public reactor_io_object
        {

        public:

            reactor_io_socket(reactor_io_service & service,int fd)
                    :reactor_io_object(service, nio_socket(fd))
            {

            }

            reactor_io_socket(reactor_io_service & service, int af, int type, int protocol)
                    :reactor_io_object(service,create_socket(af,type,protocol))
                    ,_af(af)
                    ,_type(type)
                    ,_protocol(protocol)
            {

            }

            ~reactor_io_socket()
            {

            }

            void bind(const address & addr,std::error_code ec) noexcept
            {
                if (0 == ::bind((int)get(), addr, (socklen_t)addr.length()))
                {
                    ec = std::error_code(errno,std::system_category());
                }
            }

            void listen(int backlog, std::error_code ec) noexcept
            {
                if (0 == ::listen((int)get(), backlog))
                {
                    ec = std::error_code(errno, std::system_category());
                }
            }
            template <typename Callback>
            void accept(Callback && callback,std::error_code & ec) noexcept
            {
                auto op = std::unique_ptr<reactor_accept_op<Callback>>(new reactor_accept_op<Callback>(service(),(int)get(),std::forward<Callback>(callback)));

                if(op->action())
                {
                    service().complete(op.release());
                }
                else
                {
                    push_read_op(op.release());
                }
            }

            template <typename Callback>
            void connect(const address & addr, Callback && callback, std::error_code & ec) noexcept
            {
                auto op = std::unique_ptr<reactor_connect_op<Callback>>(new reactor_connect_op<Callback>((int)get(),addr,std::forward<Callback>(callback)));

                if(op->action())
                {
                    service().complete(op.release());
                }
                else
                {
                    push_write_op(op.release());
                }
            }

            template <typename Callback>
            void recv(buffer buff, int flags, Callback && callback)
            {
                auto op = std::unique_ptr<reactor_recv_op<Callback>>(new reactor_recv_op<Callback>((int)get(),buff,flags,std::forward<Callback>(callback)));

                if(op->action())
                {
                    service().complete(op.release());
                }
                else
                {
                    push_read_op(op.release());
                }
            }

            template <typename Callback>
            void send(const_buffer buff, int flags, Callback && callback)
            {
                auto op = std::unique_ptr<reactor_send_op<Callback>>(new reactor_send_op<Callback>((int)get(),buff,flags,std::forward<Callback>(callback)));

                if(op->action())
                {
                    service().complete(op.release());
                }
                else
                {
                    push_write_op(op.release());
                }
            }

        private:

            static handler nio_socket(int fd)
            {

#if defined(__APPLE__)
                int set = 1;

                if(0 != setsockopt(fd, SOL_SOCKET, SO_NOSIGPIPE, (void *)&set, sizeof(int)))
                {
                    close(fd);
                    throw std::system_error(errno,std::system_category());
                }
#endif //

                if(fcntl(fd, F_SETFL, fcntl(fd,F_GETFL,0) | O_NONBLOCK ) < 0)
                {
                    close(fd);
                    throw std::system_error(errno,std::system_category());
                }

                return (handler)fd;
            }


            static handler create_socket(int af, int type, int protocol)
            {

                auto fd = socket(af, type, protocol);

                if(-1 == fd)
                {
                    throw std::system_error(errno,std::system_category());
                }

                nio_socket(fd);

                return (handler)fd;
            }

        private:
            int								_af;
            int								_type;
            int								_protocol;

        };

        typedef reactor_io_socket io_socket;


        template <typename Callback>
        inline bool reactor_accept_op<Callback>:: accept_action (reactor_accept_op* op)
        {
            socklen_t len = max_addr_buffer_length;

            char buff[max_addr_buffer_length];

            int conn = accept(op->_fd,(sockaddr*)buff,&len);

            if(conn == -1)
            {
                if(errno == EAGAIN || errno == EWOULDBLOCK)
                {
                    return false;
                }

                op->_ec = std::error_code(errno,std::system_category());

                return true;
            }

            op->_addr = std::move(address((sockaddr*)buff,len));


            op->_conn = conn;

            return true;
        }

        template <typename Callback>
        inline void reactor_accept_op<Callback>::read_complete(io::reactor_accept_op<Callback> *op)
        {
            auto socket = std::unique_ptr<reactor_io_socket>(new reactor_io_socket(op->_service,op->_conn));

            op->_callback(socket,std::move(op->_addr),op->_ec);
        }
    }
}

#endif //LEMON_IO_IOCP_IO_SOCKET_HPP