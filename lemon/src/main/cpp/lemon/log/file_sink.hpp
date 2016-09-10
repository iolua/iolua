/**
 * 
 * @file     file_sink
 * @brief    Copyright (C) 2016  yayanyang All Rights Reserved 
 * @author   yayanyang
 * @date     2016/01/12
 */
#ifndef LEMON_LOG_FILE_SINK_HPP
#define LEMON_LOG_FILE_SINK_HPP

#include <string>
#include <fstream>
#include <lemon/log/sink.hpp>
#include <lemon/fs/fs.hpp>

namespace lemon{namespace log{

	class file_sink : public sink, private nocopy
	{
	public:

		file_sink(const std::vector<std::string> & sources,const fs::filepath dir, const std::string &name)
			:sink(sources)
			,_dir(dir)
			, _name(name)
			, _ext(".log")
			, _time_suffix(true)
			, _maxsize((std::uintmax_t)-1)
			, _blocks(0)
		{
			if (!fs::exists(dir))
			{
				fs::create_directories(dir);
			}
		}

		file_sink(const fs::filepath dir,const std::string &name)
			:_dir(dir)
			,_name(name)
			,_ext(".log")
			,_time_suffix(true)
			,_maxsize((std::uintmax_t) -1)
			,_blocks(0)
		{
			if (!fs::exists(dir))
			{
				fs::create_directories(dir);
			}
		}

		virtual ~file_sink() 
		{
			_stream.close();
		}

		file_sink & ext(const std::string & name) 
		{
			_ext = name;
			return *this;
		}

		file_sink & time_suffix(bool flag)
		{
			_time_suffix = flag;
			return *this;
		}

		file_sink & max_size(std::uintmax_t maxsize)
		{
			_maxsize = maxsize;
			return *this;
		}

		void write(const message & msg) final;
	
	private:

		void openfile();

		std::string calc_file_name();

	private:

		const fs::filepath			_dir;
		const std::string			_name;
		std::string 				_ext;
		bool						_time_suffix;
		std::uintmax_t				_maxsize;
		size_t						_blocks;
		std::ofstream				_stream;
		std::string					_filename;
	};

}}


#endif //LEMON_LOG_FILE_SINK_HPP