#include <iolua/liblog.hpp>
#include <lemon/log/log.hpp>
#include <iolua/task.hpp>

namespace iolua {

    static auto& logger = lemon::log::get("iolua");

    static int logger_write(lua_State *L, lemon::log::level level)
    {
        int n = lua_gettop(L) - 1;

        auto l = (lemon::log::logger*)luaL_checkudata(L,1,"iolua_logger");

        lua_getglobal(L,"string");

        lua_pushstring(L, "format");

        lua_gettable(L,-2);

        lua_insert(L,-n - 2);

        lua_pop(L,1);

        if(lua_pcall(L,n,1,0) != LUA_OK)
        {
            return luaL_error(L,"call string.format error :%s",lua_tostring(L,-1));
        }

        auto msg = lua_tostring(L,-1);

        lua_Debug debug;

        lua_getstack(L,3, &debug);

        lua_getinfo(L,"lS", &debug);

        auto file = lemon::fs::filepath(debug.source + 1).filename();

        l->write(level, std::string(msg),file.string().c_str(),debug.currentline);

        return 0;
    }

    static int logger_error(lua_State *L)
    {
        return logger_write(L, lemon::log::level::error);
    }

    static int logger_warn(lua_State *L)
    {
        return logger_write(L, lemon::log::level::warn);
    }

    static int logger_info(lua_State *L)
    {
        return logger_write(L, lemon::log::level::info);
    }

    static int logger_debug(lua_State *L)
    {
        return logger_write(L, lemon::log::level::debug);
    }

    static int logger_trace(lua_State *L)
    {
        return logger_write(L, lemon::log::level::trace);
    }

    static int logger_verbose(lua_State *L)
    {
        return logger_write(L, lemon::log::level::verbose);
    }

    static luaL_Reg logger_funcs[] = {
            { "error", logger_error },
            { "warn", logger_warn },
            { "info", logger_info },
            { "debug", logger_debug },
            { "trace", logger_trace },
            { "verb", logger_verbose },
            { NULL, NULL }
    };

    static int lopen_log(lua_State *L)
    {
        auto logger = &lemon::log::get(luaL_checkstring(L,1));

        lua_pushlightuserdata(L,(void*)logger);

        if(luaL_newmetatable(L, "iolua_logger"))
        {
            lua_pushstring(L, "__index");
            luaL_newlib(L, logger_funcs);
            lua_rawset(L,-3);
        }

        lua_setmetatable(L,-2);

        return 1;
    }

    static luaL_Reg funcs[] = {

            { "open", lopen_log },

            { NULL, NULL }
    };

    static const char * embed_script = "";

    void iolua_openlog(task * tk)
    {
        luaL_newlibtable(tk->L(),funcs);

        lua_pushlightuserdata(tk->L(),tk);
        luaL_setfuncs(tk->L(),funcs,1);
        lua_setglobal(tk->L(),"log");

        if (LUA_OK != luaL_dostring(tk->L(),embed_script)) {
            lemonE(logger,"%s",lua_tostring(tk->L(),-1));
            lua_pop(tk->L(),-1);
        }
    }
}