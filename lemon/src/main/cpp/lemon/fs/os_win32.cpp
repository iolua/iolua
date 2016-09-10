#include <lemon/fs/os.hpp>

#ifdef WIN32


namespace lemon {namespace fs {

#define __READONLY_PERMS	\
	perms((int)perms::all & ~(int)__WRITE_PERMS)
#define __WRITE_PERMS	\
	perms((int)perms::owner_write | (int)perms::group_write | (int)perms::others_write)


#define _FILE_ATTRIBUTE_REGULAR	\
	(FILE_ATTRIBUTE_ARCHIVE \
	| FILE_ATTRIBUTE_COMPRESSED \
	| FILE_ATTRIBUTE_ENCRYPTED \
	| FILE_ATTRIBUTE_HIDDEN \
	| FILE_ATTRIBUTE_NORMAL \
	| FILE_ATTRIBUTE_NOT_CONTENT_INDEXED \
	| FILE_ATTRIBUTE_OFFLINE \
	| FILE_ATTRIBUTE_READONLY \
	| FILE_ATTRIBUTE_SPARSE_FILE \
	| FILE_ATTRIBUTE_SYSTEM \
	| FILE_ATTRIBUTE_TEMPORARY)


	filepath current_path(std::error_code &e) noexcept
	{
		wchar_t buff[MAX_PATH];

		DWORD length = ::GetCurrentDirectoryW(MAX_PATH, buff);

		if (length == 0)
		{
			e = std::error_code(GetLastError(), std::system_category());

			return "";
		}

		return filepath(std::wstring(buff, length));
	}

	void current_path(const filepath & path, std::error_code &err) noexcept
	{
		if (!::SetCurrentDirectoryW(path.wstring().c_str()))
		{
			err = std::error_code(GetLastError(), std::system_category());
		}
	}

	bool exists(const filepath & path) noexcept
	{
		if (INVALID_FILE_ATTRIBUTES == GetFileAttributesW(path.wstring().c_str()))
		{
			return false;
		}

		return true;
	}

	void create_directory(const filepath& path, std::error_code & err) noexcept
	{
		if (0 == CreateDirectoryW(path.wstring().c_str(), NULL))
		{
			err = std::error_code(GetLastError(), std::system_category());
		}
	}

	void create_symlink(const filepath &from, const filepath &to, std::error_code &err) noexcept
	{

		auto flags = is_directory(from) ? SYMBOLIC_LINK_FLAG_DIRECTORY : 0;



		if (0 == CreateSymbolicLinkW(to.wstring().c_str(), from.wstring().c_str(), flags)) {

			err = std::error_code(GetLastError(), std::system_category());
		}
	}


	bool is_directory(const filepath &source) noexcept
	{
		auto attrs = GetFileAttributesW(source.wstring().c_str());
		if (INVALID_FILE_ATTRIBUTES == attrs)
		{
			return false;
		}

		if ((attrs & FILE_ATTRIBUTE_DIRECTORY) != 0)
		{
			return true;
		}

		return false;
	}

	void remove_file(const filepath & path, std::error_code &err) noexcept
	{
		auto pathName = path.wstring();

		if (!is_directory(path))
		{
			bool flag = false;

			for (;;)
			{
				if (0 == DeleteFileW(pathName.c_str())) {

					if (GetLastError() == ERROR_ACCESS_DENIED && flag == false) {
						DWORD attrs = GetFileAttributesW(pathName.c_str());
						attrs &= ~FILE_ATTRIBUTE_READONLY;
						SetFileAttributesW(pathName.c_str(), attrs);
						flag = true;
						continue;
					}

					err = std::error_code(GetLastError(), std::system_category());
				}

				break;
			}

			return;
		}

		if (0 == RemoveDirectoryW(pathName.c_str())) {
			err = std::error_code(GetLastError(), std::system_category());
		}
	}

	void copy_file(const filepath & from, const filepath& to, std::error_code & errc) noexcept
	{
		if(!::CopyFileW(from.wstring().c_str(), to.wstring().c_str(), TRUE))
		{
			errc = std::error_code(GetLastError(),std::system_category());
		}
	}

	static file_type _Map_mode(int _Mode)
	{	
		if ((_Mode &FILE_ATTRIBUTE_REPARSE_POINT) != 0)
			return (file_type::symlink);
		
		// map Windows file attributes to file_status
		if ((_Mode & FILE_ATTRIBUTE_DIRECTORY) != 0)
			return (file_type::directory);
		else if ((_Mode & _FILE_ATTRIBUTE_REGULAR) != 0)
			return (file_type::regular);
		else
			return (file_type::unknown);
	}

	file_status status(const filepath & path, std::error_code &) noexcept
	{
		WIN32_FILE_ATTRIBUTE_DATA data;

		perms pmode;

		if(GetFileAttributesExW(path.wstring().c_str(), GetFileExInfoStandard, &data))
		{
			pmode = data.dwFileAttributes & FILE_ATTRIBUTE_READONLY ? __READONLY_PERMS : perms::all;

			return file_status(_Map_mode(data.dwFileAttributes),pmode);
		}


		int _Errno = GetLastError();

		if (_Errno == ERROR_BAD_NETPATH
			|| _Errno == ERROR_BAD_PATHNAME
			|| _Errno == ERROR_FILE_NOT_FOUND
			|| _Errno == ERROR_INVALID_DRIVE
			|| _Errno == ERROR_INVALID_NAME
			|| _Errno == ERROR_INVALID_PARAMETER
			|| _Errno == ERROR_PATH_NOT_FOUND)

			return file_status(file_type::not_found);
		else
			return file_status(file_type::unknown);
	}

	std::uintmax_t file_size(const filepath& path, std::error_code& ec)
	{
		WIN32_FILE_ATTRIBUTE_DATA data;

		if (GetFileAttributesExW(path.wstring().c_str(), GetFileExInfoStandard, &data))
		{
			return ((uintmax_t)data.nFileSizeHigh << 32 | data.nFileSizeLow);
		}

		ec = std::make_error_code(std::errc::operation_not_permitted);

		return (uintmax_t)-1;
	}
}}


#endif