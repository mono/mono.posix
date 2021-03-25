/*
 * <grp.h> wrapper functions.
 *
 * Authors:
 *   Jonathan Pryor (jonpryor@vt.edu)
 *
 * Copyright (C) 2004-2005 Jonathan Pryor
 */

#if defined (HAVE_CONFIG_H)
#include <config.h>
#endif

#include <sys/types.h>
#ifdef HAVE_SYS_PARAM_H
#include <sys/param.h>
#endif

#include <cstdio>
#include <cstdlib>
#include <cerrno>
#include <cstring>
#include <unistd.h>	/* for setgroups on Mac OS X */

#include "map.hh"
#include "mph.hh"

using int_limits = std::numeric_limits<int>;

static inline void
count_members (char **gr_mem, int *count, size_t *mem)
{
	*count = 0;

	// ensure that later (*mem)+1 doesn't result in integer overflow
	if (*mem > int_limits::max () - 1)
		return;

	for (char *cur = *gr_mem; cur != nullptr; cur = *++gr_mem) {
		size_t len;
		len = strlen (cur);

		if (!(len < int_limits::max () - ((*mem) + 1)))
			break;

		++(*count);
		*mem += (len + 1);
	}
}

static inline bool
copy_group (struct Mono_Posix_Syscall__Group *to, struct group *from)
{
	to->gr_gid    = from->gr_gid;
	to->gr_name   = nullptr;
	to->gr_passwd = nullptr;
	to->gr_mem    = nullptr;
	to->_gr_buf_  = nullptr;

	size_t nlen = strlen (from->gr_name);
	size_t buflen = 2;

	if (!(nlen < int_limits::max () - buflen)) {
		return false;
	}
	buflen += nlen;

	size_t plen = strlen (from->gr_passwd);
	if (!(plen < int_limits::max () - buflen)) {
		return false;
	}
	buflen += plen;

	int count = 0;
	count_members (from->gr_mem, &count, &buflen);

	to->_gr_nmem_ = count;
	char *cur = to->_gr_buf_ = static_cast<char*>(malloc (buflen));
	char **to_mem = to->gr_mem = static_cast<char**>(malloc (sizeof(char*) * (count+1)));
	if (to->_gr_buf_ == nullptr || to->gr_mem == nullptr) {
		free (to->_gr_buf_);
		free (to->gr_mem);
		return false;
	}

	to->gr_name = strcpy (cur, from->gr_name);
	cur += (nlen + 1);
	to->gr_passwd = strcpy (cur, from->gr_passwd);
	cur += (plen + 1);

	int i = 0;
	for (; i != count; ++i) {
		to_mem [i] = strcpy (cur, from->gr_mem[i]);
		cur += (strlen (from->gr_mem[i])+1);
	}
	to_mem [i] = nullptr;

	return true;
}

int32_t
Mono_Posix_Syscall_getgrnam (const char *name, struct Mono_Posix_Syscall__Group *gbuf)
{
	if (gbuf == nullptr) {
		errno = EFAULT;
		return -1;
	}

	errno = 0;
	struct group *_gbuf = getgrnam (name);
	if (_gbuf == nullptr) {
		return -1;
	}

	if (!copy_group (gbuf, _gbuf)) {
		errno = ENOMEM;
		return -1;
	}
	return 0;
}

int32_t
Mono_Posix_Syscall_getgrgid (mph_gid_t gid, struct Mono_Posix_Syscall__Group *gbuf)
{
	if (gbuf == nullptr) {
		errno = EFAULT;
		return -1;
	}

	errno = 0;
	struct group *_gbuf = getgrgid (gid);
	if (_gbuf == nullptr) {
		return -1;
	}

	if (!copy_group (gbuf, _gbuf)) {
		errno = ENOMEM;
		return -1;
	}
	return 0;
}

