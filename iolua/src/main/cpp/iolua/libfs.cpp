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
        std::error_code ec;

        lemon::fs::create_directory(luaL_checkstring(L,1),ec);

        if(ec)
        {
            return luaL_error(L,"create directory :%s\nerror :%s",luaL_checkstring(L,1),ec.message().c_str());
        }

        return 0;
    }

    static int fs_file_size(lua_State* L)
    {
        std::error_code ec;

        auto filesize = lemon::fs::file_size(luaL_checkstring(L,1),ec);

        if(ec)
        {
            return luaL_error(L,"create directory :%s\nerror :%s",luaL_checkstring(L,1),ec.message().c_str());
        }

        lua_pushinteger(L, (lua_Integer) filesize);

        return 1;
    }


    static int fs_create_symlink(lua_State* L)
    {
        std::error_code ec;

        auto from  = luaL_checkstring(L,1);

        auto to = luaL_checkstring(L,2);

        lemon::fs::create_symlink(from, to, ec);

        if(ec)
        {
            return luaL_error(L,"create symlink \n\tfrom:%s\n\tto:%s\nerror :%s", from, to, ec.message().c_str());
        }

        return 0;
    }

    static int fs_copy_file(lua_State* L)
    {
        std::error_code ec;

        auto from  = luaL_checkstring(L,1);

        auto to = luaL_checkstring(L,2);

        lemon::fs::copy_file(from, to, ec);

        if(ec)
        {
            return luaL_error(L,"copy file \n\tfrom:%s\n\tto:%s\nerror :%s", from, to, ec.message().c_str());
        }

        return 0;
    }

    static int fs_is_directory(lua_State* L)
    {
        std::error_code ec;

        if(lemon::fs::is_directory(luaL_checkstring(L,1)))
        {
            lua_pushboolean(L, 1);
        }
        else
        {
            lua_pushboolean(L, 2);
        }

        return 1;
    }

    static int fs_file_type(lua_State* L)
    {
        std::error_code ec;

        auto status = lemon::fs::status(luaL_checkstring(L,1), ec);

        if(ec)
        {
            return luaL_error(L,"get file status \n\t%s\n\terror :%s", luaL_checkstring(L,1), ec.message().c_str());
        }

        switch (status.type())
        {

            case lemon::fs::file_type::none:
                lua_pushstring(L,"none");
                break;
            case lemon::fs::file_type::not_found:
                lua_pushstring(L,"not_found");
                break;
            case lemon::fs::file_type::regular:
                lua_pushstring(L,"regular");
                break;
            case lemon::fs::file_type::directory:
                lua_pushstring(L,"directory");
                break;
            case lemon::fs::file_type::symlink:
                lua_pushstring(L,"symlink");
                break;
            case lemon::fs::file_type::block:
                lua_pushstring(L,"block");
                break;
            case lemon::fs::file_type::character:
                lua_pushstring(L,"character");
                break;
            case lemon::fs::file_type::fifo:
                lua_pushstring(L,"fifo");
                break;
            case lemon::fs::file_type::socket:
                lua_pushstring(L,"socket");
                break;
            case lemon::fs::file_type::unknown:
                lua_pushstring(L,"unknown");
                break;
        }


        return 1;
    }

    static int fs_remove_file(lua_State* L)
    {
        std::error_code ec;

        lemon::fs::remove_file(luaL_checkstring(L,1), ec);

        if(ec)
        {
            return luaL_error(L,"remove file \n\t%s\n\terror :%s", luaL_checkstring(L,1), ec.message().c_str());
        }

        return 0;
    }


    static int fs_native_path(lua_State* L)
    {

        int n = lua_gettop(L);

        if(0 == n) {
            return 0;
        }

        lemon::fs::filepath path;

        for(int i =1; i <= n ; i ++)
        {
            path.append(lemon::fs::filepath(luaL_checkstring(L,i)));
        }

        lua_pushstring(L,path.string().c_str());

        return 1;
    }

    static int fs_generic_path(lua_State* L)
    {

        int n = lua_gettop(L);

        if(0 == n) {
            return 0;
        }

        lemon::fs::filepath path;

        for(int i =1; i <= n ; i ++)
        {
            path.append(lemon::fs::filepath(luaL_checkstring(L,i)));
        }

        lua_pushstring(L,path.generic_string().c_str());

        return 1;
    }

    static luaL_Reg funcs[] = {
            { "current_path", fs_current_path },
            { "exists", fs_exists },
            { "create_directory", fs_create_directory },
            { "symlink", fs_create_symlink },
            { "is_directory", fs_is_directory },
            { "remove_file", fs_remove_file },
            { "copy_file", fs_copy_file },
            { "file_size", fs_file_size },
            { "file_type", fs_file_type },
            { "path", fs_native_path },
            { "generic_path", fs_generic_path },
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