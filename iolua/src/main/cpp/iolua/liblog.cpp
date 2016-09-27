#include <vector>
#include <iolua/liblog.hpp>
#include <iolua/task.hpp>

namespace iolua {

    static auto& logger = lemon::log::get("iolua");

    static int logger_write(lua_State *L, lemon::log::level level)
    {
        int n = lua_gettop(L) - 1;

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

		lua_Debug debug = { 0 };

		lua_getstack(L, 1, &debug);

		lua_getinfo(L, "lS", &debug);

        auto file = lemon::fs::filepath(debug.source + 1).filename();

		auto l = (lemon::log::logger*)luaL_checkudata(L, 1, "iolua_logger");

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
        auto l = &lemon::log::get(luaL_checkstring(L,1));

        lua_pushlightuserdata(L,(void*)l);

        if(luaL_newmetatable(L, "iolua_logger"))
        {
            lua_pushstring(L, "__index");
            luaL_newlib(L, logger_funcs);
            lua_rawset(L,-3);
        }

        lua_setmetatable(L,-2);

        return 1;
    }

    static std::vector<std::string> parse_sources(lua_State *L,int offset)
    {
        auto sources_count = lua_gettop(L);

        if (sources_count < offset) return {};

        std::vector<std::string> sources;

        for(int i = offset; i <= sources_count; i ++ )
        {
            sources.push_back(luaL_checkstring(L,i));
        }

        return sources;
    }

    static std::uint32_t parse_flags(lua_State *L,int args)
    {
        auto sources_count = lua_gettop(L);

        if (sources_count < args) return 0;

        // flags
        // t: timestamp
        // s: source
        // f: file_lines
        // l: level
        auto flags_string = std::string(luaL_checkstring(L,args));

        std::uint32_t flags = 0;

        if(flags_string.find("t") != std::string::npos)
        {
            flags |= (std::uint32_t)lemon::log::display_flag::timestamp;
        }

        if(flags_string.find("s") != std::string::npos)
        {
            flags |= (std::uint32_t)lemon::log::display_flag::source;
        }

        if(flags_string.find("f") != std::string::npos)
        {
            flags |= (std::uint32_t)lemon::log::display_flag::file_lines;
        }

        if(flags_string.find("l") != std::string::npos)
        {
            flags |= (std::uint32_t)lemon::log::display_flag::level;
        }

        return flags;
    }

    static int logger_open_console(lua_State *L)
    {
        std::unique_ptr<lemon::log::sink> sink(new lemon::log::console(parse_sources(L,2)));

        sink->set_display_flags(parse_flags(L,1));

        lemon::log::add_sink(std::move(sink));

        return 0;
    }

    static int logger_set_levels(lua_State *L)
    {
        auto flags_string = std::string(luaL_checkstring(L,1));

        int flags = 0;

        if(flags_string.find("e") != std::string::npos)
        {
            flags |= (int)lemon::log::level ::error;
        }

        if(flags_string.find("w") != std::string::npos)
        {
            flags |= (int)lemon::log::level ::warn;
        }

        if(flags_string.find("i") != std::string::npos)
        {
            flags |= (int)lemon::log::level ::info;
        }

        if(flags_string.find("d") != std::string::npos)
        {
            flags |= (int)lemon::log::level ::debug;
        }

        if(flags_string.find("t") != std::string::npos)
        {
            flags |= (int)lemon::log::level ::trace;
        }

        if(flags_string.find("v") != std::string::npos)
        {
            flags |= (int)lemon::log::level ::verbose;
        }

        auto sources = parse_sources(L,2);

        lemon::log::set_levels(flags,sources);

        return 0;
    }

    static int logger_open_file_sink(lua_State *L)
    {

        auto dir = luaL_checkstring(L,1);
        auto name = luaL_checkstring(L,2);

        auto maxsize = (std::uintmax_t )-1;

        auto time_suffix = true;

        auto offset =  3;

        if(lua_type(L,3) == LUA_TNUMBER)
        {
            maxsize = (std::uintmax_t)luaL_checkstring(L, 3);

            offset = 4;

            if(lua_type(L,4) == LUA_TBOOLEAN)
            {
                time_suffix = (bool)lua_toboolean(L, 4);

                offset = 5;
            }
        }
        else if(lua_type(L,3) == LUA_TBOOLEAN)
        {
            time_suffix = (bool)lua_toboolean(L, 3);

            offset = 4;
        }

        auto sources = parse_sources(L,offset);

        std::unique_ptr<lemon::log::sink> sink(new lemon::log::file_sink(sources,dir,name));

        reinterpret_cast<lemon::log::file_sink*>(sink.get())->time_suffix(time_suffix);
        reinterpret_cast<lemon::log::file_sink*>(sink.get())->max_size(maxsize);

        lemon::log::add_sink(std::move(sink));

        return 0;
    }

    static luaL_Reg funcs[] = {

            { "open", lopen_log },

            { "console", logger_open_console },

            { "file", logger_open_file_sink },

            { "level", logger_set_levels },

            { NULL, NULL }
    };

    void iolua_openlog(task * tk)
    {
        luaL_newlibtable(tk->L(),funcs);

        lua_pushlightuserdata(tk->L(),tk);
        luaL_setfuncs(tk->L(),funcs,1);
        lua_setglobal(tk->L(),"log");
    }
}