#ifndef LEMON_FS_DIR_ITERATOR_POSIX_HPP
#define LEMON_FS_DIR_ITERATOR_POSIX_HPP

#ifndef WIN32

#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>

#include <memory>
#include <system_error>

#include <lemon/config.h>
#include <lemon/fs/filepath.hpp>

namespace lemon { namespace fs {

	class directory_iterator_impl;

	class directory_iterator_impl
	{
	public:
		directory_iterator_impl(const filepath &path)
		{
			_dir = opendir(path.string().c_str());

			if (_dir == nullptr)
			{
				throw std::system_error(errno, std::system_category());
			}
		}

		~directory_iterator_impl()
		{
			closedir(_dir);
		}

		bool has_next() const
		{
			dirent *entry = readdir(_dir);

			if(nullptr != entry)
			{
				_filepath = std::string(entry->d_name);
				return true;
			}

			return false;
		}

		filepath next()
		{
			return _filepath;
		}

	private:
		DIR			        *_dir;
		mutable filepath	_filepath;
	};
}
}

#endif //WIN32

#endif //LEMON_FS_DIR_ITERATOR_POSIX_HPP

