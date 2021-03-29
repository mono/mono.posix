using System;

namespace Mono.Posix
{
	[Flags]
	[Obsolete ("Use Mono.Unix.Native.FilePermissions", true)]
	public enum StatMode
	{
		[Obsolete ("Use Mono.Unix.Native.FilePermissions.S_IFSOCK", true)]
		Socket = 0xC000, // socket

		[Obsolete ("Use Mono.Unix.Native.FilePermissions.S_IFLNK", true)]
		SymLink = 0xA000, // symbolic link

		[Obsolete ("Use Mono.Unix.Native.FilePermissions.S_IFREG", true)]
		Regular = 0x8000, // regular file

		[Obsolete ("Use Mono.Unix.Native.FilePermissions.S_IFBLK", true)]
		BlockDevice = 0x6000, // block device

		[Obsolete ("Use Mono.Unix.Native.FilePermissions.S_IFDIR", true)]
		Directory = 0x4000, // directory

		[Obsolete ("Use Mono.Unix.Native.FilePermissions.S_IFCHR", true)]
		CharDevice = 0x2000, // character device

		[Obsolete ("Use Mono.Unix.Native.FilePermissions.S_IFIFO", true)]
		FIFO = 0x1000, // fifo

		[Obsolete ("Use Mono.Unix.Native.FilePermissions.S_ISUID", true)]
		SUid = 0x800, // set UID bit

		[Obsolete ("Use Mono.Unix.Native.FilePermissions.S_ISGID", true)]
		SGid = 0x400, // set GID bit

		[Obsolete ("Use Mono.Unix.Native.FilePermissions.S_ISVTX", true)]
		Sticky = 0x200, // sticky bit

		[Obsolete ("Use Mono.Unix.Native.FilePermissions.S_IRUSR", true)]
		OwnerRead = 0x100, // owner has read permission

		[Obsolete ("Use Mono.Unix.Native.FilePermissions.S_IWUSR", true)]
		OwnerWrite = 0x80, // owner has write permission

		[Obsolete ("Use Mono.Unix.Native.FilePermissions.S_IXUSR", true)]
		OwnerExecute = 0x40, // owner has execute permission

		[Obsolete ("Use Mono.Unix.Native.FilePermissions.S_IRGRP", true)]
		GroupRead = 0x20, // group has read permission

		[Obsolete ("Use Mono.Unix.Native.FilePermissions.S_IWGRP", true)]
		GroupWrite = 0x10, // group has write permission

		[Obsolete ("Use Mono.Unix.Native.FilePermissions.S_IXGRP", true)]
		GroupExecute = 0x8, // group has execute permission

		[Obsolete ("Use Mono.Unix.Native.FilePermissions.S_IROTH", true)]
		OthersRead = 0x4, // others have read permission

		[Obsolete ("Use Mono.Unix.Native.FilePermissions.S_IWOTH", true)]
		OthersWrite = 0x2, // others have write permisson

		[Obsolete ("Use Mono.Unix.Native.FilePermissions.S_IXOTH", true)]
		OthersExecute = 0x1, // others have execute permission
	}
}
