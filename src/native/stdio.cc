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

#include <cstdarg>
#include <cstdio>
#include <cstdlib>

#include "map.hh"
#include "mph.hh"

int32_t
Mono_Posix_Syscall_L_ctermid ()
{
#ifndef HOST_WIN32
	return L_ctermid;
#else
	return -1;
#endif
}

int32_t
Mono_Posix_Syscall_L_cuserid ()
{
#if defined(__APPLE__) || defined (__OpenBSD__) || defined (HOST_WIN32)
	return -1;
#else
	return L_cuserid;
#endif
}

mph_size_t
Mono_Posix_Stdlib_fread (unsigned char *ptr, mph_size_t size, mph_size_t nmemb, FILE* stream)
{
	if (mph_have_size_t_overflow (size) || mph_have_size_t_overflow (nmemb)) {
		return 0;
	}

	return fread (ptr, size, nmemb, stream);
}

mph_size_t
Mono_Posix_Stdlib_fwrite (unsigned char *ptr, mph_size_t size, mph_size_t nmemb, FILE* stream)
{
	if (mph_have_size_t_overflow (size) || mph_have_size_t_overflow (nmemb)) {
		return 0;
	}

	size_t ret = fwrite (ptr, size, nmemb, stream);
#ifdef HOST_WIN32
	// Workaround for a particular weirdness on Windows triggered by the
	// StdioFileStreamTest.Write() test method. The test writes 15 bytes to a
	// file, then rewinds the file pointer and reads the same bytes. It then
	// writes 15 additional bytes to the file. This second write fails on
	// Windows with 0 returned from fwrite(). Calling fseek() followed by a retry
	// of fwrite() like we do here fixes the issue.
	if (ret != nmemb) {
		fseek (stream, 0, SEEK_CUR);
		ret = fwrite (ptr + (ret * nmemb), size, nmemb - ret, stream);
	}
#endif
	return ret;
}

#ifdef HAVE_VSNPRINTF
int32_t
Mono_Posix_Stdlib_snprintf (char *s, mph_size_t n, char *format, ...)
{
	if (mph_have_size_t_overflow (n)) {
		return -1;
	}

	va_list ap;
	int32_t r;

	va_start (ap, format);
	r = vsnprintf (s, (size_t) n, format, ap);
	va_end (ap);

	return r;
}
#endif /* def HAVE_VSNPRINTF */

int32_t
Mono_Posix_Stdlib__IOFBF ()
{
	return _IOFBF;
}

int32_t
Mono_Posix_Stdlib__IOLBF ()
{
	return _IOLBF;
}

int32_t
Mono_Posix_Stdlib__IONBF ()
{
	return _IONBF;
}

int32_t
Mono_Posix_Stdlib_BUFSIZ ()
{
	return BUFSIZ;
}

int32_t
Mono_Posix_Stdlib_EOF ()
{
	return EOF;
}

int32_t
Mono_Posix_Stdlib_FOPEN_MAX ()
{
	return FOPEN_MAX;
}

int32_t
Mono_Posix_Stdlib_FILENAME_MAX ()
{
	return FILENAME_MAX;
}

int32_t
Mono_Posix_Stdlib_L_tmpnam ()
{
	return L_tmpnam;
}

void*
Mono_Posix_Stdlib_stdin ()
{
	return stdin;
}

void*
Mono_Posix_Stdlib_stdout ()
{
	return stdout;
}

void*
Mono_Posix_Stdlib_stderr ()
{
	return stderr;
}

int32_t
Mono_Posix_Stdlib_TMP_MAX ()
{
	return TMP_MAX;
}

void*
Mono_Posix_Stdlib_tmpfile ()
{
	return tmpfile ();
}

int32_t
Mono_Posix_Stdlib_setvbuf (FILE* stream, char* buf, int mode, mph_size_t size)
{
	if (mph_have_size_t_overflow (size)) {
		return -1;
	}

	return setvbuf (stream, buf, mode, size);
}

int 
Mono_Posix_Stdlib_setbuf (FILE* stream, char* buf)
{
	setbuf (stream, buf);
	return 0;
}

FILE*
Mono_Posix_Stdlib_fopen (const char* path, const char* mode)
{
	// NULL mode segfaults, NULL path does not
	if (mode == nullptr) {
		return nullptr;
	}

	return fopen (path, mode);
}

FILE*
Mono_Posix_Stdlib_freopen (const char* path, const char* mode, FILE* stream)
{
	// NULL mode and stream segfault, NULL path does not
	if (mode == nullptr || stream == nullptr) {
		return nullptr;
	}

	return freopen (path, mode, stream);
}

