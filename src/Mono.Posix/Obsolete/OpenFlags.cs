using System;

namespace Mono.Posix
{
	[Flags]
	[Obsolete ("Use Mono.Unix.Native.OpenFlags", true)]
	public enum OpenFlags
	{
		[Obsolete ("Use Mono.Unix.Native.OpenFlags.O_RDONLY", true)]
		O_RDONLY    = 0,

		[Obsolete ("Use Mono.Unix.Native.OpenFlags.O_WDONLY", true)]
		O_WRONLY    = 1,

		[Obsolete ("Use Mono.Unix.Native.OpenFlags.O_RDWR", true)]
		O_RDWR      = 2,

		[Obsolete ("Use Mono.Unix.Native.OpenFlags.O_CREAT", true)]
		O_CREAT     = 4,

		[Obsolete ("Use Mono.Unix.Native.OpenFlags.O_EXCL", true)]
		O_EXCL      = 8,

		[Obsolete ("Use Mono.Unix.Native.OpenFlags.O_NOCTTY", true)]
		O_NOCTTY    = 16,

		[Obsolete ("Use Mono.Unix.Native.OpenFlags.O_TRUNC", true)]
		O_TRUNC     = 32,

		[Obsolete ("Use Mono.Unix.Native.OpenFlags.O_APPEND", true)]
		O_APPEND    = 64,

		[Obsolete ("Use Mono.Unix.Native.OpenFlags.O_NONBLOCK", true)]
		O_NONBLOCK  = 128,

		[Obsolete ("Use Mono.Unix.Native.OpenFlags.O_SYNC", true)]
		O_SYNC      = 256,
	}
}
