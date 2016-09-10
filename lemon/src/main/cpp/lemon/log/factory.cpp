#include <cstddef>
#include <sstream>
#include <lemon/log/sink.hpp>
#include <lemon/log/factory.hpp>
#include <lemon/log/logger.hpp>

namespace lemon{ namespace log{
	factory::factory()
		:_levels((int)level::all),_exitflag(false)
	{
		_writer = std::thread(&factory::write_loop, this);
	}

	void factory::write_loop()
	{

		std::unique_lock<std::mutex> lock(_mutex);

		for(;;)
		{
			if(_messages.empty())
			{
				if (_exitflag) break;
				_notify.wait(lock);
			}

			auto messages = std::move(_messages);

			lock.unlock();

			for(auto& msg : messages)
			{
				for(auto s :_sinks)
				{
					if(s.second->apply(msg.Source))
					{
						s.second->write(msg);
					}
				}
			}

			lock.lock();
		}
	}

	void factory::write(const message &msg)
	{
		std::lock_guard<std::mutex> lock(_mutex);

		_messages.push_back(msg);

		_notify.notify_one();
	}

	void factory::setlevels(int levels, const std::vector<std::string> &loggers)
	{
		std::lock_guard<std::mutex> lock(_mutex);

		if(loggers.empty())
		{
			_levels = levels;

			return;
		}

		for(auto source: loggers)
		{
			if (_loggers.count(source) == 0)
			{
				auto l = new logger(source, *this);

				l->levels(_levels);

				_loggers[source] = l;
			}
			else
			{
				auto l = _loggers[source];

				l->levels(levels);
			}
		}
	}


	void factory::add_sink(std::unique_ptr<sink> s)
	{
		std::stringstream stream;

		stream << (ptrdiff_t)s.get();

		add_sink(stream.str(), std::move(s));
	}

	void factory::add_sink(const std::string & name, std::unique_ptr<sink> s)
	{
		std::lock_guard<std::mutex> lock(_mutex);

		_sinks[name] = std::shared_ptr<sink>(s.release());
	}

	void factory::remove_sink(const std::string & name)
	{
		std::lock_guard<std::mutex> lock(_mutex);

		_sinks.erase(name);
	}

	void factory::remove_all_sinks()
	{
		std::lock_guard<std::mutex> lock(_mutex);

		_sinks.clear();
	}

	std::shared_ptr<sink> factory::get_sink(const std::string & name) const
	{
		std::lock_guard<std::mutex> lock(_mutex);

		auto iter = _sinks.find(name);

		if (iter == _sinks.end())
		{
			return{};
		}

		return iter->second;
	}

	/**
	* get or create new logger
	*/
	const logger& factory::get(const std::string &name)
	{
		std::unique_lock<std::mutex> lock(_mutex);

		if(_loggers.count(name) == 0)
		{
			auto source = new logger(name, *this);

			source->levels(_levels);

			_loggers[name] = source;
		}

		return *_loggers[name];
	}

	/**
	* close logger factory
	*/
	void factory::close()
	{
		{
			std::unique_lock<std::mutex> lock(_mutex);

			if(!_exitflag)
			{
				_exitflag = true;

				_notify.notify_one();
			}
		}

		if(_writer.joinable()) {
			_writer.join();
		}
	}
}}