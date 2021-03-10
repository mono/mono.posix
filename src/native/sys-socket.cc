/*
 * <sys/socket.h> wrapper functions.
 *
 * Authors:
 *   Steffen Kiess (s-kiess@web.de)
 *
 * Copyright (C) 2015 Steffen Kiess
 */
#if defined (HAVE_CONFIG_H)
#include <config.h>
#endif

#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <sys/un.h>
#include <unistd.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>

#include "map.hh"
#include "mph.hh"
#include "sys-uio.hh"

int
Mono_Posix_SockaddrStorage_get_size (void)
{
	return sizeof (struct sockaddr_storage);
}

int
Mono_Posix_SockaddrUn_get_sizeof_sun_path (void)
{
	struct sockaddr_un sun;
	return sizeof (sun.sun_path);
}

int
Mono_Posix_Cmsghdr_getsize (void)
{
	return sizeof (struct cmsghdr);
}

int
Mono_Posix_FromInAddr (struct Mono_Posix_InAddr* source, void* destination)
{
	memcpy (&((struct in_addr*)destination)->s_addr, &source->s_addr, 4);
	return 0;
}

int
Mono_Posix_ToInAddr (void* source, struct Mono_Posix_InAddr* destination)
{
	memcpy (&destination->s_addr, &((struct in_addr*)source)->s_addr, 4);
	return 0;
}

int
Mono_Posix_FromIn6Addr (struct Mono_Posix_In6Addr* source, void* destination)
{
	memcpy (&((struct in6_addr*)destination)->s6_addr, &source->addr0, 16);
	return 0;
}

int
Mono_Posix_ToIn6Addr (void* source, struct Mono_Posix_In6Addr* destination)
{
	memcpy (&destination->addr0, &((struct in6_addr*)source)->s6_addr, 16);
	return 0;
}


int
Mono_Posix_Syscall_socketpair (int domain, int type, int protocol, int* socket1, int* socket2)
{
	int filedes[2] = {-1, -1};
	int r;

	r = socketpair (domain, type, protocol, filedes);

	*socket1 = filedes[0];
	*socket2 = filedes[1];
	return r;
}

int
Mono_Posix_Syscall_getsockopt (int socket, int level, int option_name, void* option_value, int64_t* option_len)
{
	socklen_t len;
	int r;

	mph_return_if_socklen_t_overflow (*option_len);

	len = *option_len;

	r = getsockopt (socket, level, option_name, option_value, &len);

	*option_len = len;

	return r;
}

int
Mono_Posix_Syscall_getsockopt_timeval (int socket, int level, int option_name, struct Mono_Posix_Timeval* option_value)
{
	struct timeval tv;
	int r;
	socklen_t size;

	size = sizeof (struct timeval);
	r = getsockopt (socket, level, option_name, &tv, &size);

	if (r != -1 && size == sizeof (struct timeval)) {
		if (Mono_Posix_ToTimeval (&tv, option_value) != 0)
			return -1;
	} else {
		memset (option_value, 0, sizeof (struct Mono_Posix_Timeval));
		if (r != -1)
			errno = EINVAL;
	}

	return r;
}

int
Mono_Posix_Syscall_getsockopt_linger (int socket, int level, int option_name, struct Mono_Posix_Linger* option_value)
{
	struct linger ling;
	int r;
	socklen_t size;

	size = sizeof (struct linger);
	r = getsockopt (socket, level, option_name, &ling, &size);

	if (r != -1 && size == sizeof (struct linger)) {
		if (Mono_Posix_ToLinger (&ling, option_value) != 0)
			return -1;
	} else {
		memset (option_value, 0, sizeof (struct Mono_Posix_Linger));
		if (r != -1)
			errno = EINVAL;
	}

	return r;
}

int
Mono_Posix_Syscall_setsockopt (int socket, int level, int option_name, void* option_value, int64_t option_len)
{
	mph_return_if_socklen_t_overflow (option_len);

	return setsockopt (socket, level, option_name, option_value, option_len);
}

int
Mono_Posix_Syscall_setsockopt_timeval (int socket, int level, int option_name, struct Mono_Posix_Timeval* option_value)
{
	struct timeval tv;

	if (Mono_Posix_FromTimeval (option_value, &tv) != 0)
		return -1;

	return setsockopt (socket, level, option_name, &tv, sizeof (struct timeval));
}

