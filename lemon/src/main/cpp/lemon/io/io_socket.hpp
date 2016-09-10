#ifndef LEMON_IO_IO_SOCKET_HPP
#define LEMON_IO_IO_SOCKET_HPP

#ifdef WIN32
#include <lemon/io/iocp_io_socket.hpp>
#else
#include <lemon/io/reactor_io_socket.hpp>
#endif //WIN32

#include <lemon/io/io_service.hpp>

namespace lemon {
    namespace io {
		/**
         * stream socket acceptor
         */
        class io_socket_acceptor : nocopy
        {
        public:
            io_socket_acceptor(io_service & service, int af, int type, int protocol)
                    :_socket(new io_socket(service,af, type, protocol))
            {

            }

            io_socket_acceptor(io_socket * socket): _socket(socket)
            {

            }

            /**
             * bind server socket's address
             */
            void bind(const address & addr, std::error_code ec) noexcept
            {
                _socket->bind(addr,ec);
            }


            void bind(const address & addr)
            {
                std::error_code ec;
                bind(addr, ec);

                if (ec)
                {
                    throw std::system_error(ec);
                }
            }

            void listen(int backlog, std::error_code ec) noexcept
            {
                _socket->listen(backlog, ec);
            }

            void listen(int backlog)
            {
                std::error_code ec;
                listen(backlog, ec);

                if(ec)
                {
                    throw std::system_error(ec);
                }
            }

            template<typename Callback>
            void accept(Callback &&callback, std::error_code & ec) noexcept
            {
                _socket->accept(std::forward<Callback>(callback), ec);
            }

            template<typename Callback>
            void accept(Callback &&callback)
            {
                std::error_code ec;
                accept(std::forward<Callback>(callback), ec);

                if (ec)
                {
                    throw std::system_error(ec);
                }
            }
        private:
            std::unique_ptr<io_socket>					_socket;
        };



        /**
         * the stream socket facade
         */
        class io_stream_socket : nocopy
        {
        public:
            io_stream_socket(io_service & service, int af, int type, int protocol)
                    :_socket(new io_socket(service, af, type, protocol))
            {

            }

            io_stream_socket(std::unique_ptr<io_socket> & socket) : _socket(std::move(socket))
            {

            }

            template <typename Callback>
            void recv(buffer buff, int flags, Callback && callback)
            {
                _socket->recv(buff, flags, std::forward<Callback>(callback));
            }

            template <typename Callback>
            void send(const_buffer buff, int flags, Callback && callback)
            {
                _socket->send(buff, flags, std::forward<Callback>(callback));
            }


			template <typename Callback>
			void connect(const address & addr, Callback && callback, std::error_code & ec) noexcept
			{
				_socket->connect(addr, callback, ec);
			}

			template <typename Callback>
			void connect(const address & addr, Callback && callback)
			{
				std::error_code ec;
				_socket->connect(addr, callback, ec);

				if (ec)
				{
					throw std::system_error(ec);
				}
			}

        protected:
            std::unique_ptr<io_socket>					_socket;
        };
    }
}

#endif //LEMON_IO_IO_SOCKET_HPP