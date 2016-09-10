#ifndef LEMON_IO_BUFF_HPP
#define LEMON_IO_BUFF_HPP

#include <cstddef>
#include <cstring>

namespace lemon{ namespace io{

	struct buffer
	{
		void			    *data;
		std::size_t			length;
	};

	struct const_buffer
	{
		const void		    *data;
		std::size_t			length;
	};

    template<size_t N>
	inline buffer buff(char (&source)[N])
    {
        return {source,N};
    }

	template<size_t N>
	inline const_buffer cbuff(const char(&source)[N])
	{
		return{ source,N };
	}

	inline const_buffer cbuff(const char * source)
	{
		return { source,strlen(source) };
	}

	
}}

#endif //LEMON_IO_BUFF_HPP