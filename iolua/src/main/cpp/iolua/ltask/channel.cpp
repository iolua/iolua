#include <mutex>
#include <cassert>
#include <iolua/ltask/channel.hpp>
#include <iolua/ltask/scheduler.hpp>
#include <lemon/log/log.hpp>
namespace iolua {
    namespace ltask {
        static auto& console = lemon::log::get("ltask");

        channel::channel(scheduler *sc, uint32_t id)
                :_scheduler(sc),_id(id),_counter(1)
        {

        }

        channel::~channel()
        {
            lemonD(console,"close channel(%d)",_id);
        }

        bool channel::addref()
        {
            if(++ _counter  == 1) {
                close();

                return false;
            }

            return true;
        }

        void channel::close() {
            if( -- _counter == 0) {
                delete this;
            }
        }

        void channel::push(void * data)
        {
            task_id id;

            {
                std::unique_lock<lemon::spin_mutex> lock(_mutex);

                _data.push(data);

                if(_blockings.empty())  return;

                id = _blockings.front();

                _blockings.pop();
            }


            _scheduler->resume(id);

        }

        void* channel::pop()
        {
            std::unique_lock<lemon::spin_mutex> lock(_mutex);

            if(_data.empty()) return nullptr;

            auto data = _data.front();

            _data.pop();

            return data;
        }

        bool channel::do_select(task_id source)
        {
            std::unique_lock<lemon::spin_mutex> lock(_mutex);

            if (_data.empty()) {
                _blockings.push(source);
                return false;
            }

            return true;
        }
    }
}