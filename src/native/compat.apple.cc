#if defined (HAVE_CONFIG_H)
#include <config.h>
#endif // HAVE_CONFIG_H

#include <cerrno>

#include "compat.apple.hh"

extern "C" {
	// Available since iOS 11.0
	// Available since tvOS 11.0
	int futimens(int __fd, const struct timespec __times[2]) __attribute__((weak_import));

	// Available since iOS 11.0
	// Available since tvOS 11.0
	int utimensat(int __fd, const char *__path, const struct timespec __times[2], int __flag) __attribute__((weak_import));

	// Available since iOS 14.0
	// Available since tvOS 14.0
	ssize_t preadv(int d, const struct iovec *iov, int iovcnt, off_t offset) __attribute__((weak_import));

	// Available since iOS 14.0
	// Available since vtOS 14.0
	ssize_t pwritev (int fildes, const struct iovec *iov, int iovcnt, off_t offset) __attribute__((weak_import));
}

//
// We cannot use __builtin_available because it works only in shared libraries
//
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunguarded-availability-new"

int __apple_futimens(int __fd, const struct timespec __times[2])
{
	if (futimens == nullptr) {
		errno = ENOSYS;
		return -1;
	}

	return futimens (__fd, __times);
}

int __apple_utimensat(int __fd, const char *__path, const struct timespec __times[2], int __flag)
{
	if (utimensat == nullptr) {
		errno = ENOSYS;
		return -1;
	}

	return utimensat (__fd, __path, __times, __flag);
}

ssize_t __apple_preadv(int d, const struct iovec *iov, int iovcnt, off_t offset)
{
	if (preadv == nullptr) {
		errno = ENOSYS;
		return -1;
	}

	return preadv (d, iov, iovcnt, offset);
}

ssize_t __apple_pwritev (int fildes, const struct iovec *iov, int iovcnt, off_t offset)
{
	if (pwritev == nullptr) {
		errno = ENOSYS;
		return -1;
	}

	return pwritev (fildes, iov, iovcnt, offset);
}
#pragma clang diagnostic pop
