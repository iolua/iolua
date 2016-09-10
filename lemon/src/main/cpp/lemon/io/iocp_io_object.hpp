/**
 * 
 * @file     iocp_io_object
 * @brief    Copyright (C) 2016  yayanyang All Rights Reserved 
 * @author   yayanyang
 * @date     2016/01/24
 */
#ifndef LEMON_IO_IOCP_IO_OJBECT_HPP
#define LEMON_IO_IOCP_IO_OJBECT_HPP
#include <utility>
#include <system_error>
#include <lemon/config.h>
#include <lemon/nocopy.hpp>

#include <lemon/io/handler.hpp>
#include <lemon/io/iocp_op.hpp>

namespace lemon{
	namespace io{

		class iocp_io_service;
		class iocp_io_object;

		void iocp_io_service_register(
			iocp_io_service& service,
			iocp_io_object *obj,
			std::error_code & ec
			) noexcept;

		void iocp_io_service_unregister(
			iocp_io_service& service,
			iocp_io_object *obj
			) noexcept;

		class iocp_io_object : private nocopy
		{
		public:
			iocp_io_object(iocp_io_service & service, handler fd)
				:_service(service),_fd(fd),_read_header(nullptr),_write_header(nullptr)
			{
				std::error_code ec;

				iocp_io_service_register(service, this, ec);

				if(ec)
				{
					throw std::system_error(ec);
				}
			}

			handler get() const
			{
				return _fd;
			}

			bool remove_op(iocp_op *op)
			{
				if(op->op == iocp_op_t::read)
				{
					return pop(&_read_header, op);
				}

				if (op->op == iocp_op_t::write)
				{
					return pop(&_write_header, op);
				}

				return false;
			}

			void add_op(iocp_op *op)
			{
				if (op->op == iocp_op_t::read)
				{
					push(&_read_header, op);
				}

				if (op->op == iocp_op_t::write)
				{
					push(&_write_header, op);
				}
			}

			iocp_io_service &service()
			{
				return _service;
			}
		protected:
			void close()
			{
				iocp_io_service_unregister(service(), this);
			}
		private:
			static void push(iocp_op **header, iocp_op *irp) noexcept
			{
				irp->next = *header;

				if(*header != nullptr)
				{
					(*header)->prev = irp;
				}
				
				*header = irp;
			}

			static bool pop(iocp_op ** header, iocp_op *irp) noexcept
			{

				if(irp->next)
				{
					irp->next->prev = irp->prev;
				}

				if(irp->prev)
				{
					irp->prev->next = irp->next;
				}

				if(irp == *header)
				{
					*header = irp->next;
				}

				return true;
			}

		private:
		

		private:
			iocp_io_service				&_service;
			handler						_fd;
			iocp_op						*_read_header;
			iocp_op						*_write_header;
		};
	}
}

#endif //LEMON_IO_IOCP_IO_OJBECT_HPP