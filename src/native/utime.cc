/*
 * <stdio.h> wrapper functions.
 *
 * Authors:
 *   Jonathan Pryor (jonpryor@vt.edu)
 *
 * Copyright (C) 2004-2006 Jonathan Pryor
 */

#if defined (HAVE_CONFIG_H)
#include <config.h>
#endif

#include <sys/types.h>
#include <utime.h>

#include "map.hh"
#include "mph.hh"

int32_t
Mono_Posix_Syscall_utime (const char* filename, struct Mono_Posix_Utimbuf* buf, int use_buf)
{
	struct utimbuf _buf;
	struct utimbuf *pbuf = nullptr;

	if (buf != nullptr && use_buf) {
		_buf.actime  = buf->actime;
		_buf.modtime = buf->modtime;
		pbuf = &_buf;
	}

	return utime (filename, pbuf);
}

/*
 * vim: noexpandtab
 */
