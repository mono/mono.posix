using System;

namespace Mono.Posix
{
	[Obsolete ("Use Mono.Unix.Native.Stat", true)]
	public struct Stat
	{
		[Obsolete ("Use Mono.Unix.Native.Stat.st_dev", true)]
		public readonly int Device;

		[Obsolete ("Use Mono.Unix.Native.Stat.st_ino", true)]
		public readonly int INode;

		[Obsolete ("Use Mono.Unix.Native.Stat.st_mode", true)]
		public readonly StatMode Mode;

		[Obsolete ("Use Mono.Unix.Native.Stat.st_nlink", true)]
		public readonly int NLinks;

		[Obsolete ("Use Mono.Unix.Native.Stat.st_uid", true)]
		public readonly int Uid;

		[Obsolete ("Use Mono.Unix.Native.Stat.st_gid", true)]
		public readonly int Gid;

		[Obsolete ("Use Mono.Unix.Native.Stat.st_rdev", true)]
		public readonly long DeviceType;

		[Obsolete ("Use Mono.Unix.Native.Stat.st_size", true)]
		public readonly long Size;

		[Obsolete ("Use Mono.Unix.Native.Stat.st_blksize", true)]
		public readonly long BlockSize;

		[Obsolete ("Use Mono.Unix.Native.Stat.st_blocks", true)]
		public readonly long Blocks;

		[Obsolete ("Use Mono.Unix.Native.Stat.st_atime", true)]
		public readonly DateTime ATime;

		[Obsolete ("Use Mono.Unix.Native.Stat.st_mtime", true)]
		public readonly DateTime MTime;

		[Obsolete ("Use Mono.Unix.Native.Stat.st_ctime", true)]
		public readonly DateTime CTime;

		[Obsolete ("Use Mono.Unix.Native.NativeConvert.LocalUnixEpoch", true)]
		public static readonly DateTime UnixEpoch = new DateTime (1970, 1, 1);

		[Obsolete ("Use Mono.Unix.Native.NativeConvert.ToDateTime", true)]
		public static DateTime UnixToDateTime (long unix)
		{
			throw new NotImplementedException ();
		}
	}
}
