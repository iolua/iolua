#include <lemon/log/log.hpp>
#include <iolua/iolua.hpp>
#include <iolua/task.hpp>
#include <iolua/iolua_error.hpp>
#include <iolua/libtask.hpp>
#include <iolua/liblog.hpp>
#include <iolua/libchan.hpp>
#include <iolua/libsocket.hpp>
#include <iolua/libpipe.hpp>
#include <iolua/libexec.hpp>
#include <iolua/libfs.hpp>
#include <sqlite/lsqlite3.h>
namespace iolua {

    static auto& logger = lemon::log::get("iolua_dispatcher");

    iolua_State::iolua_State()
    {
        _timer_wheel.start();

        _L = luaL_newstate();

        if (_L == nullptr)
        {
            throw std::system_error(make_error_code(errc::out_of_memory));
        }

        int threads = std::thread::hardware_concurrency();

        for( int i = 0; i < threads; i ++ )
        {
            _workers.push_back(std::thread(&iolua_State::do_schedule,this));
        }


    }

    iolua_State::~iolua_State()
    {
        join();

		lua_close(_L);
    }

    void iolua_State::do_schedule()
    {
        std::unique_lock<lemon::spin_mutex> locker(_mutex);

        while(!_exit)
        {
            if (_runningQ.empty())
            {
                _condition.wait(locker);

                continue;
            }

            auto task = _runningQ.front();

            _runningQ.pop();

            task->set_state(task_state::running);

            locker.unlock();

            auto exit = task->run();

            locker.lock();

            if(exit)
            {
                task->set_state(task_state::closed);
            }

            switch (task->get_state())
            {
                case task_state::running:
                    task->set_state(task_state::sleeping);

                    _sleepingQ[task->id()] = task;

                    break;
                case task_state::prevent_sleeping:

                    task->set_state(task_state::running);

                    _runningQ.push(task);

                    break;

                case task_state::closed :
                    _tasks.erase(task->id());
					locker.unlock();
                    delete task;
					locker.lock();

                    break;

                default:

                    lemonE(logger,"inner error :illegal task(%d) state(%d). ", task->id(),task->get_state());

                    _tasks.erase(task->id());
					locker.unlock();
                    delete task;
					locker.lock();
                    break;
            }

            if(_tasks.empty())
            {
                _exit = true;

                _condition.notify_all();
            }
        }
    }

    void iolua_State::exit()
    {
        std::unique_lock<lemon::spin_mutex> locker(_mutex);

        _exit = true;

        _condition.notify_all();
    }

    inline int lua_task_create(lua_State *L)
    {

        iolua_State * sc = (iolua_State*)lua_touserdata(L,lua_upvalueindex(1));

        sc->create_task(L);

        return 0;
    }

    void iolua_State::start(const std::string & name, const std::vector<std::string> & args)
    {
        lua_pushlightuserdata(_L,this);

        lua_pushcclosure(_L, lua_task_create, 1);

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

    void iolua_State::join()
    {
        for( auto & t : _workers )
        {
            if(t.joinable())
            {
                t.join();
            }
        }
    }

    std::uint32_t iolua_State::create_task(lua_State *L)
    {
        std::uint32_t id;

        std::unique_lock<lemon::spin_mutex> lock(_mutex);

        for(;;)
        {
            id = _idgen ++ ;

            if(id != 0 && _tasks.count(id) == 0 ) break;
        }

        lemonD(logger,"create task(%d)",id)

        auto t = new task(this,L,id);

        _tasks[id] = t;

        _runningQ.push(t);

        _condition.notify_one();

        return id;
    }

    void iolua_State::open_libs(task * t)
    {
        luaL_openlibs(t->L());

        iolua_opentask(t);
        iolua_openlog(t);
        iolua_open_chan(t);
        iolua_open_socket(t);
        iolua_openpipe(t);
        iolua_open_exec(t);
        iolua_open_fs(t);
        luaopen_lsqlite3(t->L());

        lua_setglobal(t->L(),"sqlite3");
    }

    bool iolua_State::wake_up(std::uint32_t task_id)
    {
        std::unique_lock<lemon::spin_mutex> lock(_mutex);

        auto iter = _tasks.find(task_id);

        if (iter != _tasks.end())
        {

			if (iter->second->get_state() == task_state::running)
			{
				iter->second->set_state(task_state::prevent_sleeping);

				return true;
			}

			if (_sleepingQ.count(task_id))
			{
				_sleepingQ.erase(task_id);

				_runningQ.push(iter->second);
			}

			_condition.notify_one();

            return true;
        }

        return false;

    }
}