#ifndef IOLUA_CHAN_HPP
#define IOLUA_CHAN_HPP

#include <queue>
#include <atomic>
#include <cstdint>
#include <lemon/mutex.hpp>
#include <lemon/nocopy.hpp>
#include <iolua/shared_object.hpp>
namespace iolua {

    class iolua_State;

    class channel final: public shared_object
    {
    public:
        channel(iolua_State * context);

        ~channel();

		void write_message(void * message);

		void* read_message();

		bool do_select(std::uint32_t task_id);

		void close();

	private:
		iolua_State					*_context;
		lemon::spin_mutex			_mutex;
		std::queue<void*>			_messageQ;
		std::queue<std::uint32_t>   _selectQ;
		bool						_closed = false;
    };
}

#endif //IOLUA_CHAN_HPP