int
Mono_Posix_Syscall_setsockopt_linger (int socket, int level, int option_name, struct Mono_Posix_Linger* option_value)
{
	struct linger ling;

	if (Mono_Posix_FromLinger (option_value, &ling) != 0)
		return -1;

	return setsockopt (socket, level, option_name, &ling, sizeof (struct linger));
}

static bool
get_addrlen (struct Mono_Posix__SockaddrHeader* address, socklen_t& addrlen)
{
	if (address == nullptr) {
		addrlen = 0;
		return true;
	}

	switch (address->type) {
		case Mono_Posix_SockaddrType_SockaddrStorage:
			mph_return_if_socklen_t_overflow (((struct Mono_Posix__SockaddrDynamic*) address)->len);
			addrlen = ((struct Mono_Posix__SockaddrDynamic*) address)->len;
			return true;

		case Mono_Posix_SockaddrType_SockaddrUn:
			mph_return_if_socklen_t_overflow (offsetof (struct sockaddr_un, sun_path) + ((struct Mono_Posix__SockaddrDynamic*) address)->len);
			addrlen = offsetof (struct sockaddr_un, sun_path) + ((struct Mono_Posix__SockaddrDynamic*) address)->len;
			return true;

		case Mono_Posix_SockaddrType_Sockaddr:
			addrlen = sizeof (struct sockaddr);
			return true;

		case Mono_Posix_SockaddrType_SockaddrIn:
			addrlen = sizeof (struct sockaddr_in);
			return true;

		case Mono_Posix_SockaddrType_SockaddrIn6:
			addrlen = sizeof (struct sockaddr_in6);
			return false;

		default:
			addrlen = 0;
			errno = EINVAL;
			return false;
	}
}

int
Mono_Posix_Sockaddr_GetNativeSize (struct Mono_Posix__SockaddrHeader* address, int64_t* size)
{
	socklen_t value = 0;

	bool ret = get_addrlen (address, value);
	*size = value;
	return ret ? 0 : -1;
}

int
Mono_Posix_FromSockaddr (struct Mono_Posix__SockaddrHeader* source, void* destination)
{
	if (!source)
		return 0;

	switch (source->type) {
	case Mono_Posix_SockaddrType_SockaddrStorage:
		// Do nothing, don't copy source->sa_family into addr->sa_family
		return 0;

	case Mono_Posix_SockaddrType_SockaddrUn:
		memcpy (((struct sockaddr_un*) destination)->sun_path, ((struct Mono_Posix__SockaddrDynamic*) source)->data, ((struct Mono_Posix__SockaddrDynamic*) source)->len);
		break;

	case Mono_Posix_SockaddrType_Sockaddr:
		break;

	case Mono_Posix_SockaddrType_SockaddrIn:
		if (Mono_Posix_FromSockaddrIn ((struct Mono_Posix_SockaddrIn*) source, (struct sockaddr_in*) destination) != 0)
			return -1;
		break;

	case Mono_Posix_SockaddrType_SockaddrIn6:
		if (Mono_Posix_FromSockaddrIn6 ((struct Mono_Posix_SockaddrIn6*) source, (struct sockaddr_in6*) destination) != 0)
			return -1;
		break;

	default:
		errno = EINVAL;
		return -1;
	}

	int family;
	if (Mono_Posix_FromUnixAddressFamily (source->sa_family, &family) != 0)
		return -1;
	((struct sockaddr*) destination)->sa_family = family;

	return 0;
}

