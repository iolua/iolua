#include <iolua/libio.hpp>
#include <iolua/task.hpp>
#include <lemon/log/log.hpp>
#include <iolua/serializer.h>


namespace iolua {
	
	static auto& logger = lemon::log::get("iolua");

	static int lsocket_close(lua_State* L)
	{
		task * tk = (task*)lua_touserdata(L, lua_upvalueindex(1));

		tk->context()->close_io_object(luaL_checkinteger(L, 1));

		return 0;
	}

	static int lsocket_bind(lua_State* L)
	{
		task * tk = (task*)lua_touserdata(L, lua_upvalueindex(1));

		auto id = luaL_checkinteger(L, 1);

		auto object = tk->context()->query_io_object(id);

		if (object == nullptr)
		{
			return luaL_error(L, "call bind on closed socket(%d)", id);
		}

		lemon::io::io_socket * socket = object->pointer<lemon::io::io_socket>();

		std::error_code ec;

		auto host = luaL_checkstring(L, 2);

		auto port = luaL_checkstring(L, 3);

		auto addresses = lemon::io::getaddrinfo(host, port, socket->af(), socket->type(), AI_PASSIVE, ec);

		if (ec) {
			object->unref();
			return luaL_error(L, "getaddrinfo(%s:%s) error :%s", host, port, ec.message().c_str());
		}

		for (auto address : addresses) {

			socket->bind(address.addr(), ec);

			if (ec) {
				object->unref();
				return luaL_error(L, "sock(%d) bind(%s:%s) error :%s", id, host, port, ec.message().c_str());
			}
		}

		object->unref();

		return 0;
	}


	static int lsocket_listen(lua_State * L)
	{

		task * tk = (task*)lua_touserdata(L, lua_upvalueindex(1));

		auto id = luaL_checkinteger(L, 1);

		auto object = tk->context()->query_io_object(id);

		if (object == nullptr)
		{
			return luaL_error(L, "call bind on closed socket(%d)", id);
		}

		lemon::io::io_socket * socket = object->pointer<lemon::io::io_socket>();

		std::error_code ec;

		socket->listen(SOMAXCONN, ec);

		object->unref();

		if (ec) 
		{
			return luaL_error(L, "sock(%d) listen error :%s", id, ec.message().c_str());
		}

		return 0;
	}

	static int lsocket_accept(lua_State *L)
	{
		task * tk = (task*)lua_touserdata(L, lua_upvalueindex(1));

		auto id = luaL_checkinteger(L, 1);

		auto object = tk->context()->query_io_object(id);

		if (object == nullptr)
		{
			return luaL_error(L, "call accept on closed socket(%d)", id);
		}
		
		lemon::io::io_socket * socket = object->pointer<lemon::io::io_socket>();

		std::error_code ec;

		auto context = tk->context();

		auto promise_id = context->create_io_promise<io_promise_accept>();

		auto task_id = tk->id();
        lemonD(logger,"task(%d) socket accept ...", task_id);
		socket->accept([=](std::unique_ptr<lemon::io::io_socket> & socket, const lemon::io::address & addr, const std::error_code & ec){
			auto promise = (io_promise_accept*)context->query_io_promise(promise_id);
			if (promise)
			{
				promise->accept(socket, addr, ec);
				promise->unref();
                lemonD(logger,"wake-up task(%d)", task_id);
				context->wakeup(task_id);
			}

		}, ec);

		object->unref();

		if (ec)
		{
			return luaL_error(L, "call accept on socket(%d) error :%s", id, ec.message().c_str());
		}

		return lua_yieldk(L, 0, promise_id, &io_promise::k_func);
	}

	static int lsocket_connect(lua_State *L)
	{
		task * tk = (task*)lua_touserdata(L, lua_upvalueindex(1));

		auto id = luaL_checkinteger(L, 1);

		auto object = tk->context()->query_io_object(id);

		if (object == nullptr)
		{
			return luaL_error(L, "call bind on closed socket(%d)", id);
		}

		lemon::io::io_socket * socket = object->pointer<lemon::io::io_socket>();

		std::error_code ec;

		auto host = luaL_checkstring(L, 2);

		auto port = luaL_checkstring(L, 3);

		auto addresses = lemon::io::getaddrinfo(host, port, socket->af(), socket->type(), AI_PASSIVE, ec);

		if (ec) {
			object->unref();
			return luaL_error(L, "getaddrinfo(%s:%s) error :%s", host, port, ec.message().c_str());
		}

		auto context = tk->context();

		auto promise_id = context->create_io_promise<io_promise_connect>();

		auto task_id = tk->id();

		for (auto address : addresses) 
		{
			socket->connect(address.addr(), [=](const std::error_code& ec) {
				auto promise = (io_promise_connect*)context->query_io_promise(promise_id);
				if (promise)
				{
					promise->connect(ec);
					promise->unref();
					context->wakeup(task_id);
				}
			}, ec);

			if (ec) {
				object->unref();
				return luaL_error(L, "sock(%d) bind(%s:%s) error :%s", id, host, port, ec.message().c_str());
			}

			break;
		}

		object->unref();

		return lua_yieldk(L, 0, promise_id, &io_promise::k_func);
	}


