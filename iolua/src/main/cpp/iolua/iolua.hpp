#ifndef IOLUA_HPP
#define IOLUA_HPP

#include <lua/lua.hpp>
#include <lemon/config.h>
#include <lemon/mutex.hpp>
#include <lemon/nocopy.hpp>

namespace iolua {

    /**
     * define iolua's main entrypoint class
     */
    class iolua_State : private lemon::nocopy
    {
    public:

    };
}

#endif //IOLUA_HPP