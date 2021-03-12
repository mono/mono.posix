/*
 * <sys/sendfile.h> wrapper functions.
 *
 * Authors:
 *   Jonathan Pryor (jonpryor@vt.edu)
 *
 * Copyright (C) 2004 Jonathan Pryor
 */

#if defined (HAVE_CONFIG_H)
#include <config.h>
#endif

#include <sys/types.h>
#include <cerrno>

#include "map.hh"
#include "mph.hh"

#ifdef HAVE_SYS_SENDFILE_H
#include <sys/sendfile.h>
#endif /* ndef HAVE_SYS_SENDFILE_H */

#ifdef HAVE_SENDFILE
mph_ssize_t
Mono_Posix_Syscall_sendfile (int out_fd, int in_fd, mph_off_t *offset, mph_size_t count)
{
	if (offset == nullptr) {
		errno = EINVAL;
		return -1;
	}

	if (mph_have_off_t_overflow (*offset)) {
		return -1;
	}

	ssize_t r;
	off_t _offset = *offset;

#if defined(HOST_DARWIN) || defined(HOST_BSD)
	/* The BSD version has 6 arguments */
	g_assert_not_reached ();
#else
	r = sendfile (out_fd, in_fd, &_offset, (size_t) count);
#endif

	*offset = _offset;

	return r;
}
#endif /* ndef HAVE_SENDFILE */

/*
 * vim: noexpandtab
 */
