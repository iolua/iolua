/**
 * @file     io_service
 * @brief    Copyright (C) 2015  yayanyang All Rights Reserved 
 * @author   yayanyang
 * @date     2015/12/07
 */
#ifndef LEMON_IO_IO_SERVICE_HPP
#define LEMON_IO_IO_SERVICE_HPP
#if defined(__linux)
#include <lemon/io/reactor_io_service_epoll.hpp>
#endif

#if defined(__APPLE__)
#include <lemon/io/reactor_io_service_kqueue.hpp>
#endif

#if defined(WIN32)
#include <lemon/io/iocp_io_service.hpp>
#endif //WIN32

#endif //LEMON_IO_IO_SERVICE_HPP