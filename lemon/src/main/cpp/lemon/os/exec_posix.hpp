
#ifndef LEMON_OS_EXEC_POSIX_HPP
#define LEMON_OS_EXEC_POSIX_HPP

#include <unistd.h>
#include <sys/wait.h>


#include <lemon/config.h>
#include <lemon/fs/fs.hpp>
#include <lemon/nocopy.hpp>
#include <lemon/log/log.hpp>
#include <lemon/nocopy.hpp>
#include <lemon/os/args_convert.hpp>

#include <string>
#include <thread>
#include <utility>
#include <iostream>
#include <unordered_map>
#include <lemon/io/handler.hpp>

namespace lemon{
    namespace os {

        typedef ::pid_t process_id_t;

        /**
         * get self process idm
         * @return
         */
        inline process_id_t current_process_id() {
            return getpid();
        }

        /**
         * get parent process id
         * @return process_id_t
         */
        inline process_id_t parent_process_id() {
            return getppid();
        }
    }
}

namespace lemon{ namespace os{



    class process : private  nocopy
    {
    public:
        process(const std::string & path,io::handler in,io::handler out,io::handler err)
                :_path(path)
                ,_workpath(fs::current_path())
                ,_logger(lemon::log::get("process"))
                ,_stdin(in)
                ,_stdout(out)
                ,_stderr(err)
        {

        }

        void start(std::error_code & err,const std::vector<std::string> & args) noexcept
        {
            _pid = fork();

            switch (_pid)
            {
            case -1:
                lemonE(_logger,"catch error :%s", std::error_code(errno,std::system_category()).message().c_str());
                //throw std::system_error(errno,std::system_category(),"create stderr pipe error");
            case 0:
                // child process
                try
                {
                    exec(err,args);
                }
                catch(const std::exception e)
                {
                    lemonE(_logger,"catch error :%s",e.what());
                }

                break;
            default:
                break;
            }
        }

        int wait(std::error_code & err) noexcept
        {
            int status = 0;

            pid_t ret;

            do{

                ret = ::waitpid(_pid, &status, 0);

            } while ((ret == -1 && errno == EINTR) || (ret != -1 && !WIFEXITED(status)));

            if (ret == -1 && errno != ECHILD)
            {
                err = std::error_code(errno,std::system_category());
                lemonE(_logger,"catch error :%s", err.message().c_str())

            }

            return WEXITSTATUS(status);
        }

        void work_path(const fs::filepath & path, std::error_code & err) noexcept
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

        const std::unordered_map<std::string,std::string >& env() const noexcept
        {
            return _env;
        };

    private:

        void exec(std::error_code & err,const std::vector<std::string> & buff)
        {
            fs::current_path(_workpath);

            // redirect stdin/stdout/stderr

            if(_stdin)
            {
                if (dup2(_stdin, STDIN_FILENO) == -1) {
                    perror("redirecting stdin");
                    return;
                }
            }

            if(_stdout)
            {
                // redirect stdout
                if (dup2(_stdout, STDOUT_FILENO) == -1) {
                    perror("redirecting stdout");
                    return;
                }
            }

            if(_stderr)
            {
                // redirect stderr
                if (dup2(_stderr, STDERR_FILENO) == -1) {
                    perror("redirecting stderr");
                    return;
                }
            }


            ::close(_stdin);
            ::close(_stdout);
            ::close(_stderr);

            const char ** argv = new const char*[buff.size() + 2];

            argv[0] = _path.string().c_str();

            int i = 1;

            for(auto &arg : buff)
            {
                argv[i] = arg.c_str();

                i ++;
            }

            argv[i] = NULL;

            if (-1 == execv(_path.string().c_str(), (char*const*)argv))
            {
                printf("exec error:%s\n\t%s\n",_path.string().c_str(),std::error_code(errno,std::system_category()).message().c_str());
                //throw  std::system_error(errno,std::system_category(),_path.string());
            }
        }

    private:
        pid_t                                           _pid;
        const fs::filepath                              _path;
        fs::filepath                                    _workpath;
        std::unordered_map<std::string,std::string >    _env;
        const lemon::log::logger                        &_logger;
        int                                             _stdin;
        int                                             _stdout;
        int                                             _stderr;
    };


    inline void process_start(process &impl,std::error_code & err,const std::vector<std::string> & args) noexcept
    {
        impl.start(err,args);
    }

    /**
     * wait process exit
     */
    inline int process_wait(process &impl,std::error_code & err) noexcept
    {
        return impl.wait(err);
    }

    /**
     * set the process work path
     */
    inline void process_work_path(process &proc,const fs::filepath & path, std::error_code & err) noexcept
    {
        proc.work_path(path,err);
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
    inline const std::unordered_map<std::string,std::string>& process_env(process & proc)
    {
        return proc.env();
    };
}}

#endif //LEMON_OS_EXEC_POSIX_HPP