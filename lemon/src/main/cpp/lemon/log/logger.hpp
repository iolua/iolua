#ifndef LEMON_LOG_LOGGER_HPP
#define LEMON_LOG_LOGGER_HPP

#include <string>
#include <chrono>
#include <unordered_set>

#include <lemon/nocopy.hpp>

namespace lemon{ namespace log{

	// log level enum
	enum class level {
		error = 1, warn = 2, info = 4, debug = 8, trace = 16, verbose = 32,all = 63
	};

	class sink;
	class factory;

	struct message
	{
		level                                     LEVEL;
		std::chrono::system_clock::time_point     TS;
		std::string                               Source;
		std::string                               File;
		int                                       Lines;
		std::string                               Content;
	};

	class logger : private nocopy
	{
	public:

		logger(const std::string & name, factory & f);

		int levels() const { return _levels; }

		void levels(int levels) { _levels = levels; }

		const std::string name() const { return _name; }

		void write(level l, const std::string && content, const std::string && file, int lines) const;

	private:
		const std::string								_name;
		factory											&_factory;
		int												_levels;
	};
}}
#endif //LEMON_LOG_LOGGER_HPP