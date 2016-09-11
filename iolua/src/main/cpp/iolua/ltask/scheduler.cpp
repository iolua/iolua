#include <shared_mutex>
#include <iolua/ltask/scheduler.hpp>
#include <iolua/ltask/ltask_error.hpp>
#include <lemon/log/log.hpp>

namespace iolua {
    namespace ltask {

        static auto& logger = lemon::log::get("ltask");

        scheduler::scheduler(int threads)
            :_exit(false), _io_object_map(_io_service)
        {
            for( int i = 0; i < threads; i ++ )
            {
                _threads.push_back(std::thread(&scheduler::do_schedule,this));
            }

            _L = luaL_newstate();

            if (_L == nullptr) {
                throw std::system_error(make_error_code(errc::out_of_memory));
            }

			_io_thread = std::thread(&scheduler::do_io_schedule, this);
        }

        scheduler::~scheduler()
        {
            exit(0);
            join();
        }

        void scheduler::exit(int code)
        {
            std::lock_guard<lemon::shared_mutex> lock(_mutex);

            _exit = true;

			_exitCode = code;

            _cv.notify_all();

			_io_service.close();
        }

        void scheduler::join()
        {
            for( auto & t : _threads )
            {
                if(t.joinable())
                {
                    t.join();
                }
            }

			if(_io_thread.joinable()) 
			{
				_io_thread.join();
			}

        }

        uint32_t scheduler::create_channel(lua_State * /*L*/)
        {
            uint32_t id;

            {
                std::shared_lock<lemon::shared_mutex> lock(_mutex);

                for(;;)
                {
                    id = _idgen ++ ;

                    if(id != 0 &&  _channels.count(id) == 0 ) break;
                }
            }

            auto c = new channel(this,id);

            std::lock_guard<lemon::shared_mutex> lock(_mutex);

            _channels[id] = c;

            return id;
        }

        void scheduler::close_channel(uint32_t id)
        {
            std::lock_guard<lemon::shared_mutex> lock(_mutex);

            auto c = _channels[id];

            if(c) {
                c->close();
                _channels.erase(id);
            }
        }

        channel * scheduler::get_channel(uint32_t id)
        {
            std::shared_lock<lemon::shared_mutex> lock(_mutex);

            auto c = _channels[id];

            if(c && c->addref()) {
                return c;
            }

            return nullptr;

        }

        task_id scheduler::create_task(lua_State *L)
        {
            task_id id;

            {
                std::shared_lock<lemon::shared_mutex> lock(_mutex);

                for(;;)
                {
                    id = _idgen ++ ;

                    if(id != 0 && _tasks.count(id) == 0 ) break;
                }
            }

            auto t = new task(this,L,id);

            std::lock_guard<lemon::shared_mutex> lock(_mutex);

            _tasks[id] = t;

            _runningTasks.push(t);

            _cv.notify_one();

            return id;
        }

        void scheduler::do_schedule()
        {
            std::unique_lock<lemon::shared_mutex> lock(_mutex);

            while(!_exit)
            {
                if (_runningTasks.empty())
                {
                    _cv.wait(lock);

                    continue;
                }

                auto t = _runningTasks.front();

                _runningTasks.pop();

                lemonD(logger,"schedule task(%d,%d)", t->id(),t->state());

                lock.unlock();

                t->run();

                lock.lock();

                lemonD(logger,"schedule task(%d,%d) -- yield", t->id(),t->state());

                if (t->state() == task_state::closed) {
                    _tasks.erase(t->id());

                    delete t;

                    continue;
                }

                if(!t->sleepable()) {
                    _runningTasks.push(t);
                    continue;
                }
                
                _sleepingTasks[t->id()] = t;
            }
        }

        inline int ltask_create(lua_State *L){

            scheduler * sc = (scheduler*)lua_touserdata(L,lua_upvalueindex(1));

            sc->create_task(L);

            return 0;
        }

        void scheduler::start(const std::string & name, const std::vector<std::string> & args)
        {
            lua_pushlightuserdata(_L,this);

            lua_pushcclosure(_L,ltask_create,1);

            lua_pushstring(_L,name.c_str());

            for(auto & arg : args) {
                lua_pushstring(_L,arg.c_str());
            }

            auto err = lua_pcall(_L,(int)args.size() + 1,0, 0);

            if (err != LUA_OK) {
                std::string err_msg = lua_tostring(_L,-1);
                lua_close(_L);
                throw std::system_error(make_error_code(errc::create_task_error),err_msg);
            }
        }

        uint32_t scheduler::channel_select(task_id source, uint32_t * channels, int count)
        {
            for(int i = 0 ; i < count; i ++ ) {

                auto c = get_channel(channels[i]);

                if(c && c->do_select(source)) {
                    return channels[i];
                }
            }

            return 0;
        }

        void scheduler::resume(task_id id)
        {
            std::unique_lock<lemon::shared_mutex> lock(_mutex);

            auto task = _sleepingTasks[id];

            if (task != nullptr) {
                _sleepingTasks.erase(id);

                _runningTasks.push(task);

                _cv.notify_one();

                return;

            }

            task = _tasks[id];

            if (task != nullptr) {

                task->stay_awake(true);

                return;
            }

            lemonW(logger,"resume task(%d) -- failed not found",id);
        }


		void scheduler::do_io_schedule()
		{
			while (true)
			{
				std::error_code ec;
				_io_service.run_one(ec);

				if(ec == lemon::io::errc::io_service_closed){
					break;
				}
			}
		}
    }
}