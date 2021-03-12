/*
 * <signal.h> wrapper functions.
 *
 * Authors:
 *   Jonathan Pryor (jonpryor@vt.edu)
 *   Jonathan Pryor (jpryor@novell.com)
 *   Tim Jenks (tim.jenks@realtimeworlds.com)
 *
 * Copyright (C) 2004-2005 Jonathan Pryor
 * Copyright (C) 2008 Novell, Inc.
 */

#if defined (HAVE_CONFIG_H)
#include <config.h>
#endif

#include <cassert>
#include <csignal>

#include "map.hh"
#include "mph.hh"

#ifndef HOST_WIN32
#include <sys/time.h>
#include <sys/types.h>
#if defined(HAVE_POLL_H)
#include <poll.h>
#elif defined(HAVE_SYS_POLL_H)
#include <sys/poll.h>
#endif
#include <unistd.h>
#include <cstdlib>
#include <cstring>
#include <pthread.h>
#endif // ndef HOST_WIN32

#ifndef HOST_WIN32
static int count_handlers (int signum);
#endif

mph_sig_t
Mono_Posix_Stdlib_SIG_DFL ()
{
	return SIG_DFL;
}

mph_sig_t
Mono_Posix_Stdlib_SIG_ERR ()
{
	return SIG_ERR;
}

mph_sig_t
Mono_Posix_Stdlib_SIG_IGN ()
{
	return SIG_IGN;
}

void
Mono_Posix_Stdlib_InvokeSignalHandler (int signum, mph_sig_t handler)
{
	if (handler == nullptr) {
		return;
	}

	handler (signum);
}

int
Mono_Posix_SIGRTMIN (void)
{
#ifdef SIGRTMIN
	return SIGRTMIN;
#else /* def SIGRTMIN */
	return -1;
#endif /* ndef SIGRTMIN */
}

int
Mono_Posix_SIGRTMAX (void)
{
#ifdef SIGRTMAX
	return SIGRTMAX;
#else /* def SIGRTMAX */
	return -1;
#endif /* ndef SIGRTMAX */
}

int
Mono_Posix_FromRealTimeSignum ([[maybe_unused]] int offset, int *r)
{
	if (nullptr == r) {
		errno = EINVAL;
		return -1;
	}
	*r = 0;
#if defined (SIGRTMIN) && defined (SIGRTMAX)
	if ((offset < 0) || (SIGRTMIN > SIGRTMAX - offset)) {
		errno = EINVAL;
		return -1;
	}
	*r = SIGRTMIN+offset;
	return 0;
#else /* defined (SIGRTMIN) && defined (SIGRTMAX) */
# ifdef ENOSYS
	errno = ENOSYS;
# endif /* ENOSYS */
	return -1;
#endif /* defined (SIGRTMIN) && defined (SIGRTMAX) */
}

#ifndef HOST_WIN32

// Atomicity rules: Fields of Mono_Unix_UnixSignal_SignalInfo read or written by the signal handler
// (see UnixSignal.cs) should be read and written using atomic functions.
// (For simplicity, we're protecting some things we don't strictly need to.)
#define mph_int_get(ptr)     __atomic_fetch_add ((ptr), (int32_t)0, __ATOMIC_SEQ_CST)
#define mph_int_inc(ptr)     __atomic_add_fetch ((ptr), 1, __ATOMIC_SEQ_CST)
#define mph_int_dec_test(ptr)     (__atomic_sub_fetch ((ptr), 1, __ATOMIC_SEQ_CST) == 0)
#define mph_int_set(ptr,val) __atomic_store_n ((ptr), (val), __ATOMIC_SEQ_CST)
// Pointer, original, new
#define mph_int_test_and_set(ptr,expected,desired) (*(expected) == __atomic_compare_exchange_n ((ptr), (expected), (desired), false, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST))

#if HAVE_PSIGNAL

/* 
 * HACK: similar to the mkdtemp one in glib; turns out gcc "helpfully"
 * shadows system headers with "fixed" versions that omit functions...
 * in any case, psignal is another victim of poor GNU decisions. Even
 * then, we may have to do this anyways, as psignal, while present in
 * libc, isn't in PASE headers - so do it anyways
 */
#if defined(_AIX)
extern void psignal(int, const char *);
#endif

int
Mono_Posix_Syscall_psignal (int sig, const char* s)
{
	errno = 0;
	psignal (sig, s);
	return errno == 0 ? 0 : -1;
}
#endif  /* def HAVE_PSIGNAL */

static constexpr size_t NUM_SIGNALS = 64;
static Mono_Unix_UnixSignal_SignalInfo signals[NUM_SIGNALS];

