/*
 * <string.h> wrapper functions.
 *
 * Authors:
 *   Jonathan Pryor (jonpryor@vt.edu)
 *
 * Copyright (C) 2005 Jonathan Pryor
 */

#if defined (HAVE_CONFIG_H)
#include <config.h>
#endif

#include <cstring>

#include "map.hh"
#include "mph.hh"

uint64_t
Mono_Posix_Stdlib_strlen (const char* p)
{
	return strlen (p);
}

/*
 * vim: noexpandtab
 */
