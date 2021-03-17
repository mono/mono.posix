/*
 * <sys/uio.h> wrapper functions.
 *
 * Authors:
 *   Steffen Kiess (s-kiess@web.de)
 *
 * Copyright (C) 2012 Steffen Kiess
 */

#if defined (HAVE_CONFIG_H)
#include <config.h>
#endif

#include "sys-uio.hh"

#include <sys/uio.h>
#include <cstdlib>

#include "map.hh"
#include "mph.hh"

#if defined(HOST_IOS) || defined(HOST_TVOS)
#include "compat.apple.hh"
#endif // def HOST_IOS || def HOST_TVOS

MPH_API_INTERNAL struct iovec*
_mph_from_iovec_array (struct Mono_Posix_Iovec *iov, int32_t iovcnt)
{
	if (iov == nullptr) {
		errno = EFAULT;
		return nullptr;
	}

	if (iovcnt < 0) {
		errno = EINVAL;
		return nullptr;
	}

	auto v = static_cast<struct iovec*>(malloc (iovcnt * sizeof (struct iovec)));
	if (!v) {
		return nullptr;
	}

	for (int32_t i = 0; i < iovcnt; i++) {
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
	struct iovec *v = _mph_from_iovec_array (iov, iovcnt);
	if (v == nullptr) {
		return -1;
	}

	int64_t res = readv (dirfd, v, iovcnt);
	free (v);
	return res;
}
#endif /* def HAVE_READV */

#ifdef HAVE_WRITEV
int64_t
Mono_Posix_Syscall_writev (int dirfd, struct Mono_Posix_Iovec *iov, int32_t iovcnt)
{
	struct iovec *v = _mph_from_iovec_array (iov, iovcnt);
	if (v == nullptr) {
		return -1;
	}

	int64_t res = writev (dirfd, v, iovcnt);
	free (v);
	return res;
}
#endif /* def HAVE_WRITEV */

#ifdef HAVE_PREADV
int64_t
Mono_Posix_Syscall_preadv (int dirfd, struct Mono_Posix_Iovec *iov, int32_t iovcnt, int64_t off)
{
	if (mph_have_off_t_overflow (off)) {
		return -1;
	}

	struct iovec *v = _mph_from_iovec_array (iov, iovcnt);
	if (v == nullptr) {
		return -1;
	}

	int64_t res;
#if !defined(HOST_IOS) && !defined(HOST_TVOS)
	res = preadv (dirfd, v, iovcnt, off);
#else // ndef HOST_IOS && ndef HOST_TVOS
	// Available since iOS 11.0
	// Available since tvOS 11.0
	res = __apple_preadv (dirfd, v, iovcnt, off);
#endif // def HOST_IOS && def HOST_TVOS
	free (v);
	return res;
}
#endif /* def HAVE_PREADV */

#ifdef HAVE_PWRITEV
int64_t
Mono_Posix_Syscall_pwritev (int dirfd, struct Mono_Posix_Iovec *iov, int32_t iovcnt, int64_t off)
{
	if (mph_have_off_t_overflow (off)) {
		return -1;
	}

	struct iovec *v = _mph_from_iovec_array (iov, iovcnt);
	if (v == nullptr) {
		return -1;
	}

	int64_t res;
#if !defined(HOST_IOS) && !defined(HOST_TVOS)
	res = pwritev (dirfd, v, iovcnt, off);
#else // ndef HOST_IOS && ndef HOST_TVOS
	// Available since iOS 11.0
	// Available since tvOS 11.0
	res = __apple_preadv (dirfd, v, iovcnt, off);
#endif // def HOST_IOS && def HOST_TVOS
	free (v);
	return res;
}
#endif /* def HAVE_PWRITEV */

/*
 * vim: noexpandtab
 */
