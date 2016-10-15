#ifndef IOLUA_IO_HPP
#define IOLUA_IO_HPP
#include <thread>
#include <memory>
#include <type_traits>
#include <lemon/io/io.hpp>
#include <lua/lua.hpp>
#include <iolua/shared_object.hpp>

namespace iolua {
	class iolua_State;
	class io_object final: public shared_object
	{
	public:
		io_object(lemon::io::io_object* object, iolua_State * context);
		~io_object();

		template<typename _Type>
		typename std::add_pointer<_Type>::type pointer()
		{
			static_assert(std::is_base_of<lemon::io::io_object,_Type>::value, "the pointer's class type must inherit from lemon::io::io_object");

			return reinterpret_cast<typename std::add_pointer<_Type>::type>(_object.get());
		}

		template<typename _Type>
		typename std::add_const<typename std::add_pointer<_Type>::type>::type pointer() const
		{
			static_assert(std::is_base_of<lemon::io::io_object, _Type>::value, "the pointer's class type must inherit from lemon::io::io_object");

			return reinterpret_cast<typename std::add_const<typename std::add_pointer<_Type>::type>::type>(_object.get());
		}

		void close() {}

	private:
		std::unique_ptr<lemon::io::io_object>	_object;
		iolua_State *_context;
	};

	class io_object_cached : public shared_object_cached<io_object>
	{
	public:
		io_object_cached();
		~io_object_cached();

		lemon::io::io_service & io_service()
		{
			return _ioservice;
		}

	private:

		void do_dispatch();

	private:
		lemon::io::io_service		_ioservice;
		std::thread					_worker;
	};

	class io_promise : public shared_object
	{
	public:
		io_promise(iolua_State * context):_context(context) {}
	
		virtual int complete(lua_State*)
		{
			return 0;
		};
		
		iolua_State * context()
		{
			return _context;
		}

		static int k_func(lua_State *L, int status, lua_KContext ctx);

		void close() {}

	private:
		iolua_State			*_context;
	};

	class io_promise_accept : public io_promise
	{
	public:
		using io_promise::io_promise;

		void accept(std::unique_ptr<lemon::io::io_socket> & socket, const lemon::io::address & addr, const std::error_code & ec);

		int complete(lua_State* L);

	private:
		std::uint32_t								_id;
		lemon::io::address							_addr;
		std::error_code								_ec;
	};

	class io_promise_recv : public io_promise
	{
	public:
		using io_promise::io_promise;

		void recv(size_t trans, const std::error_code & ec);

		int complete(lua_State *L);

		char * buff(std::size_t maxlen)
		{
			_buff.resize(maxlen);

			return &_buff[0];
		}

	private:
		std::vector<char>				_buff;
		std::error_code					_ec;
		size_t							_trans;
	};

	class io_promise_send : public io_promise
	{
	public:
		using io_promise::io_promise;

		void send(size_t trans, const std::error_code & ec);

		int complete(lua_State *L);

		lemon::io::const_buffer buff(lemon::io::const_buffer source)
		{
			_buff.assign((char*)source.data,((char*)source.data) + source.length);

			return { &_buff[0],_buff.size() };
		}

	private:
		std::error_code					_ec;
		size_t							_trans;
		std::vector<char>				_buff;
	};

	class io_promise_connect : public io_promise
	{
	public:
		using io_promise::io_promise;

		void connect(const std::error_code & ec);

		int complete(lua_State *L);

	private:
		std::error_code					_ec;
	};

	class io_promise_exec_wait: public io_promise
	{
	public:
		using io_promise::io_promise;

		void wait(int exit_code, const std::error_code & ec);

		int complete(lua_State *L);

	private:
		int 							_exit_code;
		std::error_code					_ec;
	};
}

#endif //IOLUA_IO_HPP
