#ifndef LEMON_LOG_LOG_HPP
#define LEMON_LOG_LOG_HPP


#include <stdio.h>
#include <cstdarg>
#include <stdlib.h>

#include <lemon/log/sink.hpp>
#include <lemon/log/logger.hpp>
#include <lemon/log/file_sink.hpp>

namespace lemon{ namespace log{
	
	void add_sink(std::unique_ptr<sink> s);
	void add_sink(const std::string & name,std::unique_ptr<sink> s);
	void remove_all_sinks();
	std::shared_ptr<sink> get_sink(const std::string & name);

	const logger& get(const std::string & name);

	void close();

	inline void write(const logger & source, level l,const char *file, int lines, const char *fmt,...)
	{
		va_list args;

		va_start(args,fmt);

#ifdef WIN32
		
		int len = vsnprintf(NULL,0, fmt, args) + 1;

		char *buff = new char[len];

		vsnprintf_s(buff, len, len, fmt, args);
#else
		char *buff;
		int len = vasprintf(&buff, fmt, args);
#endif //WIN32
		va_end(args);

		source.write(l, std::string(buff, buff + len), file, lines);

#ifdef WIN32
		delete buff;
#else
		free(buff);
#endif //WIN32
	}
}}

#define lemonE(l,fmt,...) if(((l).levels() & (int)lemon::log::level::error) != 0) { \
	lemon::log::write((l),lemon::log::level::error,__FILE__,__LINE__,(fmt),##__VA_ARGS__);\
}

#define lemonW(l,fmt,...) if(((l).levels() & (int)lemon::log::level::warn) != 0) { \
	lemon::log::write((l),lemon::log::level::warn,__FILE__,__LINE__,(fmt),##__VA_ARGS__);\
}

#define lemonI(l,fmt,...) if(((l).levels() & (int)lemon::log::level::info) != 0) { \
	lemon::log::write((l),lemon::log::level::info,__FILE__,__LINE__,(fmt),##__VA_ARGS__);\
}

#define lemonD(l,fmt,...) if(((l).levels() & (int)lemon::log::level::debug) != 0) { \
	lemon::log::write((l),lemon::log::level::debug,__FILE__,__LINE__,(fmt),##__VA_ARGS__);\
}


#define lemonT(l,fmt,...) if(((l).levels() & (int)lemon::log::level::trace) != 0) { \
	lemon::log::write((l),lemon::log::level::trace,__FILE__,__LINE__,(fmt),##__VA_ARGS__);\
}

#define lemonV(l,fmt,...) if(((l).levels() & (int)lemon::log::level::verbose) != 0) { \
	lemon::log::write((l),lemon::log::level::verbose,__FILE__,__LINE__,(fmt),##__VA_ARGS__);\
}


#endif //LEMON_LOG_LOG_HPP