#include <iolua/chan.hpp>
#include <iolua/iolua.hpp>

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

		{
			std::unique_lock<lemon::spin_mutex> lock(_mutex);

			_messageQ.push(message);

			if (_selectQ.empty())  return;

			id = _selectQ.front();

			_selectQ.pop();
		}

		if(_context->wake_up(id))
		{
			return;
		}

		lemonD(logger,"wake update task(%d) failed",id);
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
			return true;
		}

		_selectQ.push(task_id);

		return false;
	}
}