int
Mono_Posix_ToSockaddr (void* source, int64_t size, struct Mono_Posix__SockaddrHeader* destination)
{
	struct Mono_Posix__SockaddrDynamic* destination_dyn;

	if (!destination || size < 0)
		return 0;

	switch (destination->type) {
	case Mono_Posix_SockaddrType_Sockaddr:
		if ((size_t)size < offsetof (struct sockaddr, sa_family) + sizeof (sa_family_t)) {
			errno = ENOBUFS;
			return -1;
		}
		break;

	case Mono_Posix_SockaddrType_SockaddrStorage:
		destination_dyn = ((struct Mono_Posix__SockaddrDynamic*) destination);
		if (size > destination_dyn->len) {
			errno = ENOBUFS;
			return -1;
		}
		destination_dyn->len = size;
		break;

	case Mono_Posix_SockaddrType_SockaddrUn:
		destination_dyn = ((struct Mono_Posix__SockaddrDynamic*) destination);
		if (destination_dyn->len < 0 || (size_t)size - offsetof (struct sockaddr_un, sun_path) > (size_t)destination_dyn->len) {
			errno = ENOBUFS;
			return -1;
		}
		destination_dyn->len = size - offsetof (struct sockaddr_un, sun_path);
		memcpy (destination_dyn->data, ((struct sockaddr_un*) source)->sun_path, size);
		break;

	case Mono_Posix_SockaddrType_SockaddrIn:
		if (size != sizeof (struct sockaddr_in)) {
			errno = ENOBUFS;
			return -1;
		}
		if (Mono_Posix_ToSockaddrIn ((struct sockaddr_in*) source, (struct Mono_Posix_SockaddrIn*) destination) != 0)
			return -1;
		break;

	case Mono_Posix_SockaddrType_SockaddrIn6:
		if (size != sizeof (struct sockaddr_in6)) {
			errno = ENOBUFS;
			return -1;
		}
		if (Mono_Posix_ToSockaddrIn6 ((struct sockaddr_in6*) source, (struct Mono_Posix_SockaddrIn6*) destination) != 0)
			return -1;
		break;

	default:
		errno = EINVAL;
		return -1;
	}

	if (Mono_Posix_ToUnixAddressFamily (((struct sockaddr*) source)->sa_family, &destination->sa_family) != 0)
		destination->sa_family = Mono_Posix_UnixAddressFamily_Unknown;

	return 0;
}

// Can't use RAII since exceptions are disabled
class SockAddr final
{
	static constexpr size_t MAX_ADDRLEN = 2048;

public:
	explicit SockAddr (Mono_Posix__SockaddrHeader* address) noexcept
	{
		if (address == nullptr || !get_addrlen (address, addrlen)) {
			return;
		}

		if (address->type == Mono_Posix_SockaddrType_SockaddrStorage) {
			// ugly...
			addr = reinterpret_cast<sockaddr*> (reinterpret_cast<Mono_Posix__SockaddrDynamic*>(address)->data);
		} else if (address->type == Mono_Posix_SockaddrType_SockaddrUn) {
			/* Use local_storage for up to 2048 bytes, use malloc() otherwise */
			need_free = addrlen > MAX_ADDRLEN;
			addr = reinterpret_cast<sockaddr*>(need_free ? malloc (addrlen) : local_storage);
		} else {
			addr = reinterpret_cast<sockaddr*>(local_storage);
		}

		valid = addr != nullptr;
	}

	~SockAddr ()
	{
		if (!need_free) {
			return;
		}

		free (addr);
	}

	socklen_t address_length () const noexcept
	{
		return addrlen;
	}

	sockaddr* to_sockaddr () const noexcept
	{
		return addr;
	}

	bool is_valid () const noexcept
	{
		return valid;
	}

private:
	bool       need_free = false;
	bool       valid = false;
	sockaddr  *addr = nullptr;
	socklen_t  addrlen = 0;
	uint8_t    local_storage[MAX_ADDRLEN];
};

int
Mono_Posix_Syscall_bind (int socket, struct Mono_Posix__SockaddrHeader* address)
{
	SockAddr sock (address);
	if (!sock.is_valid ()) {
		return -1;
	}

	if (Mono_Posix_FromSockaddr (address, sock.to_sockaddr ()) != 0) {
		return -1;
	}

	return bind (socket, sock.to_sockaddr (), sock.address_length ());
}

int
Mono_Posix_Syscall_connect (int socket, struct Mono_Posix__SockaddrHeader* address)
{
	SockAddr sock (address);
	if (!sock.is_valid ()) {
		return -1;
	}

	if (Mono_Posix_FromSockaddr (address, sock.to_sockaddr ()) != 0) {
		return -1;
	}

	return connect (socket, sock.to_sockaddr (), sock.address_length ());
}

