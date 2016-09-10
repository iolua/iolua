#include "serializer.h"
#include <iolua/ltask/task.hpp>
#include <system_error>
#include <iolua/ltask/ltask_error.hpp>
#include <lemon/log/log.hpp>
#include <iolua/ltask/builtin.hpp>

namespace iolua {
    namespace ltask {

        static auto& console = lemon::log::get("ltask");

        static int init_task(lua_State *L) {
            luaL_openlibs(L);
            const char *filename = (const char *)lua_touserdata(L,1);
            int err = luaL_loadfile(L, filename);
            if (err != LUA_OK)
                lua_error(L);
            return 1;
        }

        task::task(scheduler *sc, lua_State *L, task_id id)
            :_id(id),_state(task_state::init),_scheduler(sc)
        {
            _L = luaL_newstate();

            if (_L == NULL)
            {
                throw std::system_error(make_error_code(errc::out_of_memory));
            }

            ltask_open_libs(this);

            const char * filename = luaL_checkstring(L, 1);
            lua_pushcfunction(_L, init_task);
            lua_pushlightuserdata(_L, (void *)filename);
            int err = lua_pcall(_L, 1, 1, 0);
            if (err != LUA_OK) {
                size_t sz;
                const char * msg = lua_tolstring(_L, -1, &sz);
                if (msg) {
                    lua_pushlstring(L, msg, sz);
                    lua_close(_L);
                    lua_error(L);
                } else {
                    lua_close(_L);
                    throw std::system_error(make_error_code(errc::create_task_error),filename);
                }
            }

            lua_pushcfunction(L, seri_pack);
            lua_insert(L, 2);
            int top = lua_gettop(L);
            lua_call(L, top-2, 1);
            void * args = lua_touserdata(L, 2);
            lua_pushcfunction(_L, seri_unpack);
            lua_pushlightuserdata(_L, args);
            err = lua_pcall(_L, 1, LUA_MULTRET, 0);
            if (err != LUA_OK) {
                lua_close(_L);
                throw std::system_error(make_error_code(errc::create_task_error),"pass argument to new task error");
            }
        }

        task::~task()
        {
            lua_close(_L);
        }

        void task::run() noexcept
        {
            _resume = false;

            int args = lua_gettop(_L) - 1;
            int ret = lua_resume(_L, NULL, args);
            if (ret == LUA_YIELD) {
                lua_settop(_L, 0);
                _state = task_state::sleeping;
                return;
            }


            if (ret != LUA_OK) {
                // todo: call luaL_traceback
                lemonE(console, "resume task(%d) error :%s",_id, lua_tostring(_L,-1));
            }

            _state = task_state::closed;
        }
    }
}