/**
 *
 * @file     io_operation
 * @brief    Copyright (C) 2015  yayanyang All Rights Reserved
 * @author   yayanyang
 * @date     2015/12/07
 */
#ifndef LEMON_IO_REACTOR_IO_SERVICE_EPOLL_HPP
#define LEMON_IO_REACTOR_IO_SERVICE_EPOLL_HPP

#include <string.h>
#include <unistd.h>
#include <sys/epoll.h>

#include <vector>

#include <lemon/io/reactor_io_service.hpp>

namespace lemon{
    namespace io{

        class  reactor_io_service_epoll final : public reactor_io_service
        {
        public:
            reactor_io_service_epoll()
            {
                _epoll = epoll_create(1);

                if(-1 == _epoll)
                {
                    throw std::system_error(errno,std::system_category());
                }

                start();
            }

            ~reactor_io_service_epoll()
            {
                ::close(_epoll);
            }

        private:

            void register_io_service(handler fd, std::error_code & ec) final
            {
                struct epoll_event event = {EPOLLIN | EPOLLOUT | EPOLLRDHUP | EPOLLPRI | EPOLLERR | EPOLLHUP | EPOLLET};

                event.data.fd = fd;

                if( -1 == epoll_ctl(_epoll, EPOLL_CTL_ADD, fd, &event) )
                {
                    ec = std::error_code(errno,std::system_category());
                }

            }

            void unregister_io_service(handler fd) final
            {
                struct epoll_event event = {EPOLLIN | EPOLLOUT | EPOLLRDHUP | EPOLLPRI | EPOLLERR | EPOLLHUP | EPOLLET};

                event.data.fd = fd;

                if( -1 == epoll_ctl(_epoll, EPOLL_CTL_DEL, fd, &event) )
                {

                }
            }

            std::size_t io_events_wait(io_event* events,std::size_t max) final
            {

                std::vector<epoll_event> epoll_events(max);

                for(;;)
                {
                    int ret = epoll_wait(_epoll, &epoll_events[0], (int)epoll_events.size(), 10);

                    if(ret == -1)
                    {
                        if (errno == EINTR) continue;

                        return 0;
                    }

                    if(ret == 0)
                    {
                        continue;
                    }

                    for ( int i = 0; i < ret; i ++ )
                    {
                        if((epoll_events[i].events & EPOLLIN)!=0)
                        {
                            events[i].ops |= (int)io_event_op::read;
                        }


                        if((epoll_events[i].events & EPOLLOUT)!=0)
                        {
                            events[i].ops |= (int)io_event_op::read;
                        }

                        if(events[i].ops != (int)io_event_op::none)
                        {
                            events[i].fd = epoll_events[i].data.fd;
                        }
                    }

                    return (size_t)ret;
                }

            }

        private:
            int                             _epoll;
        };

        typedef  reactor_io_service_epoll io_service;
    }
}


#endif //LEMON_IO_REACTOR_IO_SERVICE_EPOLL_HPP