	static int lsocket_recv(lua_State* L)
	{
		task * tk = (task*)lua_touserdata(L, lua_upvalueindex(1));

		auto id = luaL_checkinteger(L, 1);

		auto object = tk->context()->query_io_object(id);

		if (object == nullptr)
		{
			return luaL_error(L, "call bind on closed socket(%d)", id);
		}

		lemon::io::io_socket * socket = object->pointer<lemon::io::io_socket>();

		std::error_code ec;

		auto context = tk->context();

		std::uint32_t promise_id;

		auto promise = context->create_io_promise<io_promise_recv>(promise_id);

		auto task_id = tk->id();

		auto maxlen = (size_t)luaL_checkinteger(L, 2);

		auto buff = lemon::io::buffer { promise->buff(maxlen),maxlen };

		promise->unref();

		int flags = 0;

		if (lua_type(L, 3) == LUA_TNUMBER) {
			flags = (int) luaL_checkinteger(L, 3);
		}

		socket->recv(buff, flags, [=](size_t trans, const std::error_code& ec) {
			auto promise = (io_promise_recv*)context->query_io_promise(promise_id);
			if (promise)
			{
				promise->recv(trans,ec);
				promise->unref();
				context->wakeup(task_id);
			}
		},ec);

		if (ec) {
			object->unref();
			return luaL_error(L, "sock(%d) recv error :%s", id, ec.message().c_str());
		}

		object->unref();

		return lua_yieldk(L, 0, promise_id, &io_promise::k_func);
	}

	static int lsocket_send(lua_State* L)
	{
		task * tk = (task*)lua_touserdata(L, lua_upvalueindex(1));

		auto id = luaL_checkinteger(L, 1);

		auto object = tk->context()->query_io_object(id);

		if (object == nullptr)
		{
			return luaL_error(L, "call bind on closed socket(%d)", id);
		}

		lemon::io::io_socket * socket = object->pointer<lemon::io::io_socket>();

		std::error_code ec;

		auto context = tk->context();

		std::uint32_t promise_id;

		auto promise = context->create_io_promise<io_promise_send>(promise_id);


		auto task_id = tk->id();

		size_t length;

		const char* buff = luaL_checklstring(L, 2, &length);

		int flags = 0;

		if (lua_type(L, 3) == LUA_TNUMBER) {
			flags = (int) luaL_checkinteger(L, 3);
		}

		socket->send( promise->buff({ buff, length }) , flags, [=](size_t trans, const std::error_code& ec) {
			auto promise = (io_promise_send*)context->query_io_promise(promise_id);
			if (promise)
			{
				promise->send(trans, ec);
				promise->unref();
				context->wakeup(task_id);
			}
		}, ec);

		if (ec) {
			object->unref();
			return luaL_error(L, "sock(%d) send error :%s", id, ec.message().c_str());
		}

		object->unref();

		return lua_yieldk(L, 0, promise_id, &io_promise::k_func);
	}


	static luaL_Reg socket_funcs[] = {
		{ "close", lsocket_close },
		{ "bind", lsocket_bind },
		{ "listen", lsocket_listen },
		{ "accept", lsocket_accept },
		{ "send", lsocket_send },
		{ "recv", lsocket_recv },
		{ "connect", lsocket_connect },
		{ NULL, NULL }
	};

	static int lsocket_create(lua_State* L)
	{
		task * tk = (task*)lua_touserdata(L, lua_upvalueindex(1));

		auto socket = new lemon::io::io_socket(tk->context()->io_service(), luaL_checkinteger(L,1), luaL_checkinteger(L, 2), luaL_checkinteger(L, 3));
		
		lua_pushinteger(L,tk->context()->create_io_object(socket));

		if (luaL_newmetatable(L, "iolua_socket"))
		{
			lua_pushstring(L, "__index");
			
			luaL_newlibtable(tk->L(), socket_funcs);

			lua_pushlightuserdata(tk->L(), tk);

			luaL_setfuncs(tk->L(), socket_funcs, 1);

			lua_rawset(L, -3);
		}

		lua_setmetatable(L, -2);

		return 1;
	}

	static int lsocket_open(lua_State* L)
	{
		task * tk = (task*)lua_touserdata(L, lua_upvalueindex(1));

		lua_pushvalue(L, -1);

		if (luaL_newmetatable(L, "iolua_socket"))
		{
			lua_pushstring(L, "__index");

			luaL_newlibtable(tk->L(), socket_funcs);

			lua_pushlightuserdata(tk->L(), tk);

			luaL_setfuncs(tk->L(), socket_funcs, 1);

			lua_rawset(L, -3);
		}

		lua_setmetatable(L, -2);

		return 1;
	}


	static luaL_Reg funcs[] = {
		{ "create", lsocket_create },
		{ "close", lsocket_close },
		{ "bind", lsocket_bind },
		{ "listen", lsocket_listen },
		{ "accept", lsocket_accept },
		{ "send", lsocket_send },
		{ "recv", lsocket_recv },
		{ "connect", lsocket_connect },
		{ "open", lsocket_open },
		{ NULL, NULL }
	};

	static const char * embed_script = "";
		


	void iolua_openio(task * tk)
	{
		luaL_newlibtable(tk->L(), funcs);

		lua_pushlightuserdata(tk->L(), tk);

		luaL_setfuncs(tk->L(), funcs, 1);

		lua_setglobal(tk->L(), "socket");

		if (LUA_OK != luaL_dostring(tk->L(), embed_script)) {
			lemonE(logger, "%s", lua_tostring(tk->L(), -1));
			lua_pop(tk->L(), -1);
		}
	}
}