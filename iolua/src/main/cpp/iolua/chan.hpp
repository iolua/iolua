#ifndef IOLUA_CHAN_HPP
#define IOLUA_CHAN_HPP

#include <atomic>
#include <cstdint>
#include <lemon/nocopy.hpp>

namespace iolua {

    class iolua_State;

    class channel : private lemon::nocopy
    {
    public:
        channel(iolua_State * context, std::uint32_t id);

        ~channel();
    };
}

#endif //IOLUA_CHAN_HPP