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
#include <iolua/shared_object.hpp>
#include <iolua/chan.hpp>
#include <iolua/io.hpp>
#include <lemon/timewheel.hpp>
namespace iolua {

    class task;
	
   
    class iolua_State final: private lemon::nocopy
    {
    public:
        iolua_State();
        ~iolua_State();

        void start(const std::string & name, const std::vector<std::string> & args);

        void join();

        void open_libs(task * t);

        std::uint32_t create_task(lua_State *L);

        bool wakeup(std::uint32_t taskid);

		std::uint32_t create_channel()
		{
			return _channels.create(this);
		}

		void close_channel(std::uint32_t id)
		{
			_channels.close(id);
		}

		channel* query_channel(std::uint32_t id)
		{
			return _channels.addref_and_fetch(id);
		}

		std::uint32_t create_io_object(lemon::io::io_object* object)
		{
			return _io_objects.create(object, this);
		}

		void close_io_object(std::uint32_t id)
		{
			_io_objects.close(id);
		}

		io_object* query_io_object(std::uint32_t id)
		{
			return _io_objects.addref_and_fetch(id);
		}

		lemon::io::io_service & io_service()
		{
			return _io_objects.io_service();
		}

		template<typename _Type>
		std::uint32_t create_io_promise()
		{
			return _io_promises.attach(new _Type(this));
		}

		template<typename _Type>
		_Type* create_io_promise(std::uint32_t & id)
		{
			auto val = new _Type(this);
			id = _io_promises.attach(val,true);

			return val;
		}

		void close_io_promise(std::uint32_t id)
		{
			_io_promises.close(id);
		}

		io_promise* query_io_promise(std::uint32_t id)
		{
			return _io_promises.addref_and_fetch(id);
		}

		lemon::timer_wheel & timer_wheel()
		{
			return _timer_wheel;
		}

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
		shared_object_cached<channel>					_channels;
		io_object_cached								_io_objects;
		shared_object_cached<io_promise>				_io_promises;
		lemon::timer_wheel								_timer_wheel = { 10 };
    };
}

#endif //IOLUA_HPP