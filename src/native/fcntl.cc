/*
 * <fcntl.h> wrapper functions.
 *
 * Authors:
 *   Jonathan Pryor (jonpryor@vt.edu)
 *
 * Copyright (C) 2004, 2006 Jonathan Pryor
 */

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <sys/types.h>
#include <sys/stat.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#include <fcntl.h>
#include <errno.h>
#ifdef HOST_WIN32
#include <corecrt_io.h>
#endif

#include "mph.hh" /* Don't remove or move after map.h! Works around issues with Android SDK unified headers */
#include "map.hh"

#ifndef HOST_WIN32
int32_t
Mono_Posix_Syscall_fcntl (int32_t fd, int32_t cmd)
{
	if (Mono_Posix_FromFcntlCommand (cmd, &cmd) == -1)
		return -1;
	return fcntl (fd, cmd);
}

int32_t
Mono_Posix_Syscall_fcntl_arg_int (int32_t fd, int32_t cmd, int arg)
{
	if (Mono_Posix_FromFcntlCommand (cmd, &cmd) == -1)
		return -1;
	return fcntl (fd, cmd, arg);
}

int32_t
Mono_Posix_Syscall_fcntl_arg_ptr (int32_t fd, int32_t cmd, void *arg)
{
	if (Mono_Posix_FromFcntlCommand (cmd, &cmd) == -1)
		return -1;
	return fcntl (fd, cmd, arg);
}

int32_t
Mono_Posix_Syscall_fcntl_arg (int32_t fd, int32_t cmd, int64_t arg)
{
	long _arg;
	int32_t _cmd;

	if (mph_have_long_overflow (arg)) {
		return -1;
	}

#ifdef F_NOTIFY
	if (cmd == F_NOTIFY) {
		int _argi;
		if (Mono_Posix_FromDirectoryNotifyFlags (arg, &_argi) == -1) {
			return -1;
		}
		_arg = _argi;
	}
	else
#endif
		_arg = (long) arg;

	if (Mono_Posix_FromFcntlCommand (cmd, &_cmd) == -1)
		return -1;
	return fcntl (fd, cmd, _arg);
}

int32_t
Mono_Posix_Syscall_fcntl_lock (int32_t fd, int32_t cmd, struct Mono_Posix_Flock *lock)
{
	struct flock _lock;
	int r;

	if (lock == nullptr) {
		errno = EFAULT;
		return -1;
	}

	if (Mono_Posix_FromFlock (lock, &_lock) == -1)
		return -1;

	if (Mono_Posix_FromFcntlCommand (cmd, &cmd) == -1)
		return -1;

	r = fcntl (fd, cmd, &_lock);

	if (Mono_Posix_ToFlock (&_lock, lock) == -1)
		return -1;

	return r;
}
#endif

int32_t
Mono_Posix_Syscall_open (const char *pathname, int32_t flags)
{
	if (Mono_Posix_FromOpenFlags (flags, &flags) == -1)
		return -1;

	return open (pathname, flags);
}

int32_t
Mono_Posix_Syscall_open_mode (const char *pathname, int32_t flags, uint32_t mode)
{
	if (Mono_Posix_FromOpenFlags (flags, &flags) == -1)
		return -1;
	if (Mono_Posix_FromFilePermissions (mode, &mode) == -1)
		return -1;

	return open (pathname, flags, mode);
}

int32_t
Mono_Posix_Syscall_get_at_fdcwd ()
{
#ifdef AT_FDCWD
	return AT_FDCWD;
#else
	return -1;
#endif
}

int32_t
Mono_Posix_Syscall_creat (const char *pathname, uint32_t mode)
{
	if (Mono_Posix_FromFilePermissions (mode, &mode) == -1)
		return -1;

	return creat (pathname, mode);
}

#ifdef HAVE_POSIX_FADVISE
int32_t
Mono_Posix_Syscall_posix_fadvise (int32_t fd, mph_off_t offset, mph_off_t len,
	int32_t advice)
{
	if (mph_have_off_t_overflow (offset) || mph_have_off_t_overflow (len)) {
		return -1;
	}

	if (Mono_Posix_FromPosixFadviseAdvice (advice, &advice) == -1)
		return -1;

	return posix_fadvise (fd, (off_t) offset, (off_t) len, advice);
}
#endif /* ndef HAVE_POSIX_FADVISE */

#ifdef HAVE_POSIX_FALLOCATE
int32_t
Mono_Posix_Syscall_posix_fallocate (int32_t fd, mph_off_t offset, mph_size_t len)
{
	if (mph_have_size_t_overflow (len) || mph_have_off_t_overflow (offset)) {
		return -1;
	}

	return posix_fallocate (fd, (off_t) offset, (size_t) len);
}
#endif /* ndef HAVE_POSIX_FALLOCATE */

/*
 * vim: noexpandtab
 */