int32_t
Mono_Posix_Stdlib_fprintf (FILE* stream, const char* format, const char* message)
{
	if (stream == nullptr) {
		return -1;
	}

	// codeql[cpp/non-constant-format] this is an fprintf wrapper for managed code so we can't have a constant string
	return fprintf (stream, format, message);
}

int32_t
Mono_Posix_Stdlib_fgetc (FILE* stream)
{
	if (stream == nullptr) {
		return EOF;
	}

	return fgetc (stream);
}

char*
Mono_Posix_Stdlib_fgets (char* str, int32_t size, FILE* stream)
{
	// Prevent a segfault or a do-nothing call
	if (size < 0 || str == nullptr || stream == nullptr) {
		return nullptr;
	}

	return fgets (str, size, stream);
}

int32_t
Mono_Posix_Stdlib_fputc (int32_t c, FILE* stream)
{
	if (stream == nullptr) {
		return EOF;
	}

	return fputc (c, stream);
}

int32_t
Mono_Posix_Stdlib_fputs (const char* s, FILE* stream)
{
	if (s == nullptr || stream == nullptr) {
		return EOF;
	}
	return fputs (s, stream);
}

int32_t
Mono_Posix_Stdlib_fclose (FILE* stream)
{
	if (stream == nullptr) {
		errno = EFAULT;
		return EOF;
	}

	return fclose (stream);
}

int32_t
Mono_Posix_Stdlib_fflush (FILE* stream)
{
	if (stream == nullptr) {
		errno = EINVAL;
		return EOF;
	}

	return fflush (stream);
}

int32_t
Mono_Posix_Stdlib_fseek (FILE* stream, int64_t offset, int origin)
{
	if (stream == nullptr) {
		errno = EFAULT;
		return -1;
	}

	if (mph_have_long_overflow (offset)) {
		errno = EINVAL;
		return -1;
	}

	return fseek (stream, offset, origin);
}

int64_t
Mono_Posix_Stdlib_ftell (FILE* stream)
{
	if (stream == nullptr) {
		errno = EFAULT;
		return -1;
	}

	return ftell (stream);
}

fpos_t*
Mono_Posix_Stdlib_CreateFilePosition ()
{
	return static_cast<fpos_t*>(malloc (sizeof(fpos_t)));
}

int32_t
Mono_Posix_Stdlib_fgetpos (FILE* stream, fpos_t* pos)
{
	if (stream == nullptr) {
		errno = EFAULT;
		return -1;
	}

	return fgetpos (stream, pos);
}

int32_t
Mono_Posix_Stdlib_fsetpos (FILE* stream, fpos_t* pos)
{
	if (stream == nullptr || pos == nullptr) {
		errno = EFAULT;
		return -1;
	}

	return fsetpos (stream, (fpos_t*) pos);
}

int
Mono_Posix_Stdlib_rewind (FILE* stream)
{
	if (stream == nullptr) {
		errno = EFAULT;
		return -1;
	}

	do {
		rewind (stream);
	} while (errno == EINTR);

	if (mph_value_in_list (errno, EAGAIN, EBADF, EFBIG, EINVAL, EIO, ENOSPC, ENXIO, EOVERFLOW, EPIPE, ESPIPE)) {
		return -1;
	}

	return 0;
}

int
Mono_Posix_Stdlib_clearerr (FILE* stream)
{
	if (stream == nullptr) {
		return -1;
	}

	clearerr (stream);
	return 0;
}

int32_t
Mono_Posix_Stdlib_ungetc (int32_t c, FILE* stream)
{
	if (stream == nullptr) {
		return EOF;
	}

	return ungetc (c, stream);
}

int32_t
Mono_Posix_Stdlib_feof (FILE* stream)
{
	if (stream == nullptr) {
		return -1;
	}

	return feof (stream);
}

int32_t
Mono_Posix_Stdlib_ferror (FILE* stream)
{
	if (stream == nullptr) {
		return -1;
	}

	return ferror (stream);
}

int
Mono_Posix_Stdlib_perror (const char* s, int err)
{
	errno = err;
	perror (s);
	return 0;
}

static constexpr size_t MPH_FPOS_LENGTH = sizeof(fpos_t) * 2;

int
Mono_Posix_Stdlib_DumpFilePosition (char *dest, void* pos, int32_t len)
{
	if (dest == nullptr)
		return MPH_FPOS_LENGTH;

	if (pos == nullptr || len <= 0) {
		errno = EINVAL;
		return -1;
	}

	auto posp = static_cast<unsigned char*>(pos);
	unsigned char *pose = posp + sizeof(fpos_t);
	char *destp = dest;

	for ( ; posp < pose && len > 1; destp += 2, ++posp, len -= 2) {
		sprintf (destp, "%02X", *posp);
	}

	if (len)
		dest[MPH_FPOS_LENGTH] = '\0';

	return destp - dest;
}

/*
 * vim: noexpandtab
 */
