#ifndef IOLUA_SHARED_OBJECT_HPP
#define IOLUA_SHARED_OBJECT_HPP

#include <atomic>
#include <lemon/nocopy.hpp>

namespace iolua {

    class shared_object : private lemon::nocopy
    {
    private:
        virtual void close() = 0;
    };
}

#endif //IOLUA_SHARED_OBJECT_HPP