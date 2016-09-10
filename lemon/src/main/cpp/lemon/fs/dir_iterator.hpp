/**
 * 
 * @file     dir_iterator
 * @brief    Copyright (C) 2015  yayanyang All Rights Reserved 
 * @author   yayanyang
 * @date     2015/12/09
 */
#ifndef LEMON_FS_DIR_ITERATOR_HPP
#define LEMON_FS_DIR_ITERATOR_HPP
#include <memory>

#include <lemon/fs/filepath.hpp>

#ifdef WIN32
#include <lemon/fs/dir_iterator_win32.hpp>
#else
#include <lemon/fs/dir_iterator_posix.hpp>
#endif //WIN32

namespace lemon{ namespace fs{

	enum class directory_options {
		none,
		follow_directory_symlink,
		skip_permission_denied
	};
	
	class directory_iterator_impl;

	class directory_iterator
	{
	public:
		directory_iterator(const filepath &path)
			:_iterator(new directory_iterator_impl(path))
		{
			
		}

		bool has_next() const
		{
			return _iterator->has_next();
		}

		filepath operator()()
		{
			return _iterator->next();
		}

	private:
		std::unique_ptr<directory_iterator_impl> _iterator;
	};

	
}}

#endif //LEMON_FS_DIR_ITERATOR_HPP