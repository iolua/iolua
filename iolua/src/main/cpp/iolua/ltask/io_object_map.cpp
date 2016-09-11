#include <iolua/ltask/io_object_map.hpp>

namespace iolua {
	namespace ltask {

		io_object_id io_object_map::newid()
		{
			io_object_id id;

			while (true) {

				id = _idgen++;

				if (id == 0 || _sockets.count(id) != 0) continue;

				return id;
			}
		}
		
		io_object_id io_object_map::create_socket(int af, int type, int protocol)
		{
			std::unique_lock<lemon::spin_mutex>	lock(_mutex);

			auto id = newid();

			auto acceptor = new io_socket(_service, af, type, protocol);

			_sockets[id] = acceptor;

			return id;
		}


		void io_object_map::close_socket(io_object_id id)
		{
			std::unique_lock<lemon::spin_mutex>	lock(_mutex);

			auto acceptor = _sockets[id];

			if(acceptor) {
				acceptor->close();
			}

			_sockets.erase(id);
		}

		

		io_socket* io_object_map::get_socket(io_object_id id)
		{
			std::unique_lock<lemon::spin_mutex>	lock(_mutex);

			auto acceptor = _sockets[id];

			if (acceptor) {
				acceptor->addref();

				return acceptor;
			}

			return nullptr;
		}
	}
}