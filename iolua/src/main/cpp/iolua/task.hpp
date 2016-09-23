#ifndef IOLUA_TASK_HPP
#define IOLUA_TASK_HPP

#include <cstdint>
#include <lua/lua.hpp>
#include <lemon/nocopy.hpp>

namespace iolua {

    class iolua_State;

    enum class task_state
    {
        init = 0, running, sleeping, closed, prevent_sleeping
    };

    class task final : private lemon::nocopy
    {
    public:

        task(iolua_State* context, lua_State *L, std::uint32_t id);

        ~task();

        bool run() ;

        std::uint32_t id() const
        {
            return _id;
        }

        lua_State* L()
        {
            return _L;
        }

        iolua_State * context()
        {
            return _context;
        }


        void set_state(task_state state)
        {
            this->_state = state;
        }

        task_state get_state() const
        {
            return _state;
        }

    private:
        lua_State                   *_L;
        iolua_State                 *_context;
        std::uint32_t               _id;
        task_state                  _state;
    };
}

#endif // IOLUA_TASK_HPP