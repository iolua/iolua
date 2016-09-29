#include <iolua/iolua.hpp>
#include <iolua/task.hpp>
#include <lemon/log/log.hpp>
namespace iolua {

    static auto& logger = lemon::log::get("iolua");

    static int task_create(lua_State *L)
    {
        task * tk = (task*)lua_touserdata(L,lua_upvalueindex(1));

        lua_pushinteger(L,tk->context()->create_task(L));

        return 1;
    }

    static int task_self(lua_State *L)
    {
        task * tk = (task*)lua_touserdata(L,lua_upvalueindex(1));

        lua_pushinteger(L, tk->id());

        return 1;
    }

    static int task_sleep(lua_State *L)
    {
        task * tk = (task*)lua_touserdata(L,lua_upvalueindex(1));

        auto task_id = tk->id();

        auto context = tk->context();

        auto &timer_wheel = tk->context()->timer_wheel();

        auto promise_id = context->create_io_promise<io_promise>();

        timer_wheel.create_timer([=](){

            auto promise = context->query_io_promise(promise_id);

            if (promise)
            {
                promise->unref();
                context->wakeup(task_id);
            }

        },std::chrono::milliseconds(luaL_checkinteger(L,1)));

        return lua_yieldk(L, 0, promise_id, &io_promise::k_func);
    }

    static luaL_Reg funcs[] = {

            { "create", task_create },
            { "id", task_self },
            { "sleep", task_sleep },
            { NULL, NULL }
    };

    static const char * embed_script = "";

    void iolua_opentask(task * tk)
    {
        luaL_openlibs(tk->L());

        luaL_newlibtable(tk->L(),funcs);

        lua_pushlightuserdata(tk->L(),tk);
        luaL_setfuncs(tk->L(),funcs,1);
        lua_setglobal(tk->L(),"task");

        if (LUA_OK != luaL_dostring(tk->L(),embed_script)) {
            lemonE(logger,"%s",lua_tostring(tk->L(),-1));
            lua_pop(tk->L(),-1);
        }
    }
}