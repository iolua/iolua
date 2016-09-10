/**
 * 
 * @file     file_status
 * @brief    Copyright (C) 2016  yayanyang All Rights Reserved 
 * @author   yayanyang
 * @date     2016/01/12
 */
#ifndef LEMON_FS_FILE_STATUS_HPP
#define LEMON_FS_FILE_STATUS_HPP

#include <chrono>

namespace lemon{namespace fs{
	enum class perms {	// names for permissions
		none = 0,
		owner_read = 0400,	// S_IRUSR
		owner_write = 0200,	// S_IWUSR
		owner_exec = 0100,	// S_IXUSR
		owner_all = 0700,	// S_IRWXU
		group_read = 040,	// S_IRGRP
		group_write = 020,	// S_IWGRP
		group_exec = 010,	// S_IXGRP
		group_all = 070,	// S_IRWXG
		others_read = 04,	// S_IROTH
		others_write = 02,	// S_IWOTH
		others_exec = 01,	// S_IXOTH
		others_all = 07,	// S_IRWXO
		all = 0777,
		set_uid = 04000,	// S_ISUID
		set_gid = 02000,	// S_ISGID
		sticky_bit = 01000,	// S_ISVTX
		mask = 07777,
		unknown = 0xFFFF,
		add_perms = 0x10000,
		remove_perms = 0x20000,
		resolve_symlinks = 0x40000
	};

	enum class file_type {
		none = 0,
		not_found = -1,
		regular = 1,
		directory = 2,
		symlink = 3,
		block = 4,
		character = 5,
		fifo = 6,
		socket = 7,
		unknown = 8
	};

	typedef std::chrono::system_clock::time_point file_time_type;

	class file_status
	{	// stores file status
	public:
		explicit file_status(
			file_type _Ftype = file_type::none,
			perms _Prms = perms::unknown) noexcept
			: _Myftype(_Ftype), _Myperms(_Prms)
		{	// construct with optional arguments
		}

		file_status(const file_status&) noexcept = default;
		~file_status() noexcept = default;
		file_status& operator=(const file_status&) noexcept = default;

		file_status(file_status&&) = default;
		file_status& operator=(file_status&&) = default;

		file_type type() const noexcept
		{	// get file type
			return (_Myftype);
		}

		perms permissions() const noexcept
		{	// get file permissions
			return (_Myperms);
		}

		void type(file_type _Ftype) noexcept
		{	// set file type
			_Myftype = _Ftype;
		}

		void permissions(perms _Prms) noexcept
		{	// set permissions
			_Myperms = _Prms;
		}

	private:
		file_type _Myftype;
		perms _Myperms;
	};
}}

#endif //LEMON_FS_FILE_STATUS_HPP