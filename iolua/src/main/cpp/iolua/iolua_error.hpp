#ifndef IOLUA_ERROR_HPP
#define IOLUA_ERROR_HPP

#include <string>
#include <system_error>

namespace iolua {
    enum class errc
    {
        out_of_memory,create_task_error, op_on_closed_chan
    };

    class io_error_category : public std::error_category
    {
    private:

        const char *name() const throw()
        {
            return "lemon::os::os_error_category";
        }

        std::string message(int val) const
        {
            switch ((errc)val)
            {
                case errc::out_of_memory:
                    return "iolua out of memory";
                case errc::create_task_error:
                    return "create task error";
				case errc::op_on_closed_chan:
					return "call method on closed chan";
            }

			return "";
        }

        std::error_condition default_error_condition(int _Errval) const throw()
        {
            return std::error_condition(_Errval, *this);
        }

        bool equivalent(int _Errval, const std::error_condition& _Cond) const throw()
        {
            return _Errval == _Cond.value();
        }

        bool equivalent(const std::error_code& _Code, int _Errval) const throw()
        {
            return _Errval == _Code.value();
        }
    };

    inline std::error_code make_error_code(errc err) noexcept {
        static io_error_category io_error_category;

        return std::error_code(static_cast<int>(err), io_error_category);
    }
}

namespace std {
    template<> struct is_error_code_enum<::iolua::errc> : true_type {};
}

#endif