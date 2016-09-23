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

    static luaL_Reg funcs[] = {

            { "create", task_create },
            { "id", task_self },
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