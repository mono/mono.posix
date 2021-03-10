/*
 * <time.h> wrapper functions.
 *
 * Authors:
 *   Jonathan Pryor (jonpryor@vt.edu)
 *
 * Copyright (C) 2004 Jonathan Pryor
 */

#if defined (HAVE_CONFIG_H)
#include <config.h>
#endif

#include <time.h>
#include <errno.h>

#include "map.hh"
#include "mph.hh"
#include <glib.h>

#if defined(HAVE_STRUCT_TIMESPEC) && _POSIX_C_SOURCE >= 199309L
int
Mono_Posix_Syscall_nanosleep (struct Mono_Posix_Timespec *req,
		struct Mono_Posix_Timespec *rem)
{
	struct timespec _req, _rem, *prem = nullptr;
	int r;

	if (req == nullptr) {
		errno = EFAULT;
		return -1;
	}

	if (Mono_Posix_FromTimespec (req, &_req) == -1)
		return -1;

	if (rem) {
		if (Mono_Posix_FromTimespec (rem, &_rem) == -1)
			return -1;
		prem = &_rem;
	}

	r = nanosleep (&_req, prem);

	if (rem && Mono_Posix_ToTimespec (prem, rem) == -1)
		return -1;

	return r;
}
#endif

#ifdef HAVE_STIME
/* AIX has stime in libc, but not at all in headers, so declare here */
#if defined(_AIX)
extern int stime(time_t);
#endif

gint32
Mono_Posix_Syscall_stime (mph_time_t *t)
{
	time_t _t;
	if (t == nullptr) {
		errno = EFAULT;
		return -1;
	}
	mph_return_if_time_t_overflow (*t);
	_t = (time_t) *t;
	return stime (&_t);
}
#endif /* ndef HAVE_STIME */

mph_time_t
Mono_Posix_Syscall_time (mph_time_t *t)
{
	time_t _t, r;
	if (t == nullptr) {
		errno = EFAULT;
		return -1;
	}

	mph_return_if_time_t_overflow (*t);

	_t = (time_t) *t;
	r = time (&_t);
	*t = _t;

	return r;
}

/*
 * vim: noexpandtab
 */