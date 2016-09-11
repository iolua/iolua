#ifndef IOLUA_LTASK_IO_PROMISE_HPP
#define IOLUA_LTASK_IO_PROMISE_HPP

#include <atomic>
#include <cstdint>
#include <system_error>
#include <unordered_map>

#include <lua/lua.hpp>
#include <lemon/mutex.hpp>
#include <lemon/nocopy.hpp>

namespace iolua {
	namespace ltask {
		class io_promise : lemon::nocopy
		{
		public:
			io_promise(uint32_t id):_id(id){}

			virtual int raise_promise(lua_State *L) = 0;

			virtual ~io_promise() {}

		private:
			uint32_t						_id;
		};

		class connect_io_promise : public io_promise
		{
		public:
			using io_promise::io_promise;

			void complete(const std::error_code & ec)
			{
				_ec = ec;
			}

			int raise_promise(lua_State *L) 
			{
				if (_ec) {
					lua_pushboolean(L, false);
					lua_pushstring(L, _ec.message().c_str());
					return 2;
				} else {
					lua_pushboolean(L, true);
					return 1;
				}
			}
		private:
			std::error_code		_ec;
		};

		class send_io_promise : public io_promise
		{
		public:
			using io_promise::io_promise;

			void complete(size_t trans, const std::error_code & ec)
			{
				_ec = ec;
				_trans = trans;
			}

			int raise_promise(lua_State *L)
			{
				if (_ec) {
					lua_pushboolean(L, false);
					lua_pushstring(L, _ec.message().c_str());
					return 2;
				}
				else {
					lua_pushboolean(L, true);
					lua_pushinteger(L, _trans);
					return 2;
				}
			}
		private:
			std::error_code		_ec;
			size_t				_trans;
		};

		class accept_io_promise : public io_promise
		{
		public:
			using io_promise::io_promise;

			void complete(io_object_id id, const lemon::io::address & addr, const std::error_code & ec)
			{
				_ec = ec;
				_obj = id;
				_addr = addr;
			}

			int raise_promise(lua_State *L)
			{
				if (_ec) {
					lua_pushboolean(L, false);
					lua_pushstring(L, _ec.message().c_str());
					return 2;
				}
				else {
					lua_pushboolean(L, true);
					lua_pushinteger(L, _obj);
					lua_pushstring(L, _addr.host().c_str());
					lua_pushinteger(L, _addr.service());
					return 4;
				}
			}
		private:
			std::error_code					_ec;
			lemon::io::address				_addr;
			io_object_id					_obj;
		};

		class recv_io_promise : public io_promise
		{
		public:
			recv_io_promise(size_t maxlength, uint32_t id):io_promise(id), _buff(maxlength)
			{

			}

			void complete(size_t trans, const std::error_code & ec)
			{
				_ec = ec;
				_trans = trans;
			}

			int raise_promise(lua_State *L)
			{
				if (_ec) {
					lua_pushboolean(L, false);
					lua_pushstring(L, _ec.message().c_str());
					return 2;
				}
				else {
					lua_pushboolean(L, true);
					lua_pushlstring(L, (const char*)&_buff[0], _trans);
					return 2;
				}
			}

			lemon::io::buffer buff()
			{
				return { &_buff[0], _buff.size() };
			}

		private:
			std::error_code			_ec;
			size_t					_trans;
			std::vector<char>		_buff;
		};


		class io_promise_map : lemon::nocopy
		{
		public:
			uint32_t create_connect_io_promise()
			{
				std::unique_lock<lemon::spin_mutex>	lock(_mutex);

				auto id = newid();

				_promises[id] = new connect_io_promise(id);

				return id;
			}

			uint32_t create_send_io_promise()
			{
				std::unique_lock<lemon::spin_mutex>	lock(_mutex);

				auto id = newid();

				_promises[id] = new send_io_promise(id);

				return id;
			}

			uint32_t create_accept_io_promise()
			{
				std::unique_lock<lemon::spin_mutex>	lock(_mutex);

				auto id = newid();

				_promises[id] = new accept_io_promise(id);

				return id;
			}

			uint32_t create_recv_io_promise(size_t maxlength, lemon::io::buffer & buff)
			{
				std::unique_lock<lemon::spin_mutex>	lock(_mutex);

				auto id = newid();

				auto promise = new recv_io_promise(maxlength, id);

				_promises[id] = promise;

				buff = promise->buff();

				return id;
			}

			void close_promise(uint32_t id)
			{
				std::unique_lock<lemon::spin_mutex>	lock(_mutex);

				auto promise = _promises[id];

				if (promise) {
					_promises.erase(id);
					delete promise;
				}
			}

			io_promise* get_promise(uint32_t id)
			{
				std::unique_lock<lemon::spin_mutex>	lock(_mutex);

				auto promise = _promises[id];

				if (promise) {
					return promise;
				}

				return nullptr;
			}

		private:
			uint32_t newid()
			{
				uint32_t id;

				while (true) {

					id = _idgen++;

					if (id == 0 || _promises.count(id) != 0) continue;

					return id;
				}
			}
		private:
			uint32_t									_idgen = 0;
			lemon::spin_mutex							_mutex;
			std::unordered_map<uint32_t, io_promise*>	_promises;
		};
	}
}

#endif // IOLUA_LTASK_IO_PROMISE_HPP
