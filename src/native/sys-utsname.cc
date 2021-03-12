/*
 * <sys/sendfile.h> wrapper functions.
 *
 * Authors:
 *   Jonathan Pryor (jonpryor@vt.edu)
 *
 * Copyright (C) 2004 Jonathan Pryor
 */
#if defined (HAVE_CONFIG_H)
#include <config.h>
#endif

#include <sys/types.h>
#include <cerrno>
#include <cstdlib>

#include "map.hh"
#include "mph.hh"

#include <sys/utsname.h>

static const mph_string_offset_t
utsname_offsets[] = {
	MPH_STRING_OFFSET(struct utsname, sysname,  MPH_STRING_OFFSET_ARRAY),
	MPH_STRING_OFFSET(struct utsname, nodename, MPH_STRING_OFFSET_ARRAY),
	MPH_STRING_OFFSET(struct utsname, release,  MPH_STRING_OFFSET_ARRAY),
	MPH_STRING_OFFSET(struct utsname, version,  MPH_STRING_OFFSET_ARRAY),
	MPH_STRING_OFFSET(struct utsname, machine,  MPH_STRING_OFFSET_ARRAY)
};

static const mph_string_offset_t
mph_utsname_offsets[] = {
	MPH_STRING_OFFSET(struct Mono_Posix_Syscall__Utsname, sysname,  MPH_STRING_OFFSET_PTR),
	MPH_STRING_OFFSET(struct Mono_Posix_Syscall__Utsname, nodename, MPH_STRING_OFFSET_PTR),
	MPH_STRING_OFFSET(struct Mono_Posix_Syscall__Utsname, release,  MPH_STRING_OFFSET_PTR),
	MPH_STRING_OFFSET(struct Mono_Posix_Syscall__Utsname, version,  MPH_STRING_OFFSET_PTR),
	MPH_STRING_OFFSET(struct Mono_Posix_Syscall__Utsname, machine,  MPH_STRING_OFFSET_PTR)
};

int
Mono_Posix_Syscall_uname (struct Mono_Posix_Syscall__Utsname *buf)
{
	if (buf == nullptr) {
		errno = EFAULT;
		return -1;
	}

	struct utsname _buf;
	int r = uname (&_buf);
	if (r == 0) {
		buf->_buf_ = _mph_copy_structure_strings (buf, mph_utsname_offsets, &_buf, utsname_offsets, sizeof(utsname_offsets)/sizeof(utsname_offsets[0]));
		buf->domainname = nullptr;
		if (buf->_buf_ == nullptr) {
			errno = ENOMEM;
			return -1;
		}
	}
	return r;
}

/*
 * vim: noexpandtab
 */
