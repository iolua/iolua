#ifndef LEMON_FS_DIR_ITERATOR_WIN32_HPP
#define LEMON_FS_DIR_ITERATOR_WIN32_HPP


#include <memory>
#include <system_error>

#include <lemon/config.h>
#include <lemon/fs/filepath.hpp>

namespace lemon {namespace fs {

		class directory_iterator_impl;

		class directory_iterator_impl
		{
		public:
			directory_iterator_impl(const filepath &path)
			{
				auto findpath = path;

				findpath /= "/*";

				if ((_handler = FindFirstFileW(findpath.wstring().c_str(), &_context)) == INVALID_HANDLE_VALUE) 
				{
					throw std::system_error(GetLastError(), std::system_category());
				}

				_has_next = true;
			}

			~directory_iterator_impl()
			{
				FindClose(_handler);
			}

			bool has_next() const
			{
				return _has_next;
			}

			filepath next()
			{
				filepath path(_context.cFileName);

				_has_next = FindNextFileW(_handler, &_context)?true:false;

				return path;
			}

		private:
			bool					_has_next;

			WIN32_FIND_DATAW		_context;

			HANDLE					_handler;
		};
	}
}


#endif //LEMON_FS_DIR_ITERATOR_WIN32_HPP