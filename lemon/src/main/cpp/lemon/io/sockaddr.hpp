/**
 *
 * @file     sockaddr
 * @brief    Copyright (C) 2016  yayanyang All Rights Reserved
 * @author   yayanyang
 * @date     2016/02/18
 */
#ifndef LEMON_IO_SOCKADDR_HPP
#define LEMON_IO_SOCKADDR_HPP

#include <vector>
#include <cerrno>
#include <string>
#include <system_error>
#include <lemon/config.h>
#include <lemon/nocopy.hpp>

namespace lemon{
	namespace io{
		class address
		{
		public:
			address():address(0){}

			address(int length)
				:_len(length)
			{

				if(length)
				{
					_buff = new char[length];
				}
			}

			~address()
			{
				if (_len)
				{
					delete[] (char*)_buff;
				}
			}

			bool empty() const
			{
				return _len == 0;
			}

			address(sockaddr * addr,int len)
				:_len(len)
			{
				if(len > 0)
				{
					_buff = new char[len];

					memcpy(_buff, addr, len);
				}
			}

			address(const address & rhs)
				:address((sockaddr*)rhs._buff,rhs._len)
			{

			}

			address(address && rhs)
			{
				*this = std::forward<address>(rhs);
			}

			address & operator = (address && rhs)
			{
				_buff = std::move(rhs._buff);

				_len = rhs._len;

				rhs._len = 0;

				return *this;
			}

			operator sockaddr*()
			{
				return (sockaddr*)_buff;
			}

			operator const sockaddr*() const
			{
				return (const sockaddr*)_buff;
			}

			int length() const
			{
				return _len;
			}

			std::string host() const
			{
				if (empty()) return{};

				const sockaddr * addr = *this;

				char host[128] = { 0 };

				switch (addr->sa_family)
				{
				case AF_INET: {

					struct sockaddr_in *v4 = (struct sockaddr_in*)addr;

					if (!inet_ntop(AF_INET, &v4->sin_addr, host, sizeof(host))) {
#ifdef WIN32
						throw std::system_error(WSAGetLastError(),std::system_category(), "call inet_ntop exception");
#else
						throw std::system_error(errno, std::system_category(), "call inet_ntop exception");
#endif
					}

					break;
				}
				case AF_INET6: {
					struct sockaddr_in6 *v6 = (struct sockaddr_in6*)addr;

					if (!inet_ntop(AF_INET6, &v6->sin6_addr, host, sizeof(host))) {
#ifdef WIN32
						throw std::system_error(WSAGetLastError(), std::system_category(), "call inet_ntop exception");
#else
						throw std::system_error(errno, std::system_category(), "call inet_ntop exception");
#endif
					}

					break;
				}
				default:
					return "unknown";
				}

				return host;
			}

			int service() const
			{
				if (empty()) return{};

				const sockaddr * addr = *this;

				switch (addr->sa_family)
				{
				case AF_INET: {

					struct sockaddr_in *v4 = (struct sockaddr_in*)addr;

					return ntohs(v4->sin_port);
				}
				case AF_INET6: {
					struct sockaddr_in6 *v6 = (struct sockaddr_in6*)addr;

					return ntohs(v6->sin6_port);
				}
				default:
					return 0;
				}
			}

		private:
			void				*_buff;
			int					_len;
		};

		class addrinfo
		{
		public:
			addrinfo(const ::addrinfo *info)
				:_addr(info->ai_addr,info->ai_addrlen)
				,_af(info->ai_family)
				,_type(info->ai_socktype)
				,_protocol(info->ai_protocol)
				,_canonname(info->ai_canonname? info->ai_canonname:"")
			{

			}

			const address & addr() const
			{
				return _addr;
			}

			int af() const
			{
				return _af;
			}

			int type() const
			{
				return _type;
			}

			int protocol() const
			{
				return _protocol;
			}

			const std::string & canonname() const
			{
				return _canonname;
			}

		private:
			address				_addr;
			int					_af;
			int					_type;
			int					_protocol;
			std::string			_canonname;
		};


		std::vector<addrinfo> getaddrinfo(
			const std::string & host,
			const std::string & service,
			int af, int type, int flags,
			std::error_code & ec);

		inline std::vector<addrinfo> getaddrinfo(
			const std::string & host,
			const std::string & service,
			int af, int type, int flags)
		{
			std::error_code ec;

			auto result = getaddrinfo(host, service, af, type, flags, ec);

			if (ec)
			{
				throw std::system_error(ec);
			}

			return result;
		}
	}
}

#endif //LEMON_IO_SOCKADDR_HPP