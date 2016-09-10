#ifndef IOLUA_LTASK_TASK_HPP
#define IOLUA_LTASK_TASK_HPP

#include <lua/lua.hpp>
#include <stdint.h>

namespace iolua {
    namespace ltask {

        using task_id = uint32_t;

        enum class task_state {
            init,sleeping,closed
        };

        class scheduler;
        class task
        {
        public:

            task(const task &) = delete;

            task(scheduler *sc, lua_State *L, task_id id);
            ~task();

            void run() noexcept ;

            task_state state() const {
                return _state;
            }

            task_id id() const {
                return _id;
            }

            lua_State * L() const {
                return _L;
            }

            scheduler* owner() {
                return _scheduler;
            }

            void stay_awake(bool flag) {
                _resume = flag;
            }

            bool sleepable() const {
                return !_resume;
            }


        private:
            task_id          _id;
            lua_State        *_L;
            task_state       _state;
            scheduler        *_scheduler;
            bool             _resume;
        };
    }
}

#endif //IOLUA_LTASK_TASKH_HPP