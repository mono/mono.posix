/*
 * <unistd.h> wrapper functions.
 *
 * Authors:
 *   Jonathan Pryor (jonpryor@vt.edu)
 *
 * Copyright (C) 2004-2006 Jonathan Pryor
 */

#if defined (HAVE_CONFIG_H)
#include <config.h>
#endif

#include <limits>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <cerrno>
#include <climits>
#include <cstring>     /* for swab(3) on Mac OS X */

#if defined(_AIX)
#include <netdb.h> /* for get/setdomainname */
/*
 * Yet more stuff in libc that isn't in headers.
 * Python does the same thing we do here.
 * see: bugs.python.org/issue18259
 */
extern int sethostname(const char *, size_t);
extern int sethostid(long);
#endif

#include "mph.hh" /* Don't remove or move after map.h! Works around issues with Android SDK unified headers */
#include "map.hh"

mph_off_t
Mono_Posix_Syscall_lseek (int32_t fd, mph_off_t offset, int32_t whence)
{
	if (mph_have_off_t_overflow (offset)) {
		return -1;
	}

	return lseek (fd, offset, whence);
}

mph_ssize_t
Mono_Posix_Syscall_read (int32_t fd, void *buf, mph_size_t count)
{
	if (mph_have_size_t_overflow (count)) {
		return -1;
	};

	return read (fd, buf, count);
}

mph_ssize_t
Mono_Posix_Syscall_write (int32_t fd, void *buf, mph_size_t count)
{
	if (mph_have_size_t_overflow (count)) {
		return -1;
	};

	return write (fd, buf, count);
}

mph_ssize_t
Mono_Posix_Syscall_pread (int32_t fd, void *buf, mph_size_t count, mph_off_t offset)
{
	if (mph_have_size_t_overflow (count) || mph_have_off_t_overflow (offset)) {
		return -1;
	};

	return pread (fd, buf, count, offset);
}

mph_ssize_t
Mono_Posix_Syscall_pwrite (int32_t fd, void *buf, mph_size_t count, mph_off_t offset)
{
	if (mph_have_size_t_overflow (count) || mph_have_off_t_overflow (offset)) {
		return -1;
	};

	return pwrite (fd, buf, count, offset);
}

int32_t
Mono_Posix_Syscall_pipe (int32_t *reading, int32_t *writing)
{
	int filedes[2] = {-1, -1};
	int r;

	if (reading == nullptr || writing == nullptr) {
		errno = EFAULT;
		return -1;
	}

	r = pipe (filedes);

	*reading = filedes[0];
	*writing = filedes[1];
	return r;
}

void*
Mono_Posix_Syscall_getcwd (char *buf, mph_size_t size)
{
	if (mph_have_size_t_overflow (size)) {
		return nullptr;
	}

	return getcwd (buf, size);
}

int64_t
Mono_Posix_Syscall_fpathconf (int filedes, int name, int defaultError)
{
	errno = defaultError;
	if (Mono_Posix_FromPathconfName (name, &name) == -1)
		return -1;
	return fpathconf (filedes, name);
}

int64_t
Mono_Posix_Syscall_pathconf (const char *path, int name, int defaultError)
{
	errno = defaultError;
	if (Mono_Posix_FromPathconfName (name, &name) == -1)
		return -1;
	return pathconf (path, name);
}

int64_t
Mono_Posix_Syscall_sysconf (int name, int defaultError)
{
	errno = defaultError;
	if (Mono_Posix_FromSysconfName (name, &name) == -1)
		return -1;
	return sysconf (name);
}

#if HAVE_CONFSTR
mph_size_t
Mono_Posix_Syscall_confstr (int name, char *buf, mph_size_t len)
{
	if (mph_have_size_t_overflow (len)) {
		return 0;
	};

	if (Mono_Posix_FromConfstrName (name, &name) == -1)
		return 0;
	return confstr (name, buf, len);
}
#endif  /* def HAVE_CONFSTR */

#ifdef HAVE_TTYNAME_R
int32_t
Mono_Posix_Syscall_ttyname_r (int fd, char *buf, mph_size_t len)
{
	if (mph_have_size_t_overflow (len)) {
		return -1;
	};

	return ttyname_r (fd, buf, len);
}
#endif /* ndef HAVE_TTYNAME_R */

int64_t
Mono_Posix_Syscall_readlink (const char *path, unsigned char *buf, mph_size_t len)
{
	if (mph_have_size_t_overflow (len)) {
		return 0;
	};

	ssize_t r = readlink (path, reinterpret_cast<char*>(buf), len);
	if (r >= 0 && (size_t)r < len)
		buf [r] = '\0';
	return r;
}

