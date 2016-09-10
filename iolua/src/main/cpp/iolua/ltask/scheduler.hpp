#ifndef IOLUA_SCHEDULER_HPP
#define IOLUA_SCHEDULER_HPP

#include <atomic>
#include <mutex>
#include <queue>
#include <string>
#include <vector>
#include <thread>
#include <lua/lua.hpp>
#include <unordered_map>
#include <condition_variable>
#include <lemon/mutex.hpp>
#include <iolua/ltask/task.hpp>
#include <iolua/ltask/channel.hpp>

namespace iolua {
    namespace ltask {

        class scheduler;

        class scheduler
        {
        public:

            scheduler(int threads);

            ~scheduler();

            void join();

            void exit(int code);

            task_id create_task(lua_State *L);

            uint32_t create_channel(lua_State *L);

            void close_channel(uint32_t id);

            channel * get_channel(uint32_t id);

            void start(const std::string & name, const std::vector<std::string> & args);

            uint32_t channel_select(task_id source, uint32_t * channels, int count);

            void resume(task_id task);

        private:

            void do_schedule();

        private:
            lua_State                               *_L;
			int										_exitCode;
            bool                                    _exit;
            std::atomic<uint32_t>                   _idgen = 0;
            std::unordered_map<task_id,task*>       _tasks;
            lemon::shared_mutex                     _mutex;
            std::condition_variable_any             _cv;
            std::vector<std::thread>                _threads;
            std::queue<task*>                       _runningTasks;
            std::unordered_map<task_id,task*>       _sleepingTasks;
            std::unordered_map<uint32_t ,channel*>  _channels;
        };
    }
}

#endif // IOLUA_SCHEDULER_HPP