#include <iolua/io.hpp>
#include <iolua/iolua.hpp>
#include <iolua/task.hpp>
namespace iolua {
	io_object::io_object(lemon::io::io_object* object, iolua_State * context)
		:_object(object),_context(context)
	{

	}
	
	io_object::~io_object()
	{

	}

	io_object_cached::io_object_cached()
	{
		_worker = std::thread(&io_object_cached::do_dispatch, this);
	}

	io_object_cached::~io_object_cached()
	{
		_ioservice.close();

		if (_worker.joinable())
		{
			_worker.join();
		}
	}

	void io_object_cached::do_dispatch()
	{

		for (;;)
		{
			std::error_code ec;
			_ioservice.run_one(ec);

			if (ec == lemon::io::errc::io_service_closed)
			{
				break;
			}
		}
	}

	int io_promise::k_func(lua_State *L, int , lua_KContext ctx)
	{
		task * tk = (task*)lua_touserdata(L, lua_upvalueindex(1));

		auto promise = tk->context()->query_io_promise((std::uint32_t)ctx);

		auto returnargs = promise->complete(L);

		promise->unref();

		tk->context()->close_io_promise((std::uint32_t)ctx);

		return returnargs;
	}

	void io_promise_accept::accept(std::unique_ptr<lemon::io::io_socket> & socket, const lemon::io::address & addr, const std::error_code & ec)
	{
		if (socket)
		{
			_id = context()->create_io_object(socket.release());
		}

	
		_addr = std::move(addr);

		_ec = ec;
	}

	int io_promise_accept::complete(lua_State* L)
	{
		if (_ec)
		{
			lua_pushboolean(L, 0);
			lua_pushfstring(L, _ec.message().c_str());
			return 2;
		}

		lua_pushboolean(L, 1);
		lua_pushinteger(L, _id);
		lua_pushstring(L, _addr.host().c_str());
		lua_pushinteger(L, (lua_Integer) _addr.service());

		return 4;
	}

	void io_promise_recv::recv(size_t trans, const std::error_code & ec)
	{
		_ec = ec;
		_trans = trans;
	}

	int io_promise_recv::complete(lua_State *L)
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

	void io_promise_send::send(size_t trans, const std::error_code & ec)
	{
		_ec = ec;
		_trans = trans;
	}

	int io_promise_send::complete(lua_State *L)
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

	void io_promise_connect::connect(const std::error_code & ec)
	{
		_ec = ec;
	}

	int io_promise_connect::complete(lua_State *L)
	{
		if (_ec) {
			lua_pushboolean(L, false);
			lua_pushstring(L, _ec.message().c_str());
			return 2;
		}
		else {
			lua_pushboolean(L, true);
			return 1;
		}
	}

	void io_promise_exec_wait::wait(int exit_code, const std::error_code & ec)
	{
		_exit_code = exit_code;

		_ec = ec;
	}

	int io_promise_exec_wait::complete(lua_State *L)
	{
		if (_ec) {
			lua_pushboolean(L, false);
			lua_pushstring(L, _ec.message().c_str());
			return 2;
		}
		else {
			lua_pushboolean(L, true);
			lua_pushinteger(L, _exit_code);
			return 2;
		}
	}
}