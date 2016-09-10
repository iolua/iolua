//
// Created by liyang on 15/11/13.
//

#ifndef LAKE_NOCOPY_HPP
#define LAKE_NOCOPY_HPP

namespace lemon {

    class nocopy
    {
    public:
        nocopy(){}
        nocopy(const nocopy&)  = delete;
        nocopy(const nocopy&&) = delete;
        nocopy &operator = (const nocopy&) = delete;
        nocopy &operator = (const nocopy&&) = delete;
    };
}

#endif //LAKE_NOCOPY_HPP
