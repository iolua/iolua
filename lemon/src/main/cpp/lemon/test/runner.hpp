//
// Created by liyang on 15/11/13.
//

#ifndef LAKE_RUNNER_HPP
#define LAKE_RUNNER_HPP

#include <vector>
#include <lemon/nocopy.hpp>


namespace lemon{namespace test{

    class runnable;

    class runner : private nocopy
    {
    public:
        /**
         * start the test runner singleton
         */
        static void run();

        /**
         * get singleton runner instance
         */
        static runner & instance();

    public:

        /**
         * add new tes/bench unit
         */
        void add(runnable *unit);

    private:

        void done();

    private:
        std::vector<runnable*> _units; /* the test/bench units */
    };
}}

#endif //LAKE_RUNNER_HPP
