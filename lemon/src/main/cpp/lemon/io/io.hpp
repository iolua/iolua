#ifndef LEMON_IO_HPP
#define LEMON_IO_HPP


#include <lemon/io/handler.hpp>
#include <lemon/io/sockaddr.hpp>
#include <lemon/io/io_errors.hpp>
#include <lemon/io/io_socket.hpp>
#include <lemon/io/io_service.hpp>

#ifdef WIN32
#include <lemon/io/iocp_io_pipe.hpp>
#include <lemon/io/iocp_io_stream.hpp>
#else

#include <lemon/io/reactor_io_pipe.hpp>
#include <lemon/io/reactor_io_stream.hpp>

#endif //WIN32



#endif //LEMON_IO_HPP