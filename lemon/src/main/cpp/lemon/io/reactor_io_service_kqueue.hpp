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
#include <sys/event.h>

#include <vector>

#include <lemon/io/reactor_io_service.hpp>

namespace lemon{
    namespace io{

        class  reactor_io_service_kqueue final : public reactor_io_service
        {
        public:
            reactor_io_service_kqueue():_logger(log::get("io_service"))
            {

                _handler = kqueue();

                if(_handler == -1)
                {
                    throw std::system_error(errno,std::system_category());
                }

                start();
            }

            ~reactor_io_service_kqueue()
            {
                doclose();
            }

            void doclose() override
            {
                if(_handler != -1) {
                    ::close(_handler);

                    _handler = -1;
                }
            }

        private:



            void register_io_service(handler fd, std::error_code & ec) final
            {
                struct kevent changes[2];
                EV_SET(&changes[0], fd, EVFILT_READ, EV_ADD | EV_ENABLE | EV_CLEAR , 0, 0, NULL);
                EV_SET(&changes[1], fd, EVFILT_WRITE, EV_ADD | EV_ENABLE | EV_CLEAR , 0, 0, NULL);
                int ret = kevent(_handler, changes, sizeof(changes)/sizeof(struct kevent), NULL, 0, NULL);

                if(ret == -1)
                {
                    ec = std::error_code(errno,std::system_category());
                }

            }

            void unregister_io_service(handler fd) final
            {
                struct kevent changes[2];
                EV_SET(&changes[0], fd, EVFILT_READ, EV_DELETE, 0, 0, NULL);
                EV_SET(&changes[1], fd, EVFILT_WRITE, EV_DELETE, 0, 0, NULL);
                int ret = kevent(_handler, changes, sizeof(changes)/sizeof(struct kevent), NULL, 0, NULL);

                if(ret == -1)
                {

                }
            }

            std::size_t io_events_wait(io_event* events,std::size_t max) final
            {

                std::vector<struct kevent> k_events(max);

                struct timespec spec = { 0, 10000000 };

                for(;;)
                {

                    int ret = kevent(_handler,NULL,0, &k_events[0], (int)k_events.size(), &spec);

                    if(ret == -1)
                    {
                        return 0;
                    }

                    if(ret == 0)
                    {
                        continue;
                    }

                    for ( int i = 0; i < ret; i ++ )
                    {
                        lemonD(_logger,"fd(%d) filter %d",k_events[i].ident, k_events[i].filter)

                        if ((k_events[i].flags & EV_EOF) == EV_EOF)
                        {
                            lemonD(_logger,"fd(%d) EOF",k_events[i].ident)
                        }

                        if( k_events[i].filter == EVFILT_READ)
                        {
                            events[i].ops |= (int)io_event_op::read;

                            events[i].fd = k_events[i].ident;
                        }
                        else if( k_events[i].filter == EVFILT_WRITE)
                        {
                            events[i].ops |= (int)io_event_op::write;

                            events[i].fd = k_events[i].ident;
                        }
                    }

                    return (size_t)ret;
                }

            }

        private:
            int                             _handler;
            const lemon::log::logger        &_logger;
        };

        typedef  reactor_io_service_kqueue io_service;
    }
}


#endif //LEMON_IO_REACTOR_IO_SERVICE_EPOLL_HPP