static int
acquire_mutex (pthread_mutex_t *mutex)
{
	int mr;
	while ((mr = pthread_mutex_lock (mutex)) == EAGAIN) {
		/* try to acquire again */
	}
	if ((mr != 0) && (mr != EDEADLK))  {
		errno = mr;
		return -1;
	}
	return 0;
}

static void
release_mutex (pthread_mutex_t *mutex)
{
	int mr;
	while ((mr = pthread_mutex_unlock (mutex)) == EAGAIN) {
		/* try to release mutex again */
	}
}

static int
keep_trying (int r)
{
	return r == -1 && errno == EINTR;
}

// This tiny ad-hoc read/write lock is needed because of the very specific
// synchronization needed between default_handler and teardown_pipes:
// - Many default_handlers can be running at once
// - The signals_mutex already ensures only one teardown_pipes runs at once
// - If teardown_pipes starts while a default_handler is ongoing, it must block
// - If default_handler starts while a teardown_pipes is ongoing, it must *not* block
// Locks are implemented as ints.

// The lock is split into a teardown bit and a handler count (sign bit unused).
// There is a teardown running or waiting to run if the teardown bit is set.
// There is a handler running if the handler count is nonzero.
static constexpr int PIPELOCK_TEARDOWN_BIT = 0x40000000;
static constexpr int PIPELOCK_COUNT_MASK = ~0xC0000000;

static inline constexpr int
PIPELOCK_GET_COUNT (int x)
{
	return (x & PIPELOCK_COUNT_MASK);
}

static inline constexpr int
PIPELOCK_INCR_COUNT (int x, int by)
{
	return (x & PIPELOCK_TEARDOWN_BIT) | (PIPELOCK_GET_COUNT (PIPELOCK_GET_COUNT (x) + by));
}

static inline void
acquire_pipelock_teardown (int *lock)
{
	int lockvalue_draining;
	// First mark that a teardown is occurring, so handlers will stop entering the lock.
	while (true) {
		int lockvalue = mph_int_get (lock);
		lockvalue_draining = lockvalue | PIPELOCK_TEARDOWN_BIT;
		if (mph_int_test_and_set (lock, &lockvalue, lockvalue_draining))
			break;
	}
	// Now wait for all handlers to complete.
	while (true) {
		if (0 == PIPELOCK_GET_COUNT (lockvalue_draining))
			break; // We now hold the lock.
		// Handler is still running, spin until it completes.
		sched_yield (); // We can call this because !defined(HOST_WIN32)
		lockvalue_draining = mph_int_get (lock);
	}
}

static inline void
release_pipelock_teardown (int *lock)
{
	while (true) {
		int lockvalue = mph_int_get (lock);
		int lockvalue_new = lockvalue & ~PIPELOCK_TEARDOWN_BIT;
		// Technically this can't fail, because we hold both the pipelock and the mutex, but
		if (mph_int_test_and_set (lock, &lockvalue, lockvalue_new))
			return;
	}
}

// Return 1 for success
static inline int
acquire_pipelock_handler (int *lock)
{
	while (true) {
		int lockvalue = mph_int_get (lock);
		if (lockvalue & PIPELOCK_TEARDOWN_BIT) // Final lock is being torn down
			return 0;
		int lockvalue_new = PIPELOCK_INCR_COUNT (lockvalue, 1);
		if (mph_int_test_and_set (lock, &lockvalue, lockvalue_new))
			return 1;
	}
}

static void
release_pipelock_handler (int *lock)
{
	while (true) {
		int lockvalue = mph_int_get (lock);
		int lockvalue_new = PIPELOCK_INCR_COUNT (lockvalue, -1);
		if (mph_int_test_and_set (lock, &lockvalue, lockvalue_new))
			return;
	}
}

// This handler is registered once for each UnixSignal object. A pipe is maintained
// for each one; Wait users read at one end of this pipe, and default_handler sends
// a write on the pipe for each signal received while the Wait is ongoing.
//
// Notice a fairly unlikely race condition exists here: Because we synchronize with
// pipe teardown, but not install/uninstall (in other words, we are only trying to
// protect against writing on a closed pipe) it is technically possible a full
// uninstall and then an install could complete after signum is checked but before
// the remaining instructions execute. In this unlikely case count could be
// incremented or a byte written on the wrong signal handler.
static void
default_handler (int signum)
{
	for (size_t i = 0; i < NUM_SIGNALS; ++i) {
		int fd;
		Mono_Unix_UnixSignal_SignalInfo* h = &signals [i];
		if (mph_int_get (&h->signum) != signum)
			continue;

		mph_int_inc (&h->count);

		if (!acquire_pipelock_handler (&h->pipelock))
			continue; // Teardown is occurring on this object, no one to send to.

		fd = mph_int_get (&h->write_fd);
		if (fd > 0) { // If any listener exists to write to
			char c = signum; // (Value is meaningless)
			int pipecounter = mph_int_get (&h->pipecnt); // Write one byte per pipe listener
			for (int j = 0; j < pipecounter; ++j) {
				int r;
				do { r = write (fd, &c, 1); } while (keep_trying (r));
			}
		}
		release_pipelock_handler (&h->pipelock);
	}
}

