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
			exec(lemon::io::io_service& service, const std::string & name)
				:exec(service, name, (int) exec_options::none)
			{

			}

			exec(lemon::io::io_service& service, const std::string & name, int options)
			{
				auto found = lookup(name);

				if (!std::get<1>(found))
				{
					throw std::system_error((int)errc::command_not_found, os_error_category());
				}

				if ((options & (int)exec_options::pipe_in))
				{
					_in.reset(new lemon::io::pipe(service));
				}

				if ((options & (int)exec_options::pipe_out))
				{
					_out.reset(new lemon::io::pipe(service));
				}

				if ((options & (int)exec_options::pipe_error))
				{
					_err.reset(new lemon::io::pipe(service));
				}

				_impl.reset(new process(
					std::get<0>(found),
					_in ? _in->in().get() : io::handler(),
					_out ? _out->out().get() : io::handler(),
					_err ? _err->out().get() : io::handler()));
			}

			~exec()
			{
				if(_wait_thread.joinable())
				{
					_wait_thread.join();
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

				std::error_code err;
				process_start(*_impl, err, args);

				if (err)
				{
					throw std::system_error(err);
				}

				_wait_thread = std::thread([&]{

					std::error_code ec;
					int result = process_wait(*_impl,ec);

					std::unique_lock<std::mutex> lock1(_mutex);

					_ec = ec;

					_exit_code = result;

					if(_wait)
					{
						_wait(_exit_code,_ec);
					}

					_impl.reset();
				});

				if(_in) _in->close_in();

				if(_out) _out->close_out();

				if(_err) _err->close_out();
			}

			template <typename ...Args>
			int run(Args &&...args)
			{
				start(std::forward<Args>(args)...);

				return 0;
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

			io::io_stream* release_in() const
			{
				return _in->release_out();
			}

			io::io_stream* release_out() const
			{
				return _out->release_in();
			}

			io::io_stream* release_err() const
			{
				return _err->release_in();
			}

			template <typename Callback>
			void wait(Callback && callback)
			{
				std::unique_lock<std::mutex> lock(_mutex);

				if(!_wait_thread.joinable())
				{
					callback(_exit_code,_ec);
				}
				else
				{
					_wait = callback;
				}
			}

			int wait()
			{
				std::error_code ec;
				int result = process_wait(*_impl,ec);

				if(ec)
				{
					throw std::system_error(ec);
				}

				return result;
			}

		private:
			std::unique_ptr<process>								_impl;
			std::mutex												_mutex;
			std::thread												_wait_thread;
			std::error_code 										_ec;
			int 													_exit_code;
			std::unique_ptr<io::pipe>								_in;
			std::unique_ptr<io::pipe>								_out;
			std::unique_ptr<io::pipe>								_err;
			std::function<void(int exitcode,std::error_code & ec)>	_wait;
		};

	}
}

#endif //LEMON_OS_EXEC_HPP