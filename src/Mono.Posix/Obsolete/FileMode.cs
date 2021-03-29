using System;

namespace Mono.Posix
{
	[Flags]
	[Obsolete ("Use Mono.Unix.Native.FilePermissions", true)]
	public enum FileMode
	{
		[Obsolete ("Use Mono.Unix.Native.FilePermissions.S_ISUID", true)]
		S_ISUID   = 2048,

		[Obsolete ("Use Mono.Unix.Native.FilePermissions.S_ISGID", true)]
		S_ISGID   = 1024,

		[Obsolete ("Use Mono.Unix.Native.FilePermissions.S_ISVTX", true)]
		S_ISVTX   = 512,

		[Obsolete ("Use Mono.Unix.Native.FilePermissions.S_IRUSR", true)]
		S_IRUSR   = 256,

		[Obsolete ("Use Mono.Unix.Native.FilePermissions.S_IWUSR", true)]
		S_IWUSR   = 128,

		[Obsolete ("Use Mono.Unix.Native.FilePermissions.S_XUSR", true)]
		S_IXUSR   = 64,

		[Obsolete ("Use Mono.Unix.Native.FilePermissions.S_IRGRP", true)]
		S_IRGRP   = 32,

		[Obsolete ("Use Mono.Unix.Native.FilePermissions.S_IWGRP", true)]
		S_IWGRP   = 16,

		[Obsolete ("Use Mono.Unix.Native.FilePermissions.S_IXGRP", true)]
		S_IXGRP   = 8,

		[Obsolete ("Use Mono.Unix.Native.FilePermissions.S_IROTH", true)]
		S_IROTH   = 4,

		[Obsolete ("Use Mono.Unix.Native.FilePermissions.S_IWOTH", true)]
		S_IWOTH   = 2,

		[Obsolete ("Use Mono.Unix.Native.FilePermissions.S_IXOTH", true)]
		S_IXOTH   = 1
	}
}
