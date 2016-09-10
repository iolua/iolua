
#include <locale>
#include <vector>

#include <stdlib.h>

#ifdef __APPLE__
#include <TargetConditionals.h>
#endif //

#include <sstream>
#include <lemon/fs/fs.hpp>
#include <lemon/strings.hpp>
#include <lemon/os/sysinfo.hpp>

namespace lemon { namespace os {
    typedef std::wstring_convert<std::codecvt<wchar_t, char, std::mbstate_t>, wchar_t> convert;

    host_t hostname()
    {

    #ifdef WIN32
		return host_t::Windows;
    #elif defined(__linux)
        #ifdef __android
            return host_t::Android;
        #else
            return host_t::Linux;
        #endif
    #elif defined(__sun)
        return host_t::Solaris;
    #elif defined(__hppa)
        return host_t::HPUX;
    #elif defined(_AIX)
        return host_t::AIX;
    #elif defined(__APPLE__)
    #if TARGET_OS_SIMULATOR == 1
        return host_t::iOS_Simulator;
    #elif TARGET_OS_IPHONE == 1
        return host_t::iOS;
    #elif TARGET_OS_MAC == 1
        return host_t::OSX;
    #else
        return host_t::OSX_Unknown;
    #endif
    #endif //WIN32
    }

	arch_t arch()
	{
#if defined(__alpha__) || defined(_M_ALPHA) || defined(__alpha)
		return arch_t::Alpha;
#elif defined(__amd64__) || defined(__amd64) || defined(_M_X64)
		return arch_t::AMD64;
#elif defined(__arm__) || defined(_ARM) || defined(_M_ARM) || defined(_M_ARMT) || defined(__arm)
		return arch_t::ARM;
#elif defined(__aarch64__) || defined(__arm64__)
		return arch_t::ARM64;
#elif defined(__hppa__) || defined(__HPPA__)
		return arch_t::HP_PA;
#elif defined(__i386__) || defined(__i386) || defined(__i386) || defined(_M_IX86) || defined(_X86_)
		return arch_t::X86;
#elif defined(__mips__) || defined(__mips)
		return arch_t::MIPS;
#elif defined(__powerpc) || defined(_M_PPC) || defined(_ARCH_PPC64) || defined(_ARCH_PPC)
		return arch_t::PowerPC;
#elif defined(__sparc__)
		return arch_t::SPARC;
#endif
	}



    #ifdef WIN32
    std::tuple<std::string, bool> getenv(const std::string &name)
    {
        auto namew = convert().from_bytes(name);

        DWORD length = ::GetEnvironmentVariableW(namew.c_str(), NULL, 0);

        if(length == 0)
        {
            return std::make_tuple(std::string(), false);
        }

        std::vector<wchar_t> buff(length);

        ::GetEnvironmentVariableW(namew.c_str(), &buff[0], (DWORD)buff.size());

        return std::make_tuple(convert().to_bytes(&buff[0]), true);
    }

	void setenv(const std::string &name, const std::string &val,std::error_code &ec)
	{
		auto namew = convert().from_bytes(name);

		auto valnew = convert().from_bytes(val);

		if (!::SetEnvironmentVariableW(namew.c_str(), valnew.c_str()))
		{
			ec = std::error_code(GetLastError(),std::system_category());
		}
	}

    #else

    std::tuple<std::string,bool> getenv(const std::string &name)
    {
        const char *val = ::getenv(name.c_str());

        if(val)
        {
            return std::make_tuple(std::string(val),true);
        }

        return std::make_tuple(std::string(), false);
    }

	void setenv(const std::string &name, const std::string &val, std::error_code &ec)
	{
		if (-1 == ::setenv(name.c_str(), val.c_str(),1))
		{
			ec = std::error_code(errno, std::system_category());
		}
	}

    #endif //WIN32


    std::string execute_suffix()
    {
    #ifdef WIN32
        return ".exe";
    #else
        return "";
    #endif
    }

#ifndef WIN32
    std::string tmpdir(std::error_code & )
    {
        auto val = getenv("TMPDIR");

        if(std::get<1>(val))
        {
            return std::get<0>(val);
        }
#ifdef __android
        return "/data/local/tmp";
#endif

        return "/tmp";
    }
#else


	std::string tmpdir(std::error_code & err)
	{
		wchar_t buff[MAX_PATH + 1];

		auto length = ::GetTempPathW(MAX_PATH, buff);

		if(length == 0)
		{
			err = std::error_code(GetLastError(),std::system_category());

			return "";
		}

		return convert().to_bytes(std::wstring(buff, buff + length));
	}
#endif


    std::tuple<std::string, bool> lookup(const std::string & cmd)
    {

		if(fs::exists(cmd))
		{
			return std::make_tuple(fs::absolute(cmd).string(), true);
		}

        auto path = os::getenv("PATH");

        if(!std::get<1>(path))
        {
            return std::make_tuple(std::string(),false);
        }

    #ifdef WIN32
        const std::string delimiter = ";";
		const std::vector<std::string> extends = { ".exe",".cmd",".bat",".com" };
    #else
        const std::string delimiter = ":";
		const std::vector<std::string> extends = { "" };
    #endif //WIN32

        auto paths = strings::split(std::get<0>(path), delimiter);

    #ifdef WIN32
        DWORD length = ::GetSystemDirectoryW(0, 0);

            std::vector<wchar_t> buff(length);

            ::GetSystemDirectoryW(&buff[0], (UINT)buff.size());

            paths.push_back(convert().to_bytes(&buff[0]));
    #else
    #endif

        for(auto p : paths)
        {
			for (auto extend : extends)
			{
				auto fullPath = fs::filepath(p) / (cmd + extend);

				if (fs::exists(fullPath))
				{
					return std::make_tuple(fullPath.string(), true);
				}
			}
           
        }

        return std::make_tuple(std::string(), false);
    }
}}