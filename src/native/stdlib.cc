/*
 * <stdlib.h> wrapper functions.
 *
 * Authors:
 *   Jonathan Pryor (jonpryor@vt.edu)
 *
 * Copyright (C) 2004-2005 Jonathan Pryor
 */
#if defined (HAVE_CONFIG_H)
#include <config.h>
#endif

#include <cstdlib>

#include "map.hh"
#include "mph.hh"

// See Stdlib.cs - don't change this string without changing Stdlib.cs
static constexpr char VersionString[] = "MonoProject-2015-12-1";
const char*
Mono_Unix_VersionString ()
{
	return VersionString;
}

int32_t
Mono_Posix_Stdlib_EXIT_FAILURE ()
{
	return EXIT_FAILURE;
}

int32_t
Mono_Posix_Stdlib_EXIT_SUCCESS ()
{
	return EXIT_SUCCESS;
}

int32_t
Mono_Posix_Stdlib_MB_CUR_MAX ()
{
	return MB_CUR_MAX;
}

int32_t
Mono_Posix_Stdlib_RAND_MAX ()
{
	return RAND_MAX;
}

void*
Mono_Posix_Stdlib_calloc (mph_size_t nmemb, mph_size_t size)
{
	if (mph_have_size_t_overflow (nmemb) || mph_have_size_t_overflow (size))
		return nullptr;

	return calloc (nmemb, size);
}

void*
Mono_Posix_Stdlib_malloc (mph_size_t size)
{
	if (mph_have_size_t_overflow (size))
		return nullptr;

	return malloc (size);
}

void*
Mono_Posix_Stdlib_realloc (void* ptr, mph_size_t size)
{
	if (mph_have_size_t_overflow (size))
		return nullptr;

	return realloc (ptr, size);
}

void
Mono_Posix_Stdlib_free (void* p)
{
	free (p);
}

/*
 * vim: noexpandtab
 */
