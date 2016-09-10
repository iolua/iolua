
#ifndef LEMON_OS_EXEC_WIN32_HPP
#define LEMON_OS_EXEC_POSIX_HPP


#include <string>
#include <thread>
#include <utility>
#include <iostream>
#include <unordered_map>


#include <lemon/config.h>
#include <lemon/io/io.hpp>
#include <lemon/fs/fs.hpp>
#include <lemon/nocopy.hpp>
#include <lemon/log/log.hpp>
#include <lemon/nocopy.hpp>
#include <lemon/os/exec_options.hpp>
#include <lemon/io/io_errors.hpp>
#include <lemon/os/args_convert.hpp>

namespace lemon {
	namespace os {

		class process : private  nocopy
		{
		public:
			process(const std::string & path,HANDLE in,HANDLE out,HANDLE err)
				:_path(path)
				,_workpath(fs::current_path())
				,_logger(lemon::log::get("process"))
				,_in(in)
				,_out(out)
				,_err(err)
			{
				
			}

			void start(std::error_code & err, const std::vector<std::string> & args) noexcept
			{

				std::wstring_convert<std::codecvt<wchar_t, char, std::mbstate_t>, wchar_t> convert;

				SECURITY_ATTRIBUTES sa;

				sa.bInheritHandle = TRUE;

				sa.lpSecurityDescriptor = NULL;

				sa.nLength = sizeof(SECURITY_ATTRIBUTES);

				HANDLE stdinHandler = GetStdHandle(STD_INPUT_HANDLE);
				HANDLE stdoutHandler = GetStdHandle(STD_OUTPUT_HANDLE);
				HANDLE stderrHandler = GetStdHandle(STD_ERROR_HANDLE);

				if(_in)
				{
					stdinHandler = _in;
				}

				if (_out)
				{
					stdoutHandler = _out;
				}

				if (_err)
				{
					stderrHandler = _err;
				}

				std::wstringstream stream;

				stream << L"\"" << _path.wstring() << L"\"";

				std::vector<wchar_t> cmdline;

				LPWSTR lpCommandLine = NULL;

				for (auto arg : args)
				{
					stream << " " << convert.from_bytes(arg);
				}

				auto str = stream.str();

				cmdline.assign(str.begin(), str.end());

				cmdline.push_back(L'\0');
				lpCommandLine = &cmdline[0];

				LPVOID lpEnvironment = NULL;

				STARTUPINFOW si;

				ZeroMemory(&_processInfo, sizeof(_processInfo));

				ZeroMemory(&si, sizeof(STARTUPINFOW));

				si.cb = sizeof(STARTUPINFOW);
				si.hStdError = stderrHandler;
				si.hStdOutput = stdoutHandler;
				si.hStdInput = stdinHandler;
				si.dwFlags |= STARTF_USESTDHANDLES | FILE_FLAG_OVERLAPPED;

				if (!::CreateProcessW(
					NULL,
					lpCommandLine,
					NULL,
					NULL,
					TRUE,
					0,
					lpEnvironment,
					_workpath.empty() ? NULL : _workpath.wstring().c_str(),
					&si,
					&_processInfo))
				{
					err = std::error_code(GetLastError(), std::system_category());
				}
			}

			int wait(std::error_code & err) noexcept
			{
				if (WAIT_FAILED == WaitForSingleObject(_processInfo.hProcess, INFINITE))
				{
					err = std::error_code(GetLastError(), std::system_category());
				}

				DWORD exitCode;

				GetExitCodeProcess(_processInfo.hProcess, &exitCode);

				return exitCode;
			}

			void work_path(const fs::filepath & path, std::error_code &) noexcept
			{
				_workpath = path;
			}

			const fs::filepath& work_path() const noexcept
			{
				return _workpath;
			}

			template <typename Env>
			void env(Env &&env)
			{

			}

			const std::unordered_map<std::string, std::string >& env() const noexcept
			{
				return _env;
			};
		private:
			PROCESS_INFORMATION								_processInfo;
			const fs::filepath                              _path;
			fs::filepath                                    _workpath;
			std::unordered_map<std::string, std::string >    _env;
			const lemon::log::logger                        &_logger;
			HANDLE											_in;
			HANDLE											_out;
			HANDLE											_err;
		};


		inline void process_start(process &impl, std::error_code & err, const std::vector<std::string> & args) noexcept
		{
			impl.start(err, args);
		}

		/**
		* wait process exit
		*/
		inline int process_wait(process &impl, std::error_code & err) noexcept
		{
			return impl.wait(err);
		}

		/**
		* set the process work path
		*/
		inline void process_work_path(process &proc, const fs::filepath & path, std::error_code & err) noexcept
		{
			proc.work_path(path, err);
		}

		/**
		* get the process work path
		*/
		inline fs::filepath process_work_path(process &proc) noexcept
		{
			return proc.work_path();
		}

		/**
		* set the new process env
		*/
		template <typename Env>
		inline void process_env(process & proc, Env &&env)
		{
			proc.env(std::forward<Env>(env));
		}

		/**
		* get the process env
		*/
		inline const std::unordered_map<std::string, std::string>& process_env(process & proc)
		{
			return proc.env();
		};
	}
}

#endif //LEMON_OS_EXEC_POSIX_HPP