static pthread_mutex_t signals_mutex = PTHREAD_MUTEX_INITIALIZER;

// A UnixSignal object is being constructed
Mono_Unix_UnixSignal_SignalInfo*
Mono_Unix_UnixSignal_install (int sig)
{
#if defined(HAVE_SIGNAL)
	if (acquire_mutex (&signals_mutex) == -1)
		return nullptr;

#if defined (SIGRTMIN) && defined (SIGRTMAX)
	/*The runtime uses some rt signals for itself so it's important to not override them.*/
	if (sig >= SIGRTMIN && sig <= SIGRTMAX && count_handlers (sig) == 0) {
		struct sigaction sinfo;
		sigaction (sig, nullptr, &sinfo);
		// On some systems sa_handler and sa_sigaction are a union, but on some they are separate members of `struct
		// sigaction` and they may have different signatures, `SIG_DFL` cannot be used to check validity of
		// `sa_sigaction`
		if (sinfo.sa_handler != SIG_DFL || sinfo.sa_sigaction != nullptr) {
			pthread_mutex_unlock (&signals_mutex);
			errno = EADDRINUSE;
			return nullptr; // This is an rt signal with an existing handler. Bail out.
		}
	}
#endif /*defined (SIGRTMIN) && defined (SIGRTMAX)*/

	Mono_Unix_UnixSignal_SignalInfo* h = nullptr;        // signals[] slot to install to
	mph_sig_t handler = nullptr;
	bool have_handler = false;         // Candidates for Mono_Unix_UnixSignal_SignalInfo handler fields
	// Scan through signals list looking for (1) an unused spot (2) a usable value for handler
	for (size_t i = 0; i < NUM_SIGNALS; ++i) {
		bool just_installed = false;
		// We're still looking for a Mono_Unix_UnixSignal_SignalInfo spot, and this one is available:
		if (h == nullptr && mph_int_get (&signals [i].signum) == 0) {
			h = &signals [i];
			h->handler = signal (sig, default_handler);
			if (h->handler == SIG_ERR) {
				h->handler = nullptr;
				h = nullptr;
				break;
			}
			else {
				just_installed = true;
			}
		}
		// Check if this slot has a "usable" (not installed by this file) handler-to-restore-later:
		// (On the first signal to be installed, signals [i] will be == h when this happens.)
		if (!have_handler && (just_installed || mph_int_get (&signals [i].signum) == sig) && signals [i].handler != default_handler) {
			have_handler = true;
			handler = signals [i].handler;
		}
		if (h != nullptr && have_handler) // We have everything we need
			break;
	}

	if (h != nullptr) {
		// If we reached here without have_handler, this means that default_handler
		// was set as the signal handler before the first UnixSignal object was installed.
		assert (have_handler);

		// Overwrite the tenative handler we set a moment ago with a known-usable one
		h->handler = handler;
		h->have_handler = 1;

		mph_int_set (&h->count, 0);
		mph_int_set (&h->pipecnt, 0);
		mph_int_set (&h->signum, sig);
	}

	release_mutex (&signals_mutex);

	return h;
#else
	fprintf (stderr, ("signal() is not supported by this platform");
	return 0;
#endif
}

static inline int
count_handlers (int signum)
{
	int count = 0;
	for (size_t i = 0; i < NUM_SIGNALS; ++i) {
		if (mph_int_get (&signals [i].signum) == signum)
			++count;
	}
	return count;
}

// A UnixSignal object is being Disposed
int
Mono_Unix_UnixSignal_uninstall (Mono_Unix_UnixSignal_SignalInfo* info)
{
#if defined(HAVE_SIGNAL)
	if (acquire_mutex (&signals_mutex) == -1)
		return -1;

	Mono_Unix_UnixSignal_SignalInfo* h = info;
	int r = -1;

	if (h == nullptr || h < signals || h > &signals [NUM_SIGNALS])
		errno = EINVAL;
	else {
		/* last UnixSignal -- we can unregister */
		int signum = mph_int_get (&h->signum);
		if (h->have_handler && count_handlers (signum) == 1) {
			mph_sig_t p = signal (signum, h->handler);
			if (p != SIG_ERR)
				r = 0;
			h->handler      = nullptr;
			h->have_handler = 0;
		}
		mph_int_set (&h->signum, 0);
	}

	release_mutex (&signals_mutex);

	return r;
#else
	fprintf (stderr, "signal() is not supported by this platform");
	return 0;
#endif
}

// Set up a Mono_Unix_UnixSignal_SignalInfo to begin waiting for signal
static int
setup_pipes (Mono_Unix_UnixSignal_SignalInfo** signals, int count, struct pollfd *fd_structs, int *currfd)
{
	int r = 0;
	for (int i = 0; i < count; ++i) {
		Mono_Unix_UnixSignal_SignalInfo* h;
		int filedes[2];

		h = signals [i];

		if (mph_int_get (&h->pipecnt) == 0) { // First listener for this Mono_Unix_UnixSignal_SignalInfo
			if ((r = pipe (filedes)) != 0) {
				break;
			}
			mph_int_set (&h->read_fd,  filedes [0]);
			mph_int_set (&h->write_fd, filedes [1]);
		}
		mph_int_inc (&h->pipecnt);
		fd_structs[*currfd].fd = mph_int_get (&h->read_fd);
		fd_structs[*currfd].events = POLLIN;
		++(*currfd); // count is verified less than NUM_SIGNALS by caller
	}
	return r;
}

// Cleanup a Mono_Unix_UnixSignal_SignalInfo after waiting for signal
static void
teardown_pipes (Mono_Unix_UnixSignal_SignalInfo** signals, int count)
{
	for (int i = 0; i < count; ++i) {
		Mono_Unix_UnixSignal_SignalInfo* h = signals [i];

		if (mph_int_dec_test (&h->pipecnt)) { // Final listener for this Mono_Unix_UnixSignal_SignalInfo
			acquire_pipelock_teardown (&h->pipelock);
			int read_fd = mph_int_get (&h->read_fd);
			int write_fd = mph_int_get (&h->write_fd);
			if (read_fd != 0)
				close (read_fd);
			if (write_fd != 0)
				close (write_fd);
			mph_int_set (&h->read_fd, 0);
			mph_int_set (&h->write_fd, 0);
			release_pipelock_teardown (&h->pipelock);
		}
	}
}

// Given pipes set up, wait for a byte to arrive on one of them
static int
wait_for_any (Mono_Unix_UnixSignal_SignalInfo** signals, int count, struct pollfd* fd_structs, int timeout, Mono_Posix_RuntimeIsShuttingDown shutting_down)
{
	int r;
	// Poll until one of this Mono_Unix_UnixSignal_SignalInfo's pipes is ready to read.
	// Once a second, stop to check if the VM is shutting down.
	do {
		r = poll (fd_structs, count, timeout);
	} while (keep_trying (r) && !shutting_down ());

	int idx = -1;
	if (r == 0)
		idx = timeout;
	else if (r > 0) { // The pipe[s] are ready to read.
		for (int i = 0; i < count; ++i) {
			Mono_Unix_UnixSignal_SignalInfo* h = signals [i];
			if (fd_structs[i].revents & POLLIN) {
				int r;
				char c;
				do {
					r = read (mph_int_get (&h->read_fd), &c, 1);
				} while (keep_trying (r) && !shutting_down ());
				if (idx == -1)
					idx = i;
			}
		}
	}

	return idx;
}

/*
 * returns: -1 on error:
 *          timeout on timeout
 *          index into _signals array of signal that was generated on success
 */
int
Mono_Unix_UnixSignal_WaitAny (Mono_Unix_UnixSignal_SignalInfo** signals, int count, int timeout /* milliseconds */, Mono_Posix_RuntimeIsShuttingDown shutting_down)
{
	if (signals == nullptr || count < 0 || static_cast<size_t>(count) > NUM_SIGNALS) {
		return -1;
	}

	if (acquire_mutex (&signals_mutex) == -1) {
		return -1;
	}

	struct pollfd fd_structs[NUM_SIGNALS];
	int currfd = 0;
	int r = setup_pipes (signals, count, &fd_structs[0], &currfd);

	release_mutex (&signals_mutex);

	if (r == 0) {
		r = wait_for_any (signals, count, &fd_structs[0], timeout, shutting_down);
	}

	if (acquire_mutex (&signals_mutex) == -1) {
		return -1;
	}

	teardown_pipes (signals, count);

	release_mutex (&signals_mutex);

	return r;
}

#endif /* ndef HOST_WIN32 */

/*
 * vim: noexpandtab
 */
