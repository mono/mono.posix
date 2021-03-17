#if !defined (__MPH_COMPAT_IOS_HH)
#define __MPH_COMPAT_IOS_HH

#if defined(HOST_IOS) || defined(HOST_TVOS) || defined(HOST_CATALYST)
#include <sys/stat.h>
#include <sys/uio.h>

int __apple_futimens (int __fd, const struct timespec __times[2]);
int __apple_utimensat (int __fd, const char *__path, const struct timespec __times[2], int __flag);
ssize_t __apple_preadv (int d, const struct iovec *iov, int iovcnt, off_t offset);
ssize_t __apple_pwritev (int fildes, const struct iovec *iov, int iovcnt, off_t offset);

#endif // def HOST_IOS || def HOST_TVOS || def HOST_CATALYST

#endif // ndef __MPH_COMPAT_IOS_HH
