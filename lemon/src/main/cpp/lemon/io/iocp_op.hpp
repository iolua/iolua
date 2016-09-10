/**
 * 
 * @file     iocp_op
 * @brief    Copyright (C) 2016  yayanyang All Rights Reserved 
 * @author   yayanyang
 * @date     2016/01/24
 */
#ifndef LEMON_IO_IOCP_OP_HPP
#define LEMON_IO_IOCP_OP_HPP

#include <lemon/config.h>
#include <lemon/io/handler.hpp>
namespace lemon{
	namespace io{

		enum class iocp_op_t
		{
			none = 0, read = 1, write = 2
		};

		class iocp_op : public OVERLAPPED
		{
		public:
			using complete_f = void(*)(iocp_op*);
			iocp_op                 *prev;
			iocp_op                 *next;
			const handler			fd;
			const iocp_op_t			op;
		protected:
		
			iocp_op(iocp_op_t o,handler h,complete_f complete)
				: _complete(complete)
				, next(nullptr)
				, prev(nullptr)
				, fd(h)
				, op(o)
			{
				memset(this, 0, sizeof(OVERLAPPED));
			}

		public:
			
			void complete(size_t trans)
			{
				_bytes_transferred = trans;

				_complete(this);
			}

			void complete(const std::error_code &ec)
			{
				_ec = ec;

				_complete(this);
			}

			void cancel(const std::error_code &ec)
			{
				_ec = ec;
			}

		private:

			complete_f                  _complete;

		protected:
			std::error_code             _ec;
			std::size_t                 _bytes_transferred;
		};
	}
}

#endif //LEMON_IO_IOCP_OP_HPP