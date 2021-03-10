/*
 * <string.h> wrapper functions.
 *
 * Authors:
 *   Jonathan Pryor (jonpryor@vt.edu)
 *
 * Copyright (C) 2005 Jonathan Pryor
 */

#include <string.h>

#include "map.hh"
#include "mph.hh"

uint64_t
Mono_Posix_Stdlib_strlen (void* p)
{
	return strlen ((const char*) p);
}

/*
 * vim: noexpandtab
 */
