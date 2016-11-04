/**
 *
 * @file     pipe
 * @brief    Copyright (C) 2015  yayanyang All Rights Reserved
 * @author   yayanyang
 * @date     2015/12/07
 */
#ifndef LEMON_IO_REACTOR_IO_PIPE_HPP
#define LEMON_IO_REACTOR_IO_PIPE_HPP
#include <mutex>
#include <tuple>
#include <memory>
#include <sstream>

#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>



#include <lemon/uuid.hpp>
#include <lemon/nocopy.hpp>
#include <lemon/io/io_service.hpp>
#include <lemon/io/reactor_io_stream.hpp>

namespace lemon {
    namespace io{

        class reactor_io_pipe : private nocopy
        {
        public:
            reactor_io_pipe(io_service & service,const std::string & origin_name)
            {
                auto fifo = std::string("/tmp/lemon-") + origin_name;

                if(mkfifo(fifo.c_str(),S_IRUSR|S_IWUSR) && (errno!=EEXIST))
                {
                    throw std::system_error(errno,std::system_category());
                }

                auto in = open(fifo.c_str(),O_NONBLOCK|O_RDONLY);

                if(-1 == in)
                {
                    throw std::system_error(errno,std::system_category());
                }

                auto out = open(fifo.c_str(),O_NONBLOCK|O_WRONLY);

                if(-1 == out)
                {
                    throw std::system_error(errno,std::system_category());
                }


                _in.reset(new reactor_io_stream(service, (handler) in));
                _out.reset(new reactor_io_stream(service, (handler) out));
            }

            reactor_io_pipe(io_service & service):reactor_io_pipe(service,random_name())
            {

            }

            io_stream & in()
            {
                return *_in;
            }

            io_stream & out()
            {
                return *_out;
            }

            io_stream* release_in()
            {
                return _in.release();
            }

            io_stream* release_out()
            {
                return _out.release();
            }

            void close_in()
            {
                if(_in) _in.reset();
            }

            void close_out()
            {
                if(_out)_out.reset();
            }

        private:

            static const std::string random_name()
            {
                lemon::uuids::random_generator r;

                return lemon::uuids::to_string(r());
            }

        private:
            std::unique_ptr<io_stream> _in;
            std::unique_ptr<io_stream> _out;
        };

        using pipe = reactor_io_pipe;
    }
}

#endif //LEMON_IO_REACTOR_IO_PIPE_HPP