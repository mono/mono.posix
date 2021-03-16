#if !defined (__MPH_COMPAT_IOS_HH)
#define __MPH_COMPAT_IOS_HH

#if defined (HOST_IOS)
#include <sys/stat.h>
#include <sys/uio.h>

int __ios_futimens (int __fd, const struct timespec __times[2]);
int __ios_utimensat (int __fd, const char *__path, const struct timespec __times[2], int __flag);
ssize_t __ios_preadv (int d, const struct iovec *iov, int iovcnt, off_t offset);
ssize_t __ios_pwritev (int fildes, const struct iovec *iov, int iovcnt, off_t offset);

#endif // HOST_IOS

#endif // __MPH_COMPAT_IOS_HH
