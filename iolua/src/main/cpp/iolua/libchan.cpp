#include <iolua/libchan.hpp>
#include <iolua/task.hpp>
#include <lemon/log/log.hpp>
#include <iolua/serializer.h>

namespace iolua {
	static auto& logger = lemon::log::get("console");

	static int lchan_open(lua_State *L)
	{
		task * tk = (task*)lua_touserdata(L, lua_upvalueindex(1));

		lua_pushinteger(L,tk->context()->create_channel());

		return 1;
	}

	static int lchan_close(lua_State *L)
	{
		task * tk = (task*)lua_touserdata(L, lua_upvalueindex(1));

		tk->context()->close_channel((std::uint32_t)luaL_checkinteger(L, 1));

		return 1;
	}
	static int do_recv(lua_State *L, int, lua_KContext ctx);

	static int do_recv(lua_State *L, int, lua_KContext)
	{
		task * tk = (task*)lua_touserdata(L, lua_upvalueindex(1));

		uint32_t id = (uint32_t)luaL_checkinteger(L, 1);

		auto chan = tk->context()->query_channel(id);

		if (chan == nullptr)
		{
			return luaL_error(L, "call recv on closed channel(%d)", id);
		}

		while (chan->do_select(tk->id()))
		{
			void * message = chan->read_message();

			if (message != nullptr)
			{

				lua_settop(L, 0);

				lua_pushboolean(L, 1);
				lua_pushcfunction(L, seri_unpack);
				lua_pushlightuserdata(L, message);
				lua_call(L, 1, LUA_MULTRET);

				return lua_gettop(L);
			}
		}

		chan->unref();

		return lua_yieldk(L, 0, 0, &do_recv);
	}

	static int lchan_recv(lua_State *L)
	{
		return do_recv(L, 0, 0);
	}

	static int lchan_send(lua_State *L)
	{
		uint32_t id = (uint32_t)luaL_checkinteger(L, 1);

		task * tk = (task*)lua_touserdata(L, lua_upvalueindex(1));

		auto c = tk->context()->query_channel(id);

		if (c == nullptr) {
			return luaL_error(L, "call recv on closed channel(%d)", id);
		}

		lua_pushcfunction(L, seri_pack);
		lua_replace(L, 1);
		int top = lua_gettop(L);
		lua_call(L, top - 1, 1);
		void * msg = lua_touserdata(L, 1);

		c->write_message(msg);
		lua_pushboolean(L, 1);
		c->unref();

		return 1;
	}

	static int do_select(lua_State *L, int, lua_KContext);

	static int do_select(lua_State *L, int, lua_KContext)
	{
		task * tk = (task*)lua_touserdata(L, lua_upvalueindex(1));

		int n = lua_gettop(L);

		for (int i = 0; i < n; i++) {
			auto channel = tk->context()->query_channel((uint32_t)luaL_checkinteger(L, i + 1));

			if (channel)
			{
				if (channel->do_select(tk->id()))
				{
					lua_pushinteger(L, channel->id());
					channel->unref();
					return 1;
				}

				channel->unref();
			}
		}

		return lua_yieldk(L, 0, 0, &do_select);
	}

	static int lchan_select(lua_State *L)
	{
		return do_select(L, 0, 0);
	}

	static luaL_Reg funcs[] = {

		{ "create", lchan_open },

		{ "close", lchan_close },

		{ "send", lchan_send },

		{ "recv", lchan_recv },

		{ "select", lchan_select },

		{ NULL, NULL }
	};

	void iolua_open_chan(task *tk)
	{
		luaL_newlibtable(tk->L(), funcs);

		lua_pushlightuserdata(tk->L(), tk);
		
		luaL_setfuncs(tk->L(), funcs, 1);
		
		lua_setglobal(tk->L(), "chan");

	}
}