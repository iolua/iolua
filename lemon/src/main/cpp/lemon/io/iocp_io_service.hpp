/**
 * 
 * @file     iocp_io_service
 * @brief    Copyright (C) 2016  yayanyang All Rights Reserved 
 * @author   yayanyang
 * @date     2016/01/24
 */
#ifndef LEMON_IO_IOCP_IO_SERVICE_HPP
#define LEMON_IO_IOCP_IO_SERVICE_HPP
#include <mutex>
#include <atomic>
#include <chrono>
#include <system_error>
#include <unordered_map>

#include <mutex>

#include <lemon/config.h>
#include <lemon/nocopy.hpp>
#include <lemon/io/handler.hpp>
#include <lemon/io/iocp_op.hpp>

namespace lemon{ 
	namespace io{

		static std::once_flag was_socket_init_flag;

		inline void was_socket_init()
		{
			WSADATA wsaData;

			DWORD result;

			if (0 != (result = WSAStartup(MAKEWORD(2, 2), &wsaData))) {
				throw std::system_error(WSAGetLastError(),std::system_category());
			}
		}

		class iocp_io_service final: private nocopy
		{
		public:
			friend void iocp_io_service_register(
				iocp_io_service& service,
				iocp_io_object *obj,
				std::error_code & ec
				) noexcept;

			friend void iocp_io_service_unregister(
				iocp_io_service& service,
				iocp_io_object *obj
				) noexcept;
		public:
			
			iocp_io_service():_joins(0)
			{
				std::call_once(was_socket_init_flag, was_socket_init);

				_handler = ::CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, NULL, 1);

				if (_handler == NULL)
				{
					throw std::system_error(GetLastError(), std::system_category());
				}
			}

			~iocp_io_service()
			{
				CloseHandle(_handler);
			}

			void run_one(std::error_code & ec) noexcept
			{
				run_one(std::chrono::seconds(-1), ec);
			}

			void run_one()
			{
				std::error_code ec;
				run_one(ec);

				if(ec)
				{
					throw std::system_error(ec);
				}
			}

			template <typename _Rep, typename _Period>
			void run_one(std::chrono::duration<_Rep, _Period> timeout)
			{
				std::error_code ec;
				run_one(timeout,ec);

				if (ec&&ec != std::errc::timed_out)
				{
					throw std::system_error(ec);
				}
			}

			void notify_all()
			{
				std::error_code ec;

				notify_all(ec);

				if(ec)
				{
					throw std::system_error(ec);
				}
			}

			void notify_all(std::error_code & ec) noexcept
			{
				int count = _joins;

				for (int i = 0; i < count; i ++)
				{
					if(!::PostQueuedCompletionStatus(_handler,0,0,nullptr))
					{
						ec = std::error_code(GetLastError(), std::system_category());
					}
				}
			}

			template <typename _Rep, typename _Period>
			void run_one(std::chrono::duration<_Rep, _Period> timeout, std::error_code & ec) noexcept
			{
				_joins++;
				__run_one(timeout, ec);
				_joins--;
			}

			void complete(iocp_op *op) noexcept
			{
				if(::PostQueuedCompletionStatus(_handler, 0, 0, op))
				{
					// TODO: log the error
				}
			}

		private:

			template <typename _Rep, typename _Period>
			void __run_one(std::chrono::duration<_Rep, _Period> timeout, std::error_code & ec) noexcept
			{

				DWORD bytes;

				ULONG_PTR completionKey;

				iocp_op * op = NULL;

				auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(timeout);

				DWORD timeoutVal = (DWORD)milliseconds.count();

				if (timeout == std::chrono::duration<_Rep, _Period>(-1))
				{
					timeoutVal = INFINITE;
				}

				if (GetQueuedCompletionStatus(_handler, &bytes, &completionKey, (LPOVERLAPPED*)&op, (DWORD)timeoutVal))
				{
					if(valid(op))
					{
						op->complete(bytes);

						delete op;
					}

					return;
				}

				DWORD lasterror = GetLastError();

				if(WAIT_TIMEOUT == lasterror)
				{
					ec = std::make_error_code(std::errc::timed_out);
					return;
				}

				if(ERROR_ABANDONED_WAIT_0 == lasterror)
				{
					ec = make_error_code(errc::io_service_closed);
					return;
				}

				if (op == NULL) {
					ec = std::error_code(lasterror, std::system_category());
					return;
				}

				if(valid(op)){
					op->complete(std::error_code(lasterror, std::system_category()));
				}

				delete op;
			}

			bool valid(iocp_op *op)
			{
				if (nullptr == op) return false;

				std::lock_guard<std::mutex> lock(_mutex);

				auto iter = _handlers.find(op->fd);

				if(iter != _handlers.end())
				{
					return iter->second->remove_op(op);
				}

				return false;
			}
		
		private:
			std::mutex												_mutex;
			std::atomic<int>										_joins;
			HANDLE													_handler;
			std::unordered_map<handler, iocp_io_object*>            _handlers; // register objects
		};

		inline void iocp_io_service_register(
			iocp_io_service& service,
			iocp_io_object *obj,
			std::error_code & ec
			) noexcept
		{

			if (NULL == CreateIoCompletionPort(obj->get(),service._handler, 0, 0))
			{
				ec = std::error_code(GetLastError(), std::system_category());
			}
			else
			{
				std::lock_guard<std::mutex> lock(service._mutex);

				service._handlers[obj->get()] = obj;
			}
		}

		inline void iocp_io_service_unregister(
			iocp_io_service& service,
			iocp_io_object * obj
			) noexcept
		{
			std::lock_guard<std::mutex> lock(service._mutex);

			service._handlers.erase(obj->get());
		}


		typedef iocp_io_service io_service;
	}
}

#endif //LEMON_IO_IOCP_IO_SERVICE_HPP