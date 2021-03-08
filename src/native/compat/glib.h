#if !defined (__MPH_COMPAT_GLIB_H)
#define __MPH_COMPAT_GLIB_H

#include <limits.h>
#include <stdint.h>
#include <inttypes.h>
#include <unistd.h>

#ifdef  __cplusplus
#define G_BEGIN_DECLS  extern "C" {
#define G_END_DECLS    }
#define G_EXTERN_C     extern "C"
#else
#define G_BEGIN_DECLS  /* nothing */
#define G_END_DECLS    /* nothing */
#define G_EXTERN_C     /* nothing */
#endif

/*
 * Basic data types
 */
typedef int            gint;
typedef unsigned int   guint;
typedef short          gshort;
typedef unsigned short gushort;
typedef long           glong;
typedef unsigned long  gulong;
typedef void *         gpointer;
typedef const void *   gconstpointer;
typedef char           gchar;
typedef unsigned char  guchar;

/* Types defined in terms of the stdint.h */
typedef int8_t         gint8;
typedef uint8_t        guint8;
typedef int16_t        gint16;
typedef uint16_t       guint16;
typedef int32_t        gint32;
typedef uint32_t       guint32;
typedef int64_t        gint64;
typedef uint64_t       guint64;
typedef float          gfloat;
typedef double         gdouble;
typedef int32_t        gboolean;

typedef size_t         gsize;
typedef ssize_t        gssize;

#define G_MINSHORT           SHRT_MIN
#define G_MAXSHORT           SHRT_MAX
#define G_MAXUSHORT          USHRT_MAX
#define G_MAXINT             INT_MAX
#define G_MININT             INT_MIN
#define G_MAXINT8            INT8_MAX
#define G_MAXUINT8           UINT8_MAX
#define G_MININT8            INT8_MIN
#define G_MAXINT16           INT16_MAX
#define G_MAXUINT16          UINT16_MAX
#define G_MININT16           INT16_MIN
#define G_MAXINT32           INT32_MAX
#define G_MAXUINT32          UINT32_MAX
#define G_MININT32           INT32_MIN
#define G_MININT64           INT64_MIN
#define G_MAXINT64           INT64_MAX
#define G_MAXUINT64          UINT64_MAX

// - Pointers should only be converted to or from pointer-sized integers.
// - Any size integer can be converted to any other size integer.
// - Therefore a pointer-sized integer is the intermediary between
//   a pointer and any integer type.
#define GPOINTER_TO_INT(ptr)   ((gint)(gssize)(ptr))
#define GPOINTER_TO_UINT(ptr)  ((guint)(gsize)(ptr))
#define GINT_TO_POINTER(v)     ((gpointer)(gssize)(v))
#define GUINT_TO_POINTER(v)    ((gpointer)(gsize)(v))

#define G_STMT_START    do
#define G_STMT_END      while (0)

#ifndef MAX
#define MAX(a,b) (((a)>(b)) ? (a) : (b))
#endif

#if !defined (FALSE)
#define FALSE 0
#endif // ndef FALSE

#if !defined (TRUE)
#define TRUE 1
#endif // ndef TRUE

#endif // __MPH_COMPAT_GLIB_H