#ifdef HAVE_GETGRNAM_R
int32_t
Mono_Posix_Syscall_getgrnam_r (const char *name, struct Mono_Posix_Syscall__Group *gbuf, struct group** gbufp)
{
	if (gbuf == nullptr) {
		errno = EFAULT;
		return -1;
	}

	char *buf = nullptr;
	size_t buflen = 2;
	int r;
	struct group _grbuf;

	do {
		char *buf2 = static_cast<char*>(realloc (buf, buflen *= 2));
		if (buf2 == nullptr) {
			free (buf);
			errno = ENOMEM;
			return -1;
		}
		buf = buf2;
		errno = 0;
	} while ((r = getgrnam_r (name, &_grbuf, buf, buflen, gbufp)) && recheck_range (r));

	/* On Solaris, this function returns 0 even if the entry was not found */
	if (r == 0 && !(*gbufp))
		r = errno = ENOENT;

	if (r == 0 && !copy_group (gbuf, &_grbuf)) {
		r = errno = ENOMEM;
	}
	free (buf);

	return r;
}
#endif /* ndef HAVE_GETGRNAM_R */

#ifdef HAVE_GETGRGID_R
int32_t
Mono_Posix_Syscall_getgrgid_r (mph_gid_t gid, struct Mono_Posix_Syscall__Group *gbuf, struct group** gbufp)
{
	if (gbuf == nullptr) {
		errno = EFAULT;
		return -1;
	}

	char *buf = nullptr;
	size_t buflen = 2;
	int r;
	struct group _grbuf;

	do {
		char *buf2 = static_cast<char*>(realloc (buf, buflen *= 2));
		if (buf2 == nullptr) {
			free (buf);
			errno = ENOMEM;
			return -1;
		}
		buf = buf2;
		errno = 0;
	} while ((r = getgrgid_r (gid, &_grbuf, buf, buflen, gbufp)) && recheck_range (r));

	/* On Solaris, this function returns 0 even if the entry was not found */
	if (r == 0 && !(*gbufp))
		r = errno = ENOENT;

	if (r == 0 && !copy_group (gbuf, &_grbuf))
		r = errno = ENOMEM;
	free (buf);

	return r;
}
#endif /* ndef HAVE_GETGRGID_R */

#if HAVE_GETGRENT
int32_t
Mono_Posix_Syscall_getgrent (struct Mono_Posix_Syscall__Group *grbuf)
{
	if (grbuf == nullptr) {
		errno = EFAULT;
		return -1;
	}

	errno = 0;
	struct group *gr = getgrent ();
	if (gr == nullptr)
		return -1;

	if (!copy_group (grbuf, gr)) {
		errno = ENOMEM;
		return -1;
	}
	return 0;
}
#endif  /* def HAVE_GETGRENT */

#ifdef HAVE_FGETGRENT
int32_t
Mono_Posix_Syscall_fgetgrent (void *stream, struct Mono_Posix_Syscall__Group *grbuf)
{
	if (grbuf == nullptr) {
		errno = EFAULT;
		return -1;
	}

	errno = 0;
	struct group *gr = fgetgrent ((FILE*) stream);
	if (gr == nullptr)
		return -1;

	if (!copy_group (grbuf, gr)) {
		errno = ENOMEM;
		return -1;
	}
	return 0;
}
#endif /* ndef HAVE_FGETGRENT */

#if HAVE_SETGROUPS
int32_t
Mono_Posix_Syscall_setgroups (mph_size_t size, mph_gid_t *list)
{
	if (mph_have_size_t_overflow (size)) {
		return -1;
	}

	return setgroups ((size_t) size, list);
}
#endif  /* def HAVE_SETGROUPS */

#if HAVE_SETGRENT
int
Mono_Posix_Syscall_setgrent ()
{
	errno = 0;
	do {
		setgrent ();
	} while (errno == EINTR);

	if (mph_value_in_list (errno, EIO, EMFILE, ENFILE, ENOMEM, ERANGE)) {
		return -1;
	}

	return 0;
}
#endif  /* def HAVE_SETGRENT */

#if HAVE_ENDGRENT
int
Mono_Posix_Syscall_endgrent ()
{
	endgrent();
	return 0;
}
#endif  /* def HAVE_ENDGRENT */

/*
 * vim: noexpandtab
 */
