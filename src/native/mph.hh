/*
 * Common/shared macros and routines.
 *
 * This file contains macros of the form
 *
 *   mph_return_if_TYPE_overflow(val);
 *
 * Which tests `val' for a TYPE underflow/overflow (that is, is `val' within
 * the range for TYPE?).  If `val' can't fit in TYPE, errno is set to
 * EOVERFLOW, and `return -1' is executed (which is why it's a macro).
 *
 * Assumptions:
 *
 * I'm working from GLibc, so that's the basis for my assumptions.  They may
 * not be completely portable, in which case I'll need to fix my assumptions.
 * :-(
 *
 * See the typedefs for type size assumptions.  These typedefs *must* be kept
 * in sync with the types used in Mono.Posix.dll.
 *
 * See also:
 *   http://developer.apple.com/documentation/Darwin/Reference/ManPages/
 */

#ifndef INC_mph_H
#define INC_mph_H

#if defined (HAVE_CONFIG_H)
#include <config.h>
#endif // HAVE_CONFIG_HAVE

#include <type_traits>

#include <stddef.h>             /* offsetof */
#include <limits.h>             /* LONG_MAX, ULONG_MAX */
#include <errno.h>              /* for ERANGE */
#include <glib.h>               /* for g* types, etc. */

#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif

#ifdef HAVE_STDINT_H
#include <stdint.h>             /* for SIZE_MAX */
#endif

#ifdef ANDROID_UNIFIED_HEADERS
#ifdef HAVE_STDIO_H
#include <stdio.h>
#endif

#undef st_atime_nsec
#undef st_mtime_nsec
#undef st_ctime_nsec

#ifndef L_cuserid
#define L_cuserid       9       /* size for cuserid(); UT_NAMESIZE + 1 */
#endif

/* NDK unified headers will define fpos_t to be 64-bit if large files support is
 * enabled (which is the case with Mono) so we need to make sure the offsets here
 * are actually 32-bit for Android APIs before API24 which did NOT have the 64-bit
 * versions.
 */
#if !defined(fgetpos) && __ANDROID_API__ < 24
int fgetpos(FILE*, fpos_t*);
#endif

#if !defined(fsetpos) && __ANDROID_API__ < 24
int fsetpos(FILE*, const fpos_t*);
#endif

#ifdef HAVE_PWD_H
#include <pwd.h>
#endif
/* Unified headers define 'pw_gecos' to be an alias for 'pw_passwd` on 32-bit Android which
 * results in two fields named 'pw_passwd' in map.h's 'struct passwd'
 */
#if !defined(__LP64__) && defined(pw_gecos)
#undef pw_gecos
#undef HAVE_STRUCT_PASSWD_PW_GECOS
#endif

#endif

#if __APPLE__ || __BSD__ || __FreeBSD__ || __OpenBSD__
#define MPH_ON_BSD
#endif

#if defined (_MSC_VER)
#define MPH_API_EXPORT __declspec(dllexport)
#elif defined (__clang__) || defined (__GNUC__) // def _MSC_VER
#define MPH_API_EXPORT __attribute__ ((__visibility__ ("default")))
#else // def __clang__ || def __GNUC__
#define MPH_API_EXPORT
#endif

#if defined (__cplusplus)
#define MPH_API extern "C" MPH_API_EXPORT
#else
#define MPH_API MPH_API_EXPORT
#endif

#if !defined(EOVERFLOW)
#  if defined(HOST_WIN32)
#    define EOVERFLOW 75
#  elif defined(__OpenBSD__)
#    define EOVERFLOW 87
#  endif
#endif /* !defined(EOVERFLOW) */

/* 
 * Solaris/Windows don't define these BSD values, and if they're not present
 * then map.c:Mono_Posix_FromSeekFlags() breaks badly; see:
 * http://bugzilla.gnome.org/show_bug.cgi?id=370081
 */

#ifndef L_SET
#define L_SET SEEK_SET
#endif /* ndef L_SET */

#ifndef L_INCR
#define L_INCR SEEK_CUR
#endif /* ndef L_INCR */

#ifndef L_XTND
#define L_XTND SEEK_END
#endif /* ndef L_XTND */

#if !defined (HOST_WIN32)

/*
 * OS X doesn't define MAP_ANONYMOUS, but it does define MAP_ANON.
 * Alias them to fix: https://bugzilla.xamarin.com/show_bug.cgi?id=3419
 */
