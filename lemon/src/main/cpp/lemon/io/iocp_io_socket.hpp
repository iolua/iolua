/**
 *
 * @file     iocp_io_socket
 * @brief    Copyright (C) 2016  yayanyang All Rights Reserved
 * @author   yayanyang
 * @date     2016/02/18
 */
#ifndef LEMON_IO_IOCP_IO_SOCKET_HPP
#define LEMON_IO_IOCP_IO_SOCKET_HPP

#include <memory>
#include <system_error>

#include <lemon/config.h>
#include <lemon/io/sockaddr.hpp>
#include <lemon/io/iocp_io_object.hpp>
#include <lemon/io/iocp_io_service.hpp>
#include <lemon/io/iocp_io_stream.hpp>

#include <Mswsock.h>
#pragma comment(lib,"Mswsock.lib")

namespace lemon{ 
	namespace io{

		const static int max_addr_buffer_length = 56;

		class iocp_io_socket;

		template <typename Callback>
		class iocp_accept_op : public iocp_op
		{
		public:
			iocp_accept_op(handler fd,iocp_io_socket* peer, LPFN_GETACCEPTEXSOCKADDRS getsockaddrsex, Callback && callback)
				:iocp_op(iocp_op_t::read, fd, (iocp_op::complete_f)iocp_accept_op::read_complete)
				,_peer(peer)
				,_getsockaddrsex(getsockaddrsex)
				, _callback(callback)
			{
				
			}

			char * buff()
			{
				return _addrbuff;
			}

			int buffsize() const
			{
				return max_addr_buffer_length * 2;
			}

		private:

			static void read_complete(iocp_accept_op* op)
			{
				if(!op->_ec)
				{
					struct sockaddr * localAddress, *remoteAddress;

					int	localAddressSize, remoteAddressSize;

					op->_getsockaddrsex(
						op->_addrbuff,
						0,
						max_addr_buffer_length,
						max_addr_buffer_length,
						&localAddress,
						&localAddressSize,
						&remoteAddress,
						&remoteAddressSize);

					op->_callback(op->_peer, address(remoteAddress, remoteAddressSize), op->_ec);

					return;
				}

				op->_peer.reset();

				op->_callback(op->_peer, address(), op->_ec);
			}

		private:
			std::unique_ptr<iocp_io_socket>			_peer;
			LPFN_GETACCEPTEXSOCKADDRS				_getsockaddrsex;
			Callback								_callback;
			char									_addrbuff[max_addr_buffer_length*2];
			
		};

		template <typename Callback>
		class iocp_connect_op : public iocp_op
		{
		public:
			iocp_connect_op(handler fd, Callback && callback)
				:iocp_op(iocp_op_t::write, fd, (iocp_op::complete_f)iocp_connect_op::read_complete)
				, _callback(callback)
			{

			}

		private:

			static void read_complete(iocp_connect_op* op)
			{
				op->_callback(op->_ec);
			}

		private:
			Callback								_callback;
		};

		class iocp_io_socket : public iocp_io_object
		{
			
		public:
			iocp_io_socket(iocp_io_service & service, int af, int type, int protocol)
				:iocp_io_object(service,create_socket(af,type,protocol))
				,_af(af)
				,_type(type)
				,_protocol(protocol)
				,_acceptex(nullptr)
				,_connectex(nullptr)
				,_getsockaddrsex(nullptr)
			{

			}
			~iocp_io_socket()
			{
				close();
				closesocket((SOCKET)get());
			}

			void bind(const address & addr,std::error_code ec) noexcept
			{
				if (0 == ::bind((SOCKET)get(), addr, addr.length()))
				{
					ec = std::error_code(WSAGetLastError(),std::system_category());
				}
			}

			void listen(int backlog, std::error_code ec) noexcept
			{
				if (0 == ::listen((SOCKET)get(), backlog))
				{
					ec = std::error_code(WSAGetLastError(), std::system_category());
				}
			}
			template <typename Callback>
			void accept(Callback && callback,std::error_code & ec) noexcept
			{
				if (!_acceptex)
				{
					const static GUID GuidAcceptEx = WSAID_ACCEPTEX;

					win32extend(&GuidAcceptEx, &_acceptex, ec);

					if (ec) return;
				}

				if (!_getsockaddrsex)
				{
					const static GUID GuidGetAcceptExSockaddrs = WSAID_GETACCEPTEXSOCKADDRS;

					win32extend(&GuidGetAcceptExSockaddrs, &_getsockaddrsex, ec);

					if (ec) return;
				}

				iocp_io_socket *peer;

				try
				{
					peer = new iocp_io_socket(service(), _af, _type, _protocol);
				}
				catch (std::system_error & e)
				{
					ec = e.code();

					return;
				}

				std::unique_ptr<iocp_accept_op<Callback>> op(new iocp_accept_op<Callback>(get(), peer,_getsockaddrsex, std::forward<Callback>(callback)));

				add_op(op.get());

				if(!_acceptex((SOCKET)get(), (SOCKET)peer->get(), op->buff(),0 , max_addr_buffer_length, max_addr_buffer_length, 0, op.get()))
				{
					if (ERROR_IO_PENDING != GetLastError()) {
						
						op->cancel(std::error_code(GetLastError(), std::system_category()));
						service().complete(op.get());
					}
				}

				op.release();
			}

