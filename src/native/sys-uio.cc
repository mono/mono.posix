/*
 * <sys/uio.h> wrapper functions.
 *
 * Authors:
 *   Steffen Kiess (s-kiess@web.de)
 *
 * Copyright (C) 2012 Steffen Kiess
 */

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif /* ndef _GNU_SOURCE */

#include "sys-uio.hh"

#include <sys/uio.h>
#include <stdlib.h>

#include "map.hh"
#include "mph.hh"

struct iovec*
_mph_from_iovec_array (struct Mono_Posix_Iovec *iov, int32_t iovcnt)
{
	int32_t i;

	if (iovcnt < 0) {
		errno = EINVAL;
		return nullptr;
	}

	auto v = static_cast<struct iovec*>(malloc (iovcnt * sizeof (struct iovec)));
	if (!v) {
		return nullptr;
	}

	for (i = 0; i < iovcnt; i++) {
		if (Mono_Posix_FromIovec (&iov[i], &v[i]) != 0) {
			free (v);
			return nullptr;
		}
	}

	return v;
}

#ifdef HAVE_READV
int64_t
Mono_Posix_Syscall_readv (int dirfd, struct Mono_Posix_Iovec *iov, int32_t iovcnt)
{
	struct iovec* v;
	int64_t res;

	v = _mph_from_iovec_array (iov, iovcnt);
	if (!v) {
		return -1;
	}

	res = readv(dirfd, v, iovcnt);
	free (v);
	return res;
}
#endif /* def HAVE_READV */

#ifdef HAVE_WRITEV
int64_t
Mono_Posix_Syscall_writev (int dirfd, struct Mono_Posix_Iovec *iov, int32_t iovcnt)
{
	struct iovec* v;
	int64_t res;

	v = _mph_from_iovec_array (iov, iovcnt);
	if (!v) {
		return -1;
	}

	res = writev (dirfd, v, iovcnt);
	free (v);
	return res;
}
#endif /* def HAVE_WRITEV */

#ifdef HAVE_PREADV
int64_t
Mono_Posix_Syscall_preadv (int dirfd, struct Mono_Posix_Iovec *iov, int32_t iovcnt, int64_t off)
{
	struct iovec* v;
	int64_t res;

	mph_return_if_off_t_overflow (off);

	v = _mph_from_iovec_array (iov, iovcnt);
	if (!v) {
		return -1;
	}

	res = preadv (dirfd, v, iovcnt, (off_t) off);
	free (v);
	return res;
}
#endif /* def HAVE_PREADV */

#ifdef HAVE_PWRITEV
int64_t
Mono_Posix_Syscall_pwritev (int dirfd, struct Mono_Posix_Iovec *iov, int32_t iovcnt, int64_t off)
{
	struct iovec* v;
	int64_t res;

	mph_return_if_off_t_overflow (off);

	v = _mph_from_iovec_array (iov, iovcnt);
	if (!v) {
		return -1;
	}

	res = pwritev (dirfd, v, iovcnt, (off_t) off);
	free (v);
	return res;
}
#endif /* def HAVE_PWRITEV */

/*
 * vim: noexpandtab
 */
