/**
 * 
 * @file     handler
 * @brief    Copyright (C) 2015  yayanyang All Rights Reserved 
 * @author   yayanyang
 * @date     2015/12/07
 */
#ifndef LEMON_IO_HANDLER_HPP
#define LEMON_IO_HANDLER_HPP

#include <cstddef>
#include <lemon/config.h>

namespace lemon{ namespace io{
#if defined(__APPLE__)
	using handler = uintptr_t;
#elif defined(WIN32)
	using handler = HANDLE;
#else
	using handler = int;
#endif
}}

#endif //LEMON_IO_HANDLER_HPP