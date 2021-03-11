/*
 * <pwd.h> wrapper functions.
 *
 * Authors:
 *   Jonathan Pryor (jonpryor@vt.edu)
 *
 * Copyright (C) 2004-2005 Jonathan Pryor
 */

#if defined (HAVE_CONFIG_H)
#include <config.h>
#endif

#include <pwd.h>
#include <cerrno>
#include <cstring>
#include <cstdio>
#include <cstdlib>

#include "mph.hh" /* Don't remove or move after map.h! Works around issues with Android SDK unified headers */
#include "map.hh"

static const mph_string_offset_t
passwd_offsets[] = {
	MPH_STRING_OFFSET (struct passwd, pw_name,    MPH_STRING_OFFSET_PTR),
	MPH_STRING_OFFSET (struct passwd, pw_passwd,  MPH_STRING_OFFSET_PTR),
#if HAVE_STRUCT_PASSWD_PW_GECOS
	MPH_STRING_OFFSET (struct passwd, pw_gecos,   MPH_STRING_OFFSET_PTR),
#endif  /* def HAVE_STRUCT_PASSWD_PW_GECOS */
	MPH_STRING_OFFSET (struct passwd, pw_dir,     MPH_STRING_OFFSET_PTR),
	MPH_STRING_OFFSET (struct passwd, pw_shell,   MPH_STRING_OFFSET_PTR)
};

static const mph_string_offset_t
mph_passwd_offsets[] = {
	MPH_STRING_OFFSET (struct Mono_Posix_Syscall__Passwd, pw_name,    MPH_STRING_OFFSET_PTR),
	MPH_STRING_OFFSET (struct Mono_Posix_Syscall__Passwd, pw_passwd,  MPH_STRING_OFFSET_PTR),
#if HAVE_STRUCT_PASSWD_PW_GECOS
	MPH_STRING_OFFSET (struct Mono_Posix_Syscall__Passwd, pw_gecos,   MPH_STRING_OFFSET_PTR),
#endif  /* def HAVE_STRUCT_PASSWD_PW_GECOS */
	MPH_STRING_OFFSET (struct Mono_Posix_Syscall__Passwd, pw_dir,     MPH_STRING_OFFSET_PTR),
	MPH_STRING_OFFSET (struct Mono_Posix_Syscall__Passwd, pw_shell,   MPH_STRING_OFFSET_PTR)
};

/*
 * Copy the native `passwd' structure to it's managed representation.
 *
 * To minimize separate mallocs, all the strings are allocated within the same
 * memory block (stored in _pw_buf_).
 */
static inline bool
copy_passwd (struct Mono_Posix_Syscall__Passwd *to, struct passwd *from)
{
	char *buf = _mph_copy_structure_strings (to, mph_passwd_offsets, from, passwd_offsets, sizeof(passwd_offsets)/sizeof(passwd_offsets[0]));

	to->pw_uid    = from->pw_uid;
	to->pw_gid    = from->pw_gid;

	to->_pw_buf_ = buf;
	if (buf == nullptr) {
		return false;
	}

	return true;
}

int32_t
Mono_Posix_Syscall_getpwnam (const char *name, struct Mono_Posix_Syscall__Passwd *pwbuf)
{
	if (pwbuf == nullptr || name == nullptr) {
		errno = EFAULT;
		return -1;
	}

	errno = 0;
	struct passwd *pw = getpwnam (name);
	if (pw == nullptr) {
		return -1;
	}

	if (!copy_passwd (pwbuf, pw)) {
		errno = ENOMEM;
		return -1;
	}
	return 0;
}

int32_t
Mono_Posix_Syscall_getpwuid (mph_uid_t uid, struct Mono_Posix_Syscall__Passwd *pwbuf)
{
	if (pwbuf == nullptr) {
		errno = EFAULT;
		return -1;
	}

	errno = 0;
	struct passwd *pw = getpwuid (uid);
	if (pw == nullptr) {
		return -1;
	}

	if (!copy_passwd (pwbuf, pw)) {
		errno = ENOMEM;
		return -1;
	}
	return 0;
}

