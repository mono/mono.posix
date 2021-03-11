#ifndef SYS_UIO_H
#define SYS_UIO_H

#include <sys/uio.h>

#include "map.hh"
#include "mph.hh"

struct iovec*
_mph_from_iovec_array (struct Mono_Posix_Iovec *iov, int32_t iovcnt);

#endif /* SYS_UIO_H */
