#ifndef IOLUA_HPP
#define IOLUA_HPP

#include <cstdint>
#include <vector>
#include <thread>
#include <queue>
#include <unordered_map>
#include <condition_variable>

#include <lua/lua.hpp>
#include <lemon/config.h>
#include <lemon/mutex.hpp>
#include <lemon/nocopy.hpp>

namespace iolua {

    class task;
    /**
     * define iolua's main entrypoint class
     */
    class iolua_State final: private lemon::nocopy
    {
    public:
        iolua_State();
        ~iolua_State();

        void start(const std::string & name, const std::vector<std::string> & args);

        void join();

        void open_libs(task * t);

        std::uint32_t create_task(lua_State *L);

        void wakeup(std::uint32_t taskid);

    private:
        void do_schedule();
    private:
        lua_State                                       *_L;
        std::vector<std::thread>                        _workers;
        lemon::spin_mutex                               _mutex;
        std::condition_variable_any                     _condition;
        bool                                            _exit = false;
        std::uint32_t                                   _idgen = 0;
        std::unordered_map<std::uint32_t, task*>        _tasks;
        std::unordered_map<std::uint32_t, task*>        _sleepingQ;
        std::queue<task*>                               _runningQ;
    };
}

#endif //IOLUA_HPP