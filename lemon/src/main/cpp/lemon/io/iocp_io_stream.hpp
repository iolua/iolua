/**
 * 
 * @file     iocp_io_stream
 * @brief    Copyright (C) 2016  yayanyang All Rights Reserved 
 * @author   yayanyang
 * @date     2016/01/24
 */
#ifndef LEMON_IO_IOCP_IO_STREAM_HPP
#define LEMON_IO_IOCP_IO_STREAM_HPP

#include <utility>

#include <lemon/io/buff.hpp>
#include <lemon/io/iocp_op.hpp>
#include <lemon/io/iocp_io_object.hpp>
#include <lemon/io/iocp_io_service.hpp>

namespace lemon{
	namespace io{

		template <typename Callback>
		class iocp_read_op : public iocp_op
		{
		public:
			iocp_read_op(handler fd,Callback && callback)
				:iocp_op(iocp_op_t::read,fd,(iocp_op::complete_f)iocp_read_op::read_complete)
				,_callback(callback)
			{

			}

		private:

			static void read_complete(iocp_read_op* op)
			{
				op->_callback(op->_bytes_transferred, op->_ec);
			}

		private:
			Callback                _callback;
		};


		template <typename Callback>
		class iocp_write_op : public iocp_op
		{
		public:
			iocp_write_op(handler fd, Callback && callback)
				:iocp_op(iocp_op_t::write, fd, (iocp_op::complete_f)iocp_write_op::write_complete)
				, _callback(callback)
			{

			}

		private:

			static void write_complete(iocp_write_op* op)
			{
				op->_callback(op->_bytes_transferred, op->_ec);
			}

		private:
			Callback                _callback;
		};

		class iocp_io_stream : public iocp_io_object
		{
		public:
			iocp_io_stream(iocp_io_service &service,handler fd)
				:iocp_io_object(service,fd)
			{

			}
		
			~iocp_io_stream()
			{
				close();
				::CloseHandle(get());
			}
		public:
			template <typename Callback>
			void read(buffer buff, Callback && callback)
			{
				auto op = std::unique_ptr<iocp_op>(new iocp_read_op<Callback>(get(), std::forward<Callback>(callback)));

				add_op(op.get());

				DWORD read;

				if (!ReadFile(get(), buff.data, (DWORD)buff.length, &read,op.get()))
				{
					if (ERROR_IO_PENDING != GetLastError()) {
						op->cancel(std::error_code(GetLastError(), std::system_category()));
						service().complete(op.get());
					}
				}
				
				op.release();
			}
		};

		typedef iocp_io_stream io_stream;
	}
}

#endif //LEMON_IO_IOCP_IO_STREAM_HPP