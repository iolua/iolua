#include <lemon/io/io.hpp>
#include <iolua/ltask/builtin.hpp>
#include <iolua/ltask/scheduler.hpp>
#include <iolua/ltask/serializer.h>
#include <lemon/log/log.hpp>

namespace iolua {
    namespace ltask {

        static auto& logger = lemon::log::get("ltask");

        int ltask_open(lua_State *L) {

            task * tk = (task*)lua_touserdata(L,lua_upvalueindex(1));

            lua_pushinteger(L,tk->owner()->create_task(L));

            return 1;
        }

        int ltask_chan_open(lua_State *L) {
            task * tk = (task*)lua_touserdata(L,lua_upvalueindex(1));

            lua_pushinteger(L,tk->owner()->create_channel(L));

            return 1;
        }

        int ltask_chan_close(lua_State *L) {
            task * tk = (task*)lua_touserdata(L,lua_upvalueindex(1));

            tk->owner()->close_channel((uint32_t)luaL_checkinteger(L,1));

            return 0;
        }

        int ltask_chan_send(lua_State *L) {

            uint32_t id = (uint32_t)luaL_checkinteger(L, 1);

            task * tk = (task*)lua_touserdata(L,lua_upvalueindex(1));

            auto c = tk->owner()->get_channel(id);

            if (c == nullptr) {
                return 0;
            }

            lua_pushcfunction(L, seri_pack);
            lua_replace(L, 1);
            int top = lua_gettop(L);
            lua_call(L, top-1, 1);
            void * msg = lua_touserdata(L, 1);

            c->push(msg);

            lua_pushboolean(L,1);

            return 1;
        }

        int ltask_chan_recv(lua_State *L) {

            uint32_t id = (uint32_t)luaL_checkinteger(L, 1);

            task * tk = (task*)lua_touserdata(L,lua_upvalueindex(1));

            auto c = tk->owner()->get_channel(id);

            if (c == nullptr) {
                return 0;
            }

            auto msg  = c->pop();

            c->close();

            if (msg == nullptr) {
                return 0;
            }

            lua_settop(L, 0);

            lua_pushboolean(L, 1);
            lua_pushcfunction(L, seri_unpack);
            lua_pushlightuserdata(L, msg);
            lua_call(L, 1, LUA_MULTRET);

            return lua_gettop(L);
        }

        int ltask_chan_select(lua_State *L) {

            task * tk = (task*)lua_touserdata(L,lua_upvalueindex(1));

            int n = lua_gettop(L);

            std::vector<uint32_t> channels(n);

            for (int i=0; i<n; i++) {
                channels[i] = (uint32_t)luaL_checkinteger(L, i+1);
            }

            lua_pushinteger(L, tk->owner()->channel_select(tk->id(),&channels[0],n));

            return 1;
        }

		int ltask_socket(lua_State * L) {

			task * tk = (task*)lua_touserdata(L, lua_upvalueindex(1));

			auto & io_objects = tk->owner()->io_objects();

			lua_pushinteger(L,io_objects.create_socket(luaL_checkinteger(L, 1), luaL_checkinteger(L, 2),luaL_checkinteger(L, 3)));

			return 1;
		}

		int ltask_bind(lua_State * L) {
			task * tk = (task*)lua_touserdata(L, lua_upvalueindex(1));

			auto & io_objects = tk->owner()->io_objects();

			auto id = luaL_checkinteger(L, 1);

			auto sock = io_objects.get_socket(id);

			if (sock == nullptr) {

				luaL_error(L, "closed sock handler(%d)", id);

				return 0;
			}

			std::error_code ec;

			auto host = luaL_checkstring(L,2);

			auto port = luaL_checkstring(L, 3);

			auto addresses = lemon::io::getaddrinfo(host, port, (*sock)->af(),(*sock)->type(), AI_PASSIVE,ec);

			if(ec) {
				return luaL_error(L, "getaddrinfo(%s:%s) error :%s",host, port, ec.message().c_str());
			}

			for( auto address : addresses) {
				
				(*sock)->bind(address.addr(),ec);

				if(ec) {
					return luaL_error(L, "sock(%d) bind(%s:%s) error :%s", id, host, port, ec.message().c_str());
				}
			}

			return 0;
		}