#ifdef HOST_DARWIN
#ifndef MAP_ANONYMOUS
#define MAP_ANONYMOUS MAP_ANON
#endif  /* ndef MAP_ANONYMOUS */
#endif  /* ndef HOST_DARWIN */

/*
 * XATTR_AUTO is a synonym for 0 within XattrFlags, but most systems don't
 * define it.  map.c doesn't know that, though, so we ensure that it's defined
 * so that the value 0 round-trips through MonoPosixHelper.
 */

#ifndef XATTR_AUTO
#define XATTR_AUTO 0
#endif /* ndef XATTR_AUTO */

#endif /* ndef HOST_WIN32 */

typedef    int64_t mph_blkcnt_t;
typedef    int64_t mph_blksize_t;
typedef   uint64_t mph_dev_t;
typedef   uint64_t mph_ino_t;
typedef   uint64_t mph_nlink_t;
typedef    int64_t mph_off_t;
typedef   uint64_t mph_size_t;
typedef    int64_t mph_ssize_t;
typedef    int32_t mph_pid_t;
typedef   uint32_t mph_gid_t;
typedef   uint32_t mph_uid_t;
typedef    int64_t mph_time_t;
typedef    int64_t mph_clock_t;
typedef   uint64_t mph_fsblkcnt_t;
typedef   uint64_t mph_fsfilcnt_t;

/* Some versions of OS X don't define these typedefs, needed by map.c */
#ifndef HAVE_BLKCNT_T
typedef mph_blkcnt_t blkcnt_t;
#endif

#ifndef HAVE_BLKSIZE_T
typedef mph_blksize_t blksize_t;
#endif

#ifndef HAVE_SUSECONDS_T
typedef int64_t suseconds_t;
#endif

#ifdef HAVE_LARGE_FILE_SUPPORT
#define MPH_OFF_T_MAX G_MAXINT64
#define MPH_OFF_T_MIN G_MININT64
#else
#define MPH_OFF_T_MAX G_MAXINT32
#define MPH_OFF_T_MIN G_MININT32
#endif

#ifdef SIZE_MAX
#define MPH_SIZE_T_MAX SIZE_MAX
#elif SIZEOF_SIZE_T == 8
#define MPH_SIZE_T_MAX  G_MAXUINT64
#elif SIZEOF_SIZE_T == 4
#define MPH_SIZE_T_MAX  G_MAXUINT32
#else
#error "sizeof(size_t) is unknown!"
#endif

#define DECLARE_ENUM_FLAG_OPERATORS(__enum_name__)                      \
        constexpr __enum_name__ operator& (__enum_name__ x, __enum_name__ y) \
        { \
                typedef std::underlying_type<__enum_name__>::type etype; \
                return __enum_name__ (static_cast<etype> (x) & static_cast<etype> (y)); \
        } \
        constexpr __enum_name__& operator&= (__enum_name__& x, __enum_name__ y) \
        { \
                typedef std::underlying_type<__enum_name__>::type etype; \
                return x = __enum_name__ (static_cast<etype> (x) & static_cast<etype> (y)); \
        } \
        constexpr __enum_name__ operator| (__enum_name__ x, __enum_name__ y) \
        { \
                typedef std::underlying_type<__enum_name__>::type etype; \
                return __enum_name__ (static_cast<etype> (x) | static_cast<etype> (y)); \
        } \
        constexpr __enum_name__& operator|= (__enum_name__& x, __enum_name__ y) \
        { \
                typedef std::underlying_type<__enum_name__>::type etype; \
                return x = __enum_name__ (static_cast<etype> (x) | static_cast<etype> (y)); \
        } \
        constexpr __enum_name__ operator^ (__enum_name__ x, __enum_name__ y) \
        { \
                typedef std::underlying_type<__enum_name__>::type etype; \
                return __enum_name__ (static_cast<etype> (x) ^ static_cast<etype> (y)); \
        } \
        constexpr __enum_name__& operator^= (__enum_name__& x, __enum_name__ y) \
        { \
                typedef std::underlying_type<__enum_name__>::type etype; \
                return x = __enum_name__ (static_cast<etype> (x) ^ static_cast<etype> (y)); \
        } \
        constexpr __enum_name__& operator~ (__enum_name__& x) \
        { \
                typedef std::underlying_type<__enum_name__>::type etype; \
                return x = __enum_name__ (~static_cast<etype> (x)); \
        } \
        constexpr __enum_name__ operator~ (__enum_name__ x) \
        { \
                typedef std::underlying_type<__enum_name__>::type etype; \
                return __enum_name__ (~static_cast<etype> (x)); \
        }

