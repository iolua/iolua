#include <lemon/log/log.hpp>
#include <lemon/log/sink.hpp>
#include <lemon/log/factory.hpp>

namespace lemon{ namespace log{

    namespace {
        std::once_flag flag;

        factory *_factory;
    }

	void init()
	{
		_factory = new factory();
	}

	void add_sink(std::unique_ptr<sink> s)
	{
		std::call_once(flag, init);

		_factory->add_sink(std::move(s));
	}

	void add_sink(const std::string & name, std::unique_ptr<sink> s)
	{
		std::call_once(flag, init);

		_factory->add_sink(name,std::move(s));
	}
	void remove_all_sinks()
	{
		std::call_once(flag, init);

		_factory->remove_all_sinks();
	}
	std::shared_ptr<sink> get_sink(const std::string & name)
	{
		std::call_once(flag, init);

		return _factory->get_sink(name);
	}

    const logger& get(const std::string &name)
    {
        std::call_once(flag,init);

        return _factory->get(name);
    }

    void close()
    {
        std::call_once(flag,init);

		_factory->close();
    }
}}