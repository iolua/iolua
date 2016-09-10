/**
 * 
 * @file     exec_options
 * @brief    Copyright (C) 2016  yayanyang All Rights Reserved 
 * @author   yayanyang
 * @date     2016/01/14
 */
#ifndef LEMON_OS_EXEC_OPTIONS_HPP
#define LEMON_OS_EXEC_OPTIONS_HPP

namespace lemon{namespace os{
	enum class exec_options
	{
		none = 0, pipe_in = 1, pipe_out = 2,pipe_error = 4
	};
}}

#endif //LEMON_OS_EXEC_OPTIONS_HPP