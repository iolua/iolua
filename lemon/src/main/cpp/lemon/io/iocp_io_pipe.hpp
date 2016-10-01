/**
 * 
 * @file     iocp_io_pipe
 * @brief    Copyright (C) 2016  yayanyang All Rights Reserved 
 * @author   yayanyang
 * @date     2016/01/24
 */
#ifndef LEMON_IO_IOCP_IO_PIPE_HPP
#define LEMON_IO_IOCP_IO_PIPE_HPP
#include <locale>
#include <memory>
#include <sstream>

#include <lemon/config.h>
#include <lemon/uuid.hpp>

#include <lemon/io/iocp_io_stream.hpp>
#include <lemon/io/iocp_io_service.hpp>


namespace lemon{
	namespace io{

		class iocp_io_pipe 
		{

		public:
			iocp_io_pipe(iocp_io_service & service, const std::string & origin)
			{
				typedef std::wstring_convert<std::codecvt<wchar_t, char, std::mbstate_t>, wchar_t> convert;

				auto name = std::string("\\\\.\\pipe\\lemon-") + origin;

				SECURITY_ATTRIBUTES sa;

				sa.bInheritHandle = TRUE;

				sa.lpSecurityDescriptor = NULL;

				sa.nLength = sizeof(SECURITY_ATTRIBUTES);

				HANDLE reader, writer;

				convert conv;

				reader = CreateNamedPipeW(
						conv.from_bytes(name).c_str(),
						PIPE_ACCESS_INBOUND | FILE_FLAG_OVERLAPPED,
						PIPE_TYPE_BYTE | PIPE_READMODE_BYTE | PIPE_WAIT,
						PIPE_UNLIMITED_INSTANCES,
						1024 * 1024,
						1024 * 1024,
						5000,
						&sa);

				if (reader == INVALID_HANDLE_VALUE)
				{
					throw std::system_error(GetLastError(), std::system_category());
				}

				writer = CreateFileW(
						conv.from_bytes(name).c_str(),
						GENERIC_WRITE,
						0,
						&sa,
						OPEN_EXISTING,
						FILE_FLAG_OVERLAPPED,
						NULL);

				if (writer == INVALID_HANDLE_VALUE)
				{
					throw std::system_error(GetLastError(), std::system_category());
				}

				// exception safe confirm

				_in.reset(new iocp_io_stream(service, reader));
				_out.reset(new iocp_io_stream(service, writer));
			}


			iocp_io_pipe(iocp_io_service & service):iocp_io_pipe(service,random_name())
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
				_in.reset();
			}

			void close_out()
			{
				_out.reset();
			}

		private:

			static const std::string random_name()
			{
				lemon::uuids::random_generator r;

				std::stringstream stream;

				stream << "/tmp/lemon-" << lemon::uuids::to_string(r());

				return stream.str();
			}

		private:
			std::unique_ptr<io_stream> _in;
			std::unique_ptr<io_stream> _out;
		};


		typedef iocp_io_pipe pipe;
	}
}

#endif //LEMON_IO_IOCP_IO_PIPE_HPP