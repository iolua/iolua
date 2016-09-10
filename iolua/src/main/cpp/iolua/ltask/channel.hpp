#ifndef IOLUA_LTASK_CHANNEL_HPP
#define IOLUA_LTASK_CHANNEL_HPP

#include <queue>
#include <lemon/mutex.hpp>
#include <iolua/ltask/task.hpp>
#include <unordered_set>


namespace iolua {
    namespace ltask {
        class scheduler;

        class channel
        {
        public:

            channel(const channel& ) = delete;

            channel(scheduler* sc,uint32_t id);

            ~channel();

            void close();

            bool addref();

            void push(void * data);

            void* pop();

            bool do_select(task_id source);

        private:
            scheduler                   *_scheduler;
            uint32_t                    _id;
            std::atomic<int>            _counter;
            lemon::spin_mutex           _mutex;
            std::queue<void*>           _data;
            std::queue<task_id>         _blockings;

        };
    }
}

#endif //IOLUA_LTASK_CHANNEL_HPP