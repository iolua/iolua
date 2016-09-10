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
            reactor_io_pipe(io_service & service)
            {
                lemon::uuids::random_generator r;

                std::stringstream stream;

                stream << "/tmp/lemon-" << lemon::uuids::to_string(r());

                auto fifo = stream.str();

                if(mkfifo(fifo.c_str(),S_IRWXU) && (errno!=EEXIST))
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


                _in.reset(new reactor_io_stream(service,in));
                _out.reset(new reactor_io_stream(service,out));

            }

            io_stream & in()
            {
                return *_in;
            }

            io_stream & out()
            {
                return *_out;
            }

        private:
            std::unique_ptr<io_stream> _in;
            std::unique_ptr<io_stream> _out;
        };

        using pipe = reactor_io_pipe;
    }
}

#endif //LEMON_IO_REACTOR_IO_PIPE_HPP