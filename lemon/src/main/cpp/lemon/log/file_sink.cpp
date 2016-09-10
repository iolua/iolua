#include <sstream>
#include <iomanip> 
#include <lemon/fs/fs.hpp>
#include <lemon/log/logger.hpp>
#include <lemon/log/file_sink.hpp>

namespace lemon{ namespace log{

	std::string file_sink::calc_file_name()
	{
		auto path = _dir;

		std::stringstream stream;

		stream << _name;

		if (_time_suffix)
		{
			auto now = std::chrono::system_clock::now();

			std::time_t ts = std::chrono::system_clock::to_time_t(now);
#ifdef WIN32
			tm t;
			localtime_s(&t, &ts);
			tm *tm = &t;
#else
			auto tm = localtime(&ts);
#endif 

			stream << "-" << tm->tm_year + 1900 << "-" << tm->tm_mon << "-" << tm->tm_mday;
		}
		
		stream << _ext;

		path /= stream.str();

		return path.string();
	}

	void file_sink::openfile()
	{
		auto filename = calc_file_name();

		if(_filename != filename)
		{
			_filename = filename;

			_blocks = 0;

			if(_stream.is_open())
			{
				_stream.close();
			}

			_stream.open(_filename, std::ofstream::out | std::ofstream::app);
		}

		if(_blocks > 0)
		{
			std::stringstream stream;

			stream << _filename << "." << _blocks;

			filename = stream.str();
		}

		if(fs::exists(filename) && fs::file_size(filename) > _maxsize)
		{
			_stream.close();

			_blocks++;

			std::stringstream stream;

			stream << _filename << "." << _blocks;

			filename = stream.str();

			_stream.open(filename, std::ofstream::out | std::ofstream::app);
		}
	}

	void file_sink::write(const message & msg)
	{
		openfile();

		std::time_t ts = std::chrono::system_clock::to_time_t(msg.TS);
#ifdef WIN32
		tm t;
		localtime_s(&t, &ts);
		tm *tm = &t;
#else
		auto tm = localtime(&ts);
#endif 

		auto milliseconds =

			std::chrono::duration_cast<std::chrono::milliseconds>(msg.TS.time_since_epoch()).count() - 

			std::chrono::duration_cast<std::chrono::seconds>(msg.TS.time_since_epoch()).count() * 1000
			;


		_stream << tm->tm_year + 1900 << "-" << tm->tm_mon << "-" << tm->tm_mday << " "

			<< tm->tm_hour << ":" << tm->tm_min << ":" << tm->tm_sec << "."

			<< std::setw(4) << std::setfill('0') <<milliseconds << " "

			<< msg.Source << " (" << fs::filepath(msg.File).filename() << ":" << msg.Lines << ") " << msg.Content << std::endl;

		_stream.flush();
	}

}}