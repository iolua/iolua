#include <lemon/log/logger.hpp>
#include <lemon/log/factory.hpp>

namespace lemon{namespace log{

	logger::logger(const std::string & name, factory & f)
		:_name(name),_factory(f)
	{

	}

	void logger::write(level l, const std::string && content, const std::string && file, int lines) const
	{
		if(((int)l & _levels) != 0)
		{
			message msg = { l,std::chrono::system_clock::now(),_name,file,lines,content };

			_factory.write(msg);
		}
	}
}}