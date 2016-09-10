#ifndef LEMON_OS_SYSINFO_HPP
#define LEMON_OS_SYSINFO_HPP
#include <tuple>
#include <string>
#include <system_error>
#include <lemon/config.h>

namespace lemon{ namespace os{
	/**
	 * get the lemon host name
	 */
	enum class host_t {
		Unknown,Windows,Linux,Solaris,HPUX,AIX,iOS_Simulator,iOS,OSX,OSX_Unknown,Android
	};

	enum class arch_t {
		Alpha,X86,AMD64,ARM,ARM64, HP_PA, MIPS, PowerPC,SPARC
	};


	void setenv(const std::string &name, const std::string &val,std::error_code & ec);

	//
	// get host name
	host_t hostname();

	arch_t arch();

	//
	// get env by name
	std::tuple<std::string,bool> getenv(const std::string&);

	std::string execute_suffix();

	std::string tmpdir(std::error_code & err);

	inline std::string tmpdir()
	{
		std::error_code err;

		auto val = tmpdir(err);

		if(err)
		{
			throw std::system_error(err);
		}

		return val;
	}

    std::tuple<std::string,bool> lookup(const std::string & cmd);
}}

#endif //LEMON_OS_SYSINFO_HPP