			template <typename Callback>
			void connect(const address & addr, Callback && callback, std::error_code & ec) noexcept
			{
				if (!_connectex)
				{
					const static GUID GuidConnectionEx = WSAID_CONNECTEX;

					win32extend(&GuidConnectionEx, &_connectex, ec);

					if (ec) return;
				}

				lemon::io::addrinfo addinfo = getaddrinfo("", "0", _af, _type, AI_CANONNAME)[0];

				bind(addinfo.addr(),ec);

				if(ec)
				{
					return;
				}

				std::unique_ptr<iocp_connect_op<Callback>> op(new iocp_connect_op<Callback>(get(), std::forward<Callback>(callback)));

				add_op(op.get());

				DWORD sendBytes = 0;


				if (!_connectex((SOCKET)get(), addr, addr.length(), NULL, sendBytes, &sendBytes, op.get()))
				{
					if (ERROR_IO_PENDING != WSAGetLastError()) {
						op->cancel(std::error_code(GetLastError(), std::system_category()));
						service().complete(op.get());
					}
				}

				op.release();
			}

			template <typename Callback>
			void recv(buffer buff, int flags, Callback && callback)
			{
				auto op = std::unique_ptr<iocp_op>(new iocp_read_op<Callback>(get(), std::forward<Callback>(callback)));

				add_op(op.get());

				WSABUF wsaBuff;
				wsaBuff.buf = (CHAR*)buff.data;
				wsaBuff.len = (ULONG)buff.length;
				
				DWORD dflag = flags;

				if (0 != WSARecv((SOCKET)get(), &wsaBuff, 1, NULL,&dflag, op.get(),NULL))
				{
					if (ERROR_IO_PENDING != GetLastError()) {
						op->cancel(std::error_code(GetLastError(), std::system_category()));
						service().complete(op.get());
					}
				}

				op.release();
			}

			template <typename Callback>
			void send(const_buffer buff, int flags, Callback && callback)
			{
				auto op = std::unique_ptr<iocp_op>(new iocp_write_op<Callback>(get(), std::forward<Callback>(callback)));

				add_op(op.get());

				WSABUF wsaBuff;
				wsaBuff.buf = (CHAR*)buff.data;
				wsaBuff.len = (ULONG)buff.length;

				DWORD dflag = flags;

				if (0 != WSASend((SOCKET)get(), &wsaBuff, 1, NULL, dflag, op.get(), NULL))
				{
					if (ERROR_IO_PENDING != GetLastError()) {
						op->cancel(std::error_code(GetLastError(), std::system_category()));
						service().complete(op.get());
					}
				}

				op.release();
			}

		private:

			void win32extend(const GUID *guid, void* fn,std::error_code & ec) noexcept 
			{
				DWORD dwBytes;

				if (SOCKET_ERROR == WSAIoctl((SOCKET)get(),
					SIO_GET_EXTENSION_FUNCTION_POINTER,
					(LPVOID)guid,
					sizeof(GUID),
					fn,
					sizeof(fn),
					&dwBytes,
					NULL,
					NULL))
				{
					ec = std::error_code(WSAGetLastError(), std::system_category());
				}
			}

			static handler create_socket(int af, int type, int protocol) 
			{
				auto fd = WSASocketW(
					af, type, protocol, NULL, 0, WSA_FLAG_OVERLAPPED);

				if (INVALID_SOCKET == fd)
				{
					throw std::system_error(GetLastError(), std::system_category());
				}

				return (handler)fd;
			}

		private:
			int								_af;
			int								_type;
			int								_protocol;
			LPFN_ACCEPTEX	                _acceptex;
			LPFN_CONNECTEX	                _connectex;
			LPFN_GETACCEPTEXSOCKADDRS       _getsockaddrsex;
		};

		typedef iocp_io_socket io_socket;
	}
}

#endif //LEMON_IO_IOCP_IO_SOCKET_HPP