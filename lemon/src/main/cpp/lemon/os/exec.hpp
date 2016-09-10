#ifndef LEMON_OS_EXEC_HPP
#define LEMON_OS_EXEC_HPP
#include <mutex>
#include <thread>
#include <vector>
#include <memory>
#include <chrono>
#include <utility>
#include <functional>
#include <unordered_map>
#include <condition_variable>

#include <lemon/fs/fs.hpp>
#include <lemon/io/io.hpp>



#include <lemon/os/exec_options.hpp>
#include <lemon/os/os_errors.hpp>
#include <lemon/os/sysinfo.hpp>

using namespace std;

namespace lemon {
	namespace os {


		class process;

		/**
		* start new command
		*/
		void process_start(process &impl, std::error_code & err, const std::vector<std::string> & args) noexcept;

		/**
		* wait process exit
		*/
		int process_wait(process &impl, std::error_code & err) noexcept;

		/**
		* set the process work path
		*/
		void process_work_path(process &proc, const fs::filepath & path, std::error_code & err) noexcept;

		/**
		* get the process work path
		*/
		fs::filepath process_work_path(process &proc) noexcept;

		/**
		* set the new process env
		*/
		template <typename Env>
		void process_env(process & proc, Env &&env);

		/**
		* get the process env
		*/
		const std::unordered_map<std::string, std::string>& process_env(process & proc);

		class exec
		{
		public:
			exec(const std::string & name)
				:exec(name, exec_options::none)
			{

			}

			exec(const std::string & name, exec_options options)
				:_joined(true), _options(options)
			{
				auto found = lookup(name);

				if (!std::get<1>(found))
				{
					throw std::system_error((int)errc::command_not_found, os_error_category());
				}

				if (((int)options & (int)exec_options::pipe_in))
				{
					_in.reset(new io::pipe(_ioservice));
				}

				if (((int)options & (int)exec_options::pipe_out))
				{
					_out.reset(new io::pipe(_ioservice));
				}

				if (((int)options & (int)exec_options::pipe_error))
				{
					_err.reset(new io::pipe(_ioservice));
				}

				_impl.reset(new process(
					std::get<0>(found),
					_in ? _in->in().get() : io::handler(),
					_out ? _out->out().get() : io::handler(),
					_err ? _err->out().get() : io::handler()));
			}

			~exec()
			{
				if(_waiter.joinable())
				{
					_waiter.join();
				}
			}

			void work_path(const fs::filepath & path)
			{
				std::error_code err;

				process_work_path(*_impl, path, err);

				if (err)
				{
					throw std::system_error(err);
				}
			}

			template <typename ...Args>
			void start(Args &&...args)
			{
				start({ std::forward<Args>(args)... });
			}

			void start(const std::vector<std::string> & args)
			{
				std::unique_lock<std::mutex> lock(_mutex);

				if(!_joined)
				{
					throw std::system_error(errc::exec_already_started);
				}

				std::error_code err;
				process_start(*_impl, err, args);

				if (err)
				{
					throw std::system_error(err);
				}

				_joined = false;

				_waiter = std::thread(std::bind(&exec::wait_proc, this));
			}

			int wait()
			{
				std::unique_lock<std::mutex> lock(_mutex);

				for (;;)
				{
					if (_joined)
					{
						if (_options != exec_options::none)
						{
							io_service_run(lock);
						}

						if (_errcode)
						{
							throw std::system_error(_errcode);
						}

						_waiter.join();

						return _exit_code;
					}

				
					if(_options != exec_options::none)
					{
						io_service_run(lock);
					}
					else
					{
						_condition.wait(lock);
					}
				}
			}

			template <typename ...Args>
			int run(Args &&...args)
			{
				start(std::forward<Args>(args)...);

				return wait();
			}

			io::io_stream& in() const
			{
				return _in->out();
			}

			io::io_stream& out() const
			{
				return _out->in();
			}

			io::io_stream& err() const
			{
				return _err->in();
			}

		private:

			template<typename Mutex>
			void io_service_run(std::unique_lock<Mutex> & lock)
			{
				using namespace std::chrono;

				for (;;)
				{
					std::error_code err;

					_ioservice.run_one(milliseconds(10), err);

					if (!err) continue;

					if (err == io::errc::io_service_closed)
					{
						_condition.wait(lock);

						break;
					}

					if (std::errc::timed_out == err)
					{
						_condition.wait_for(lock, milliseconds(10));

						break;
					}

					if (err)
					{
						throw std::system_error(err);
					}
				}
			}

			void wait_proc()
			{
				_exit_code = process_wait(*_impl, _errcode);

				std::lock_guard<std::mutex> lock(_mutex);

				_joined = true;

				_condition.notify_all();
			}
		private:
			bool							_joined;
			exec_options					_options;
			std::unique_ptr<process>		_impl;
			std::mutex						_mutex;
			std::condition_variable			_condition;
			std::thread						_waiter;
			io::io_service					_ioservice;
			std::unique_ptr<io::pipe>		_in;
			std::unique_ptr<io::pipe>		_out;
			std::unique_ptr<io::pipe>		_err;
			int								_exit_code;
			std::error_code					_errcode;
		};

	}
}

#endif //LEMON_OS_EXEC_HPP