#ifdef HAVE_READLINKAT
int64_t
Mono_Posix_Syscall_readlinkat (int dirfd, const char *path, unsigned char *buf, mph_size_t len)
{
	if (mph_have_size_t_overflow (len)) {
		return 0;
	};

	ssize_t r = readlinkat (dirfd, path, reinterpret_cast<char*>(buf), len);
	if (r >= 0 && (size_t)r < len)
		buf [r] = '\0';
	return r;
}
#endif /* def HAVE_READLINKAT */

#if HAVE_GETLOGIN_R
int32_t
Mono_Posix_Syscall_getlogin_r (char *buf, mph_size_t len)
{
	if (mph_have_size_t_overflow (len)) {
		return 0;
	};

	return getlogin_r (buf, len);
}
#endif  /* def HAVE_GETLOGIN_R */

int32_t
Mono_Posix_Syscall_gethostname (char *buf, mph_size_t len)
{
	if (mph_have_size_t_overflow (len)) {
		return 0;
	};

	return gethostname (buf, len);
}

#if HAVE_SETHOSTNAME
int32_t
Mono_Posix_Syscall_sethostname (const char *name, mph_size_t len)
{
	if (mph_have_size_t_overflow (len)) {
		return 0;
	};

	return sethostname (name, len);
}
#endif  /* def HAVE_SETHOSTNAME */

#if HAVE_GETHOSTID
int64_t
Mono_Posix_Syscall_gethostid ()
{
	return gethostid ();
}
#endif  /* def HAVE_GETHOSTID */

#ifdef HAVE_SETHOSTID
int32_t
Mono_Posix_Syscall_sethostid (int64_t hostid)
{
	if (mph_have_long_overflow (hostid)) {
		return -1;
	}

#ifdef MPH_ON_BSD
	sethostid ((long) hostid);
	return 0;
#else
	return sethostid (static_cast<long>(hostid));
#endif
}
#endif /* def HAVE_SETHOSTID */

#ifdef HAVE_GETDOMAINNAME
int32_t
Mono_Posix_Syscall_getdomainname (char *name, mph_size_t len)
{
	if (mph_have_size_t_overflow (len)) {
		return 0;
	};

	return getdomainname (name, len);
}
#endif /* def HAVE_GETDOMAINNAME */

#ifdef HAVE_SETDOMAINNAME
int32_t
Mono_Posix_Syscall_setdomainname (const char *name, mph_size_t len)
{
	if (mph_have_size_t_overflow (len)) {
		return 0;
	};

	return setdomainname (name, len);
}
#endif /* def HAVE_SETDOMAINNAME */

/* Android implements truncate, but doesn't declare it.
 * Result is a warning during compilation, so skip it.
 */
#ifndef HOST_ANDROID
int32_t
Mono_Posix_Syscall_truncate (const char *path, mph_off_t length)
{
	if (mph_have_off_t_overflow (length)) {
		return -1;
	}

	return truncate (path, length);
}
#endif

int32_t
Mono_Posix_Syscall_ftruncate (int fd, mph_off_t length)
{
	if (mph_have_off_t_overflow (length)) {
		return -1;
	}

	return ftruncate (fd, length);
}

#if HAVE_LOCKF
int32_t
Mono_Posix_Syscall_lockf (int fd, int cmd, mph_off_t len)
{
	if (mph_have_off_t_overflow (len)) {
		return -1;
	}

	if (Mono_Posix_FromLockfCommand (cmd, &cmd) == -1)
		return -1;
	return lockf (fd, cmd, len);
}
#endif  /* def HAVE_LOCKF */

#if HAVE_SWAB
int
Mono_Posix_Syscall_swab (void *from, void *to, mph_ssize_t n)
{
	if (mph_have_long_overflow (n))
		return -1;
	swab (from, to, n);
	return 0;
}
#endif  /* def HAVE_SWAB */

#if HAVE_SETUSERSHELL
int
Mono_Posix_Syscall_setusershell ()
{
	setusershell ();
	return 0;
}
#endif  /* def HAVE_SETUSERSHELL */

#if HAVE_ENDUSERSHELL
int
Mono_Posix_Syscall_endusershell ()
{
	endusershell ();
	return 0;
}
#endif  /* def HAVE_ENDUSERSHELL */

int
Mono_Posix_Syscall_sync ()
{
	sync ();
	return 0;
}

/*
 * vim: noexpandtab
 */