		int ltask_listen(lua_State * L) {

			task * tk = (task*)lua_touserdata(L, lua_upvalueindex(1));

			auto & io_objects = tk->owner()->io_objects();

			auto id = luaL_checkinteger(L, 1);

			auto sock = io_objects.get_socket(id);

			if (sock == nullptr) {

				luaL_error(L, "closed sock handler(%d)",id);

				return 0;
			}

			std::error_code ec;

			(*sock)->listen(SOMAXCONN, ec);

			return 0;
		}

		int ltask_connect(lua_State * L) {
			task * tk = (task*)lua_touserdata(L, lua_upvalueindex(1));

			auto & io_objects = tk->owner()->io_objects();

			auto id = luaL_checkinteger(L, 1);

			auto sock = io_objects.get_socket(id);

			if (sock == nullptr) {

				luaL_error(L, "closed sock handler(%d)", id);

				return 0;
			}

			std::error_code ec;

			auto host = luaL_checkstring(L, 2);

			auto port = luaL_checkstring(L, 3);

			auto addresses = lemon::io::getaddrinfo(host, port, (*sock)->af(), (*sock)->type(), AI_PASSIVE, ec);

			if (ec) {
				return luaL_error(L, "getaddrinfo(%s:%s) error :%s", host, port, ec.message().c_str());
			}

			auto io_promies = &tk->owner()->io_promises();

			auto taskid = tk->id();
			auto sc = tk->owner();

			for (auto address : addresses) {

				auto promiseid = io_promies->create_connect_io_promise();

				(*sock)->connect(address.addr(),[=](const std::error_code& ec){
					auto promise = io_promies->get_promise(promiseid);

					if(promise) {
						reinterpret_cast<connect_io_promise*>(promise)->complete(ec);

						sc->resume(taskid);
					}
				},ec);

				if (ec) {
					io_promies->close_promise(promiseid);

					return luaL_error(L, "sock(%d) bind(%s:%s) error :%s", id, host, port, ec.message().c_str());
				}

				lua_pushinteger(L, promiseid);
			}

			return 1;
		}

		static int ltask_wait(lua_State * L)
		{
			task * tk = (task*)lua_touserdata(L, lua_upvalueindex(1));

			auto promiseId = luaL_checkinteger(L, 1);

			auto promise = tk->owner()->io_promises().get_promise(promiseId);

			if(promise)
			{
				auto ret = promise->raise_promise(L);

				tk->owner()->io_promises().close_promise(promiseId);

				return ret;
			}

			return 0;
		}

        static luaL_Reg funcs[] = {

                { "task",ltask_open } ,

                { "chan",ltask_chan_open } ,

                { "close",ltask_chan_close } ,

                { "send",ltask_chan_send } ,

                { "raw_recv",ltask_chan_recv } ,

                { "select",ltask_chan_select } ,

				{ "sock",ltask_socket } ,

				{ "listen",ltask_listen } ,

				{ "bind",ltask_bind } ,

				{ "raw_connect",ltask_connect } ,

				{ "wait",ltask_wait } ,

                { NULL, NULL }
        };

        static const char * recv_method =
                "iolua.recv = function(...)\n"
                        "while true do\n"
                            "local id = iolua.select(...)\n"
                            "if id ~= 0 then\n"
                                "local ret = table.pack(iolua.raw_recv(id))\n"
                                "if ret[1] then return table.unpack(ret,2) end\n"
                            "end\n"
                            "coroutine.yield()\n"
                        "end\n"
                "end\n"
				"iolua.connect = function(...)\n"
					"local future = iolua.raw_connect(...)\n"
					"coroutine.yield()\n"
					"return iolua.wait(future)\n"
				"end\n";;

        void ltask_open_libs(task * tk)
        {
            luaL_openlibs(tk->L());

            luaL_newlibtable(tk->L(),funcs);

            lua_pushlightuserdata(tk->L(),tk);
            luaL_setfuncs(tk->L(),funcs,1);
            lua_setglobal(tk->L(),"iolua");

            if (LUA_OK != luaL_dostring(tk->L(),recv_method)) {
                lemonE(logger,"%s",lua_tostring(tk->L(),-1));
                lua_pop(tk->L(),-1);
            }
        }
    }
}