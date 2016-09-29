#include <iolua/iolua.hpp>
#include <iolua/task.hpp>
#include <lemon/os/os.hpp>

namespace iolua {

    auto & logger = lemon::log::get("exec");

    static int lexec_wait(lua_State *L)
    {
        task * tk = (task*)lua_touserdata(L, lua_upvalueindex(1));

        auto task_id = tk->id();

        auto context = tk->context();

        auto exec = (lemon::os::exec*)luaL_checkudata(L, 1, "iolua_exec");

        auto promise_id = tk->context()->create_io_promise<io_promise_exec_wait>();

        exec->wait([=](int code, std::error_code & ec){
            auto promise = (io_promise_exec_wait*)context->query_io_promise(promise_id);
            if (promise)
            {
                promise->wait(code, ec);
                promise->unref();
                context->wake_up(task_id);
            }
        });

        return lua_yieldk(L, 0, promise_id, &io_promise::k_func);
    }

    static int lexec_start(lua_State *L)
    {
        auto exec = (lemon::os::exec*)luaL_checkudata(L, 1, "iolua_exec");

        int args = lua_gettop(L);

        std::vector<std::string> argv;

        for(int i =  2; i <= args; i ++)
        {
            argv.push_back(luaL_checkstring(L,i));
        }

        try
        {
            exec->start(argv);
        }

        catch(std::system_error & e)
        {
            luaL_error(L, "exec(%p) start error :%s",exec,e.what());
        }

        return 0;
    }

    static int lexec_pipe(lua_State *L)
    {
        task * tk = (task*)lua_touserdata(L, lua_upvalueindex(1));

        auto exec = (lemon::os::exec*)luaL_checkudata(L, 1, "iolua_exec");

        auto flags_string = std::string(luaL_checkstring(L,2));

        int counter = 0;

        for(auto c : flags_string )
        {
            switch(c)
            {
                case 'i':
                    lua_pushinteger(L, tk->context()->create_io_object(exec->release_in()));
                    counter ++;
                    break;
                case 'o':
                    lua_pushinteger(L, tk->context()->create_io_object(exec->release_out()));
                    counter ++;
                    break;
                case 'e':
                    lua_pushinteger(L, tk->context()->create_io_object(exec->release_err()));
                    counter ++;
                    break;
                default:
                    break;
            }
        }

        return counter;
    }

    static luaL_Reg exec_funcs[] = {

            { "wait", lexec_wait },

            { "start", lexec_start },

            { "pipe", lexec_pipe },

            { NULL, NULL }
    };

    static int lexec_close(lua_State *L)
    {
        auto exec = (lemon::os::exec*)luaL_checkudata(L, 1, "iolua_exec");
        lemonD(logger,"close exec(%p)",exec);
        exec->~exec();
        lemonD(logger,"close exec(%p) -- success",exec);

        return 0;
    }


    static int lexec_create(lua_State *L)
    {
        auto name = luaL_checkstring(L, 1);

        int flags = 0;

        if(lua_type(L,2) == LUA_TSTRING)
        {
            auto flags_string = std::string(luaL_checkstring(L,2));

            if(flags_string.find("i") != std::string::npos)
            {
                flags |= (int)lemon::os::exec_options::pipe_in;
            }

            if(flags_string.find("o") != std::string::npos)
            {
                flags |= (int)lemon::os::exec_options::pipe_out;
            }

            if(flags_string.find("e") != std::string::npos)
            {
                flags |= (int)lemon::os::exec_options::pipe_error;
            }
        }



        task * tk = (task*)lua_touserdata(L, lua_upvalueindex(1));

        void * block = lua_newuserdata(L,sizeof(lemon::os::exec));

        new (block) lemon::os::exec(tk->context()->io_service(), name,flags);

        if (luaL_newmetatable(L, "iolua_exec"))
        {
            lua_pushstring(L, "__index");

            luaL_newlibtable(tk->L(), exec_funcs);

            lua_pushlightuserdata(tk->L(), tk);

            luaL_setfuncs(tk->L(), exec_funcs, 1);

            lua_rawset(L, -3);

            lua_pushstring(L, "__gc");

            lua_pushcfunction(L, lexec_close);

            lua_rawset(L, -3);
        }

        lua_setmetatable(L, -2);

        return 1;
    }

    static luaL_Reg funcs[] = {
            { "create", lexec_create },
            { NULL, NULL }
    };

    void iolua_open_exec(task * tk)
    {
        luaL_newlibtable(tk->L(), funcs);

        lua_pushlightuserdata(tk->L(), tk);

        luaL_setfuncs(tk->L(), funcs, 1);

        lua_setglobal(tk->L(), "exec");
    }
}

