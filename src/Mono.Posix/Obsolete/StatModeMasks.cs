using System;

namespace Mono.Posix
{
	[Obsolete ("Use Mono.Unix.Native.FilePermissions", true)]
	public enum StatModeMasks
	{
		[Obsolete ("Use Mono.Unix.Native.FilePermissions.S_IFMT", true)]
		TypeMask = 0xF000, // bitmask for the file type bitfields

		[Obsolete ("Use Mono.Unix.Native.FilePermissions.S_RWXU", true)]
		OwnerMask = 0x1C0, // mask for file owner permissions

		[Obsolete ("Use Mono.Unix.Native.FilePermissions.S_RWXG", true)]
		GroupMask = 0x38, // mask for group permissions

		[Obsolete ("Use Mono.Unix.Native.FilePermissions.S_RWXO", true)]
		OthersMask = 0x7, // mask for permissions for others (not in group)
	}
}