#ifdef HAVE_GETPWNAM_R
int32_t
Mono_Posix_Syscall_getpwnam_r (const char *name, struct Mono_Posix_Syscall__Passwd *pwbuf, struct passwd** pwbufp)
{
	if (pwbuf == nullptr || name == nullptr) {
		errno = EFAULT;
		return -1;
	}

	char *buf = nullptr;
	size_t buflen = 2;
	int r;
	struct passwd _pwbuf;

	do {
		char *buf2 = static_cast<char*>(realloc (buf, buflen *= 2));
		if (buf2 == nullptr) {
			free (buf);
			errno = ENOMEM;
			return -1;
		}
		buf = buf2;
		errno = 0;
	} while ((r = getpwnam_r (name, &_pwbuf, buf, buflen, pwbufp)) && recheck_range (r));

	if (r == 0 && !(*pwbufp)) {
		/* On solaris, this function returns 0 even if the entry was not found */
		r = errno = ENOENT;
	}

	if (r == 0 && !copy_passwd (pwbuf, &_pwbuf)) {
		r = errno = ENOMEM;
	}

	free (buf);

	return r;
}
#endif /* ndef HAVE_GETPWNAM_R */

#ifdef HAVE_GETPWUID_R
int32_t
Mono_Posix_Syscall_getpwuid_r (mph_uid_t uid, struct Mono_Posix_Syscall__Passwd *pwbuf,	struct passwd** pwbufp)
{
	if (pwbuf == nullptr) {
		errno = EFAULT;
		return -1;
	}

	char *buf = nullptr;
	size_t buflen = 2;
	int r;
	struct passwd _pwbuf;

	do {
		char *buf2 = static_cast<char*>(realloc (buf, buflen *= 2));
		if (buf2 == nullptr) {
			free (buf);
			errno = ENOMEM;
			return -1;
		}
		buf = buf2;
		errno = 0;
	} while ((r = getpwuid_r (uid, &_pwbuf, buf, buflen, pwbufp)) &&
			recheck_range (r));

	if (r == 0 && !(*pwbufp)) {
		/* On solaris, this function returns 0 even if the entry was not found */
		r = errno = ENOENT;
	}

	if (r == 0 && !copy_passwd (pwbuf, &_pwbuf)) {
		r = errno = ENOMEM;
	}

	free (buf);

	return r;
}
#endif /* ndef HAVE_GETPWUID_R */

#if HAVE_GETPWENT
int32_t
Mono_Posix_Syscall_getpwent (struct Mono_Posix_Syscall__Passwd *pwbuf)
{
	if (pwbuf == nullptr) {
		errno = EFAULT;
		return -1;
	}

	errno = 0;
	struct passwd *pw = getpwent ();
	if (pw == nullptr)
		return -1;

	if (!copy_passwd (pwbuf, pw)) {
		errno = ENOMEM;
		return -1;
	}
	return 0;
}
#endif  /* def HAVE_GETPWENT */

#ifdef HAVE_FGETPWENT
int32_t
Mono_Posix_Syscall_fgetpwent (FILE* stream, struct Mono_Posix_Syscall__Passwd *pwbuf)
{
	if (pwbuf == nullptr) {
		errno = EFAULT;
		return -1;
	}

	errno = 0;
	struct passwd *pw = fgetpwent (stream);
	if (pw == nullptr)
		return -1;

	if (!copy_passwd (pwbuf, pw)) {
		errno = ENOMEM;
		return -1;
	}
	return 0;
}
#endif /* ndef HAVE_FGETPWENT */

#if HAVE_SETPWENT
int
Mono_Posix_Syscall_setpwent ()
{
	errno = 0;
	do {
		setpwent ();
	} while (errno == EINTR);

	if (mph_value_in_list (errno, EIO, EMFILE, ENFILE, ENOMEM, ERANGE)) {
		return -1;
	}

	return 0;
}
#endif  /* def HAVE_SETPWENT */

#if HAVE_ENDPWENT
int
Mono_Posix_Syscall_endpwent ()
{
	errno = 0;
	endpwent ();
	if (errno == EIO)
		return -1;
	return 0;
}
#endif  /* def HAVE_ENDPWENT */

/*
 * vim: noexpandtab
 */
