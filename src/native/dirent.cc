/*
 * <dirent.h> wrapper functions.
 *
 * Authors:
 *   Jonathan Pryor (jonpryor@vt.edu)
 *
 * Copyright (C) 2004-2005 Jonathan Pryor
 */
#if defined (HAVE_CONFIG_H)
#include <config.h>
#endif

#include <algorithm>

#include <dirent.h>
#include <cerrno>
#include <cstring>
#include <cstdlib>
#include <climits>
#include <unistd.h>

#include "map.hh"
#include "mph.hh"

#if defined (PATH_MAX) && defined (NAME_MAX)
static constexpr size_t MPH_PATH_MAX = std::max(PATH_MAX, NAME_MAX);
#elif defined (PATH_MAX)
static constexpr size_t MPH_PATH_MAX = PATH_MAX;
#elif defined (NAME_MAX)
static constexpr size_t MPH_PATH_MAX = NAME_MAX;
#else /* !defined PATH_MAX && !defined NAME_MAX */
static constexpr size_t MPH_PATH_MAX = 2048;
#endif

#if HAVE_SEEKDIR
int32_t
Mono_Posix_Syscall_seekdir (DIR *dir, mph_off_t offset)
{
	if (dir == nullptr || mph_have_off_t_overflow (offset)) {
		return -1;
	}

	seekdir (dir, offset);

	return 0;
}
#endif  /* def HAVE_SEEKDIR */

#if HAVE_TELLDIR
int64_t
Mono_Posix_Syscall_telldir (DIR *dir)
{
	if (dir == nullptr) {
		return -1;
	}

	return telldir (dir);
}
#endif  /* def HAVE_TELLDIR */

static inline void
copy_dirent (struct Mono_Posix_Syscall__Dirent *to, struct dirent *from)
{
	memset (to, 0, sizeof(*to));

	to->d_ino    = from->d_ino;
	to->d_name   = strdup (from->d_name);

#ifdef HAVE_STRUCT_DIRENT_D_OFF
	to->d_off    = from->d_off;
#endif
#ifdef HAVE_STRUCT_DIRENT_D_RECLEN
	to->d_reclen = from->d_reclen;
#endif
#ifdef HAVE_STRUCT_DIRENT_D_TYPE
	to->d_type   = from->d_type;
#endif
}

int32_t
Mono_Posix_Syscall_readdir (DIR *dirp, struct Mono_Posix_Syscall__Dirent *entry)
{
	if (dirp == nullptr || entry == nullptr) {
		errno = EFAULT;
		return -1;
	}

	errno = 0;
	struct dirent *d = readdir (dirp);

	if (d == nullptr) {
		return -1;
	}

	copy_dirent (entry, d);

	return 0;
}

int32_t
Mono_Posix_Syscall_readdir_r (DIR *dirp, struct Mono_Posix_Syscall__Dirent *entry, struct dirent** result)
{
	if (result == nullptr) {
		return (errno = EFAULT);
	}

	auto _entry = static_cast<struct dirent*> (malloc (sizeof (struct dirent) + MPH_PATH_MAX + 1));

	// TODO: mark the managed API as deprecated
	// readdir_r is deprecated, but we still want to support it, so hush the warning
#if defined (__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
#elif defined (__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#endif

	errno = 0;
	int r = readdir_r (dirp, _entry, (struct dirent**) result);

#if defined (__clang__)
#pragma clang diagnostic pop
#elif defined (__GNUC__)
#pragma GCC diagnostic pop
#endif

	if (r == 0 && *result != nullptr) {
		copy_dirent (entry, _entry);
	}

	free (_entry);

	return r;
}

int32_t
Mono_Posix_Syscall_rewinddir (DIR* dir)
{
	if (dir == nullptr) {
		return -1;
	}

	rewinddir (dir);
	return 0;
}

/*
 * vim: noexpandtab
 */