#define _mph_return_val_if_cb_(val, ret, cb) G_STMT_START{ \
	if (cb (val)) { \
		errno = EOVERFLOW; \
		return ret; \
	}}G_STMT_END

#define mph_have_uint_overflow(var) ((var) < 0 || (var) > UINT_MAX)

#define mph_return_val_if_uint_overflow(var, ret) \
	_mph_return_val_if_cb_(var, ret, mph_have_uint_overflow)

#define mph_return_if_uint_overflow(var) mph_return_val_if_uint_overflow(var, -1)

#define mph_have_long_overflow(var) ((var) > LONG_MAX || (var) < LONG_MIN)

#define mph_return_val_if_long_overflow(var, ret) \
	_mph_return_val_if_cb_(var, ret, mph_have_long_overflow)

#define mph_return_if_long_overflow(var) mph_return_val_if_long_overflow(var, -1)

#define mph_have_ulong_overflow(var) (var) < 0 || ((var) > ULONG_MAX)

#define mph_return_val_if_ulong_overflow(var, ret) \
	_mph_return_val_if_cb_(var, ret, mph_have_ulong_overflow)

#define mph_return_if_ulong_overflow(var) mph_return_val_if_ulong_overflow(var, -1)

#define mph_have_size_t_overflow(var) ((var) < 0 || (var) > MPH_SIZE_T_MAX)

#define mph_return_val_if_size_t_overflow(var, ret) \
	_mph_return_val_if_cb_(var, ret, mph_have_size_t_overflow)

#define mph_return_val_if_ssize_t_overflow(var, ret) \
	_mph_return_val_if_cb_(var, ret, mph_have_long_overflow)

#define mph_return_if_size_t_overflow(var) mph_return_val_if_size_t_overflow(var, -1)

#define mph_return_if_ssize_t_overflow(var) mph_return_val_if_ssize_t_overflow(var, -1)

#define mph_have_off_t_overflow(var) \
	(((var) < MPH_OFF_T_MIN) || ((var) > MPH_OFF_T_MAX))

#define mph_return_val_if_off_t_overflow(var, ret) \
	_mph_return_val_if_cb_(var, ret, mph_have_off_t_overflow)

#define mph_return_if_off_t_overflow(var) mph_return_val_if_long_overflow(var, -1)

#define mph_return_if_time_t_overflow(var) mph_return_if_long_overflow(var)

#define mph_return_if_socklen_t_overflow(var) mph_return_if_uint_overflow(var)

#define mph_return_if_val_in_list5(var,a,b,c,d,e) \
	do {                                                            \
		int v = (var);                                                \
		if (v == a || v == b || v == c || v == d || v == e)           \
			return -1;                                                  \
	} while (0)

/*
 * Helper function for functions which use ERANGE (such as getpwnam_r and
 * getgrnam_r).  These functions accept buffers which are dynamically
 * allocated so that they're only as large as necessary.  However, Linux and
 * Mac OS X differ on how to signal an error value.
 *
 * Linux returns the error value directly, while Mac OS X is more traditional,
 * returning -1 and setting errno accordingly.
 *
 * Unify the checking in one place.
 */
static inline int
recheck_range (int ret)
{
	if (ret == ERANGE)
		return 1;
	if (ret == -1)
		return errno == ERANGE;
	return 0;
}

typedef unsigned int mph_string_offset_t;

enum {
	MPH_STRING_OFFSET_PTR   = 0x0,
	MPH_STRING_OFFSET_ARRAY = 0x1,
	MPH_STRING_OFFSET_MASK  = 0x1
};

#define MPH_STRING_OFFSET(type,member,kind) ((offsetof(type,member) << 1) | kind)

char*
_mph_copy_structure_strings (
	void *to,         const mph_string_offset_t *to_offsets, 
	const void *from, const mph_string_offset_t *from_offsets, 
	size_t num_strings);

#endif /* ndef INC_mph_H */

/*
 * vim: noexpandtab
 */
