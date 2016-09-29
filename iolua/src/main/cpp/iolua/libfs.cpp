#include <iolua/libfs.hpp>
#include <iolua/task.hpp>
#include <lemon/fs/fs.hpp>
namespace iolua {

    static int fs_current_path(lua_State* L)
    {

        std::error_code ec;

        auto current_path = lemon::fs::current_path(ec);

        if(ec)
        {
            return luaL_error(L,"get current path error :%s",ec.message().c_str());
        }

        lua_pushstring(L, current_path.string().c_str());

        return 1;
    }

    static int fs_exists(lua_State* L)
    {

        if(lemon::fs::exists(luaL_checkstring(L,1)))
        {
            lua_pushboolean(L,1);
        }
        else
        {
            lua_pushboolean(L,0);
        }

        return 1;
    }

    static int fs_create_directory(lua_State* L)
    {
        return 1;
    }

    static luaL_Reg funcs[] = {
            { "current_path", fs_current_path },
            { "exists", fs_exists },
            { "create_directory", fs_create_directory },
            { NULL, NULL }
    };

    void iolua_open_fs(task * tk)
    {
        luaL_newlibtable(tk->L(), funcs);

        lua_pushlightuserdata(tk->L(), tk);

        luaL_setfuncs(tk->L(), funcs, 1);

        lua_setglobal(tk->L(), "fs");
    }
}