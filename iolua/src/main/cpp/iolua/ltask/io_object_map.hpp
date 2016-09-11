#ifndef IOLUA_LTASK_IO_OBJECT_MAP_HPP
#define IOLUA_LTASK_IO_OBJECT_MAP_HPP
#include <atomic>
#include <type_traits>
#include <unordered_set>
#include <unordered_map>
#include <lemon/mutex.hpp>
#include <lemon/nocopy.hpp>
#include <lemon/io/io.hpp>
#include <lua/lua.hpp>

namespace iolua {
	namespace ltask {

		using io_object_id = uint32_t;

		using io_service = lemon::io::io_service;

		template<typename _Type>
		class io_object : lemon::nocopy
		{
		public:
			template<typename ... _Args>
			io_object(_Args &&...args)
				:_counter(1),_object(std::forward<_Args>(args)...)
			{

			}

			~io_object()
			{

			}

			_Type * operator -> ()
			{
				return &_object;
			}

			const _Type * operator -> () const
			{
				return &_object;
			}

			void close()
			{
				 if(-- _counter == 0) {
					 delete this;
				 }
			}

			void addref()
			{
				++_counter;
			}

		private:
			std::atomic<uint32_t>	_counter;
			_Type					_object;
		};

		using io_socket = io_object<lemon::io::io_socket>;

		class io_object_map : lemon::nocopy
		{
		public:

			io_object_map(io_service & service) :_service(service) {}

			io_object_id create_socket(int af, int type, int protocol);
			
			void close_socket(io_object_id id);

			io_socket* get_socket(io_object_id id);
		private:

			io_object_id newid();

		private:
			io_service 												&_service;
			io_object_id											_idgen = 0;
			lemon::spin_mutex										_mutex;
			std::unordered_map<io_object_id, io_socket*>			_sockets;
		};
	}
}

#endif //IOLUA_LTASK_IO_OBJECT_MAP_HPP
