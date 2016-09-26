#include <iolua/chan.hpp>
#include <iolua/iolua.hpp>
#include <lemon/log/log.hpp>

namespace iolua {

	static auto& logger = lemon::log::get("iolua");

	channel::channel(iolua_State * context)
		:_context(context)
	{

	}

	channel::~channel()
	{

	}

	void channel::write_message(void * message)
	{
		std::uint32_t id;

		for (;;)
		{
			{
				std::unique_lock<lemon::spin_mutex> lock(_mutex);

				_messageQ.push(message);

				if (_selectQ.empty())  return;

				id = _selectQ.front();

				_selectQ.pop();
			}

			lemonD(logger,"channel(%d) wake-up task(%d)",this->id(),id);

			if(_context->wakeup(id))
			{
				lemonD(logger,"channel(%d) wake-up task(%d) -- success",this->id(),id);
				return;
			}

			lemonD(logger,"channel(%d) wake-up task(%d) -- failed",this->id(),id);
		}


	}

	void* channel::read_message()
	{
		std::unique_lock<lemon::spin_mutex> lock(_mutex);

		if (_messageQ.empty()) return nullptr;

		auto data = _messageQ.front();

		_messageQ.pop();

		return data;
	}

	bool channel::do_select(std::uint32_t task_id)
	{
		std::unique_lock<lemon::spin_mutex> lock(_mutex);

		if (!_messageQ.empty())
		{
			lemonD(logger,"task(%d) select channel(%d) -- noblock",task_id,id());
			return true;
		}

		lemonD(logger,"task(%d) select channel(%d) -- blocking",task_id,id());

		_selectQ.push(task_id);

		return false;
	}
}