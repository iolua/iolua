#include <lemon/io/sockaddr.hpp>

#include <errno.h>

namespace lemon {
	namespace io {
		std::vector<addrinfo> getaddrinfo(
			const std::string & host, 
			const std::string & service,
			int af, int type, int flags,
			std::error_code &ec)
		{
			::addrinfo hints, *target;

			memset(&hints, 0, sizeof(hints));

			hints.ai_family = af;

			hints.ai_socktype = type;

			hints.ai_flags |= flags;

			int status = ::getaddrinfo(host.empty()?host.c_str():0, service.c_str(), &hints, &target);

			if (status) {
#ifdef WIN32
				ec = std::error_code(WSAGetLastError(),std::system_category());
#else
				ec = std::error_code(errno, std::system_category());
#endif //LEMOON_UNICODE
				return {};
			}

			std::vector<addrinfo> addrs;


			for (::addrinfo* iter = target; iter != NULL; iter = iter->ai_next) 
			{
				addrs.push_back(addrinfo(iter));
			}

			freeaddrinfo(target);

			return addrs;
		}
	}
}