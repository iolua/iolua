#ifndef LEMON_LOG_FACTORY_HPP
#define LEMON_LOG_FACTORY_HPP

#include <mutex>
#include <atomic>
#include <vector>
#include <string>
#include <memory>
#include <thread>

#include <unordered_map>
#include <unordered_set>
#include <condition_variable>

namespace lemon{ namespace log{

	class sink;
	class logger;
	struct message;

	/**
	 * the logger's factory
	 */
	class factory
	{
	public:
		factory();

		void write(const message &msg);

		/**
		 * set the logger's log level
		 * @levels  the log levels
		 * @loggers the target loggers
		 */
		void setlevels(int levels,const std::vector<std::string> &loggers);

		/**
		 * add new logger's sink
		 */
		void add_sink(std::unique_ptr<sink> s);
		void add_sink(const std::string & name, std::unique_ptr<sink> s);

		/**
		 * remove sink
		 */
		void remove_sink(const std::string & name);

		/*
		 * remove all sink
		 */
		void remove_all_sinks();

		/**
		 * get the sink object by the register name
		 */
		std::shared_ptr<sink> get_sink(const std::string & name) const;
		/**
		 * get or create new logger
		 */
		const logger& get(const std::string &name);

		/**
		 * close logger factory
		 */
		void close();

	private:

		void write_loop();

	private:
		int															_levels;
		mutable std::mutex          								_mutex;
		std::unordered_map<std::string, logger*>					_loggers;
		std::unordered_map<std::string, std::shared_ptr<sink>>		_sinks;
		std::atomic<bool>											_exitflag;
		std::condition_variable_any									_notify;
		std::thread													_writer;
		std::vector<message>										_messages;
	};

}}

#endif //LEMON_LOG_FACTORY_HPP