int
Mono_Posix_Syscall_accept (int socket, struct Mono_Posix__SockaddrHeader* address)
{
	SockAddr sock (address);
	if (!sock.is_valid ()) {
		return -1;
	}

	socklen_t addrlen = sock.address_length ();
	int r = accept (socket, sock.to_sockaddr (), &addrlen);

	if (r != -1 && Mono_Posix_ToSockaddr (sock.to_sockaddr (), sock.address_length (), address) != 0) {
		close (r);
		r = -1;
	}

	return r;
}

#ifdef HAVE_ACCEPT4
int
Mono_Posix_Syscall_accept4 (int socket, struct Mono_Posix__SockaddrHeader* address, int flags)
{
	SockAddr sock (address);
	if (!sock.is_valid ()) {
		return -1;
	}

	socklen_t addrlen = sock.address_length ();
	int r = accept4 (socket, sock.to_sockaddr (), &addrlen, flags);

	if (r != -1 && Mono_Posix_ToSockaddr (sock.to_sockaddr (), sock.address_length (), address) != 0) {
		close (r);
		r = -1;
	}

	return r;
}
#endif

int
Mono_Posix_Syscall_getpeername (int socket, struct Mono_Posix__SockaddrHeader* address)
{
	SockAddr sock (address);
	if (!sock.is_valid ()) {
		return -1;
	}

	socklen_t addrlen = sock.address_length ();
	int r = getpeername (socket, sock.to_sockaddr (), &addrlen);

	if (r != -1 && Mono_Posix_ToSockaddr (sock.to_sockaddr (), sock.address_length (), address) != 0) {
		r = -1;
	}

	return r;
}

int
Mono_Posix_Syscall_getsockname (int socket, struct Mono_Posix__SockaddrHeader* address)
{
	SockAddr sock (address);
	if (!sock.is_valid ()) {
		return -1;
	}

	socklen_t addrlen = sock.address_length ();
	int r = getsockname (socket, sock.to_sockaddr (), &addrlen);

	if (r != -1 && Mono_Posix_ToSockaddr (sock.to_sockaddr (), sock.address_length (), address) != 0) {
		r = -1;
	}

	return r;
}

int64_t
Mono_Posix_Syscall_recv (int socket, void* message, uint64_t length, int flags)
{
	mph_return_if_size_t_overflow (length);

	return recv (socket, message, length, flags);
}

int64_t
Mono_Posix_Syscall_send (int socket, void* message, uint64_t length, int flags)
{
	mph_return_if_size_t_overflow (length);

	return send (socket, message, length, flags);
}

int64_t
Mono_Posix_Syscall_recvfrom (int socket, void* buffer, uint64_t length, int flags, struct Mono_Posix__SockaddrHeader* address)
{
	mph_return_if_size_t_overflow (length);

	SockAddr sock (address);
	if (!sock.is_valid ()) {
		return -1;
	}

	socklen_t addrlen = sock.address_length ();
	int r = recvfrom (socket, buffer, length, flags, sock.to_sockaddr (), &addrlen);

	if (r != -1 && Mono_Posix_ToSockaddr (sock.to_sockaddr (), sock.address_length (), address) != 0) {
		r = -1;
	}

	return r;
}

int64_t
Mono_Posix_Syscall_sendto (int socket, void* message, uint64_t length, int flags, struct Mono_Posix__SockaddrHeader* address)
{
	mph_return_if_size_t_overflow (length);

	SockAddr sock (address);
	if (!sock.is_valid ()) {
		return -1;
	}

	if (Mono_Posix_FromSockaddr (address, sock.to_sockaddr ()) != 0) {
		return -1;
	}

	return sendto (socket, message, length, flags, sock.to_sockaddr (), sock.address_length ());
}

int64_t
Mono_Posix_Syscall_recvmsg (int socket, struct Mono_Posix_Syscall__Msghdr* message, struct Mono_Posix__SockaddrHeader* address, int flags)
{
	SockAddr sock (address);
	if (!sock.is_valid ()) {
		return -1;
	}

	struct msghdr hdr{};

	hdr.msg_name = sock.to_sockaddr ();
	hdr.msg_namelen = sock.address_length ();
	hdr.msg_iovlen = message->msg_iovlen;
	hdr.msg_control = message->msg_control;
	hdr.msg_controllen = message->msg_controllen;

	hdr.msg_iov = _mph_from_iovec_array (message->msg_iov, message->msg_iovlen);

	int r = recvmsg (socket, &hdr, flags);

	if (r != -1 && Mono_Posix_ToSockaddr (sock.to_sockaddr (), hdr.msg_namelen, address) != 0) {
		r = -1;
	}

	free (hdr.msg_iov);

	message->msg_controllen = hdr.msg_controllen;
	message->msg_flags = hdr.msg_flags;

	return r;
}

int64_t
Mono_Posix_Syscall_sendmsg (int socket, struct Mono_Posix_Syscall__Msghdr* message, struct Mono_Posix__SockaddrHeader* address, int flags)
{
	SockAddr sock (address);
	if (!sock.is_valid ()) {
		return -1;
	}

	if (Mono_Posix_FromSockaddr (address, sock.to_sockaddr ()) != 0) {
		return -1;
	}

	struct msghdr hdr{};

	hdr.msg_name = sock.to_sockaddr ();
	hdr.msg_namelen = sock.address_length ();
	hdr.msg_iovlen = message->msg_iovlen;
	hdr.msg_control = message->msg_control;
	hdr.msg_controllen = message->msg_controllen;

	hdr.msg_iov = _mph_from_iovec_array (message->msg_iov, message->msg_iovlen);

	int r = sendmsg (socket, &hdr, flags);

	free (hdr.msg_iov);
	return r;
}

static void make_msghdr (struct msghdr* hdr, unsigned char* msg_control, int64_t msg_controllen)
{
	memset (hdr, 0, sizeof (struct msghdr));
	hdr->msg_control = msg_control;
	hdr->msg_controllen = msg_controllen;
}

static struct cmsghdr* from_offset (unsigned char* msg_control, int64_t offset)
{
	if (offset == -1)
		return nullptr;
	return reinterpret_cast<struct cmsghdr*>(msg_control + offset);
}

static int64_t to_offset (unsigned char* msg_control, void* hdr)
{
	if (!hdr)
		return -1;
	return ((unsigned char*) hdr) - msg_control;
}

#ifdef CMSG_FIRSTHDR
int64_t
Mono_Posix_Syscall_CMSG_FIRSTHDR (unsigned char* msg_control, int64_t msg_controllen)
{
	struct msghdr hdr;

	make_msghdr (&hdr, msg_control, msg_controllen);
	return to_offset (msg_control, CMSG_FIRSTHDR (&hdr));
}
#endif

#ifdef CMSG_NXTHDR
int64_t
Mono_Posix_Syscall_CMSG_NXTHDR (unsigned char* msg_control, int64_t msg_controllen, int64_t cmsg)
{
	struct msghdr hdr;

	make_msghdr (&hdr, msg_control, msg_controllen);
	return to_offset (msg_control, CMSG_NXTHDR (&hdr, from_offset (msg_control, cmsg)));
}
#endif

#ifdef CMSG_DATA
int64_t
Mono_Posix_Syscall_CMSG_DATA (unsigned char* msg_control, int64_t msg_controllen, int64_t cmsg)
{
	(void)msg_controllen;
	return to_offset (msg_control, CMSG_DATA (from_offset (msg_control, cmsg)));
}
#endif

#ifdef CMSG_ALIGN
uint64_t
Mono_Posix_Syscall_CMSG_ALIGN (uint64_t length)
{
	return CMSG_ALIGN (length);
}
#endif

#ifdef CMSG_SPACE
uint64_t
Mono_Posix_Syscall_CMSG_SPACE (uint64_t length)
{
	return CMSG_SPACE (length);
}
#endif

#ifdef CMSG_LEN
uint64_t
Mono_Posix_Syscall_CMSG_LEN (uint64_t length)
{
	return CMSG_LEN (length);
}
#endif

/*
 * vim: noexpandtab
 */

// vim: noexpandtab
// Local Variables: 
// tab-width: 4
// c-basic-offset: 4
// indent-tabs-mode: t
// End: 