#include <memory>
#include <iolua/task.hpp>
#include <iolua/libpipe.hpp>

namespace iolua {


    static int l_read(lua_State* L)
    {
        task * tk = (task*)lua_touserdata(L, lua_upvalueindex(1));

        auto id = luaL_checkinteger(L, 1);

        auto object = tk->context()->query_io_object(id);

        if (object == nullptr)
        {
            return luaL_error(L, "call read on closed pipe(%d)", id);
        }

        lemon::io::io_stream * stream = object->pointer<lemon::io::io_stream>();

        std::error_code ec;

        auto context = tk->context();

        std::uint32_t promise_id;

        auto promise = context->create_io_promise<io_promise_recv>(promise_id);

        auto task_id = tk->id();

        auto maxlen = (size_t)luaL_checkinteger(L, 2);

        auto buff = lemon::io::buffer { promise->buff(maxlen),maxlen };

        promise->unref();

        stream->read(buff, [=](size_t trans, const std::error_code& ec) {
            auto promise = (io_promise_recv*)context->query_io_promise(promise_id);
            if (promise)
            {
                promise->recv(trans,ec);
                promise->unref();
                context->wake_up(task_id);
            }
        },ec);

        if (ec) {
            object->unref();
            return luaL_error(L, "pipe(%d) read error :%s", id, ec.message().c_str());
        }

        object->unref();

        return lua_yieldk(L, 0, promise_id, &io_promise::k_func);
    }

    static int lpipe_write(lua_State* L)
    {
        task * tk = (task*)lua_touserdata(L, lua_upvalueindex(1));

        auto id = luaL_checkinteger(L, 1);

        auto object = tk->context()->query_io_object((uint32_t) id);

        if (object == nullptr)
        {
            return luaL_error(L, "call bind on closed pipe(%d)", id);
        }

        lemon::io::io_stream * stream = object->pointer<lemon::io::io_stream>();

        std::error_code ec;

        auto context = tk->context();

        std::uint32_t promise_id;

        auto promise = context->create_io_promise<io_promise_send>(promise_id);

        auto task_id = tk->id();

        size_t length;

        const char* buff = luaL_checklstring(L, 2, &length);

        stream->write( promise->buff({ buff, length }) , [=](size_t trans, const std::error_code& ec) {
            auto promise = (io_promise_send*)context->query_io_promise(promise_id);
            if (promise)
            {
                promise->send(trans, ec);
                promise->unref();
                context->wake_up(task_id);
            }
        }, ec);

        promise->unref();

        object->unref();

        if (ec) {

            return luaL_error(L, "pipe(%d) write error :%s", id, ec.message().c_str());
        }

        return lua_yieldk(L, 0, promise_id, &io_promise::k_func);
    }

    static int lpipe_create(lua_State * L)
    {
        task * tk = (task*)lua_touserdata(L, lua_upvalueindex(1));

        std::uint32_t in, out;

        if(lua_gettop(L) > 0)
        {
            auto name = luaL_checkstring(L,1);

            lemon::io::pipe pipe(tk->context()->io_service(),name);

            in = tk->context()->create_io_object(pipe.release_in());

            out = tk->context()->create_io_object(pipe.release_out());
        }
        else
        {
            lemon::io::pipe pipe(tk->context()->io_service());

            in = tk->context()->create_io_object(pipe.release_in());

            out = tk->context()->create_io_object(pipe.release_out());
        }

        lua_pushinteger(L,in);
        lua_pushinteger(L,out);

        return 2;
    }

    static int l_pipe_close(lua_State *L)
    {
        task * tk = (task*)lua_touserdata(L, lua_upvalueindex(1));

        int N = lua_gettop(L);

        for(int i = 1; i <= N; i ++)
        {
            tk->context()->close_io_object((uint32_t) luaL_checkinteger(L, i));
        }

        return 0;
    }

    static luaL_Reg funcs[] = {
            {"create",lpipe_create},
            { "read", l_read},
            { "write", lpipe_write},
            { "close", l_pipe_close},
            { NULL, NULL }
    };

    void iolua_openpipe(task * tk)
    {
        luaL_newlibtable(tk->L(), funcs);

        lua_pushlightuserdata(tk->L(), tk);

        luaL_setfuncs(tk->L(), funcs, 1);

        lua_setglobal(tk->L(), "pipe");
    }
}
