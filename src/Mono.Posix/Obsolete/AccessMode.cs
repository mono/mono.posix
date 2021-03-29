using System;

namespace Mono.Posix
{
	[Flags]
	[Obsolete ("Use Mono.Unix.Native.AccessModes", true)]
	public enum AccessMode
	{
		[Obsolete ("Use Mono.Unix.Native.AccessModes.R_OK", true)]
		R_OK = 1,

		[Obsolete ("Use Mono.Unix.Native.AccessModes.W_OK", true)]
		W_OK = 2,

		[Obsolete ("Use Mono.Unix.Native.AccessModes.X_OK", true)]
		X_OK = 4,

		[Obsolete ("Use Mono.Unix.Native.AccessModes.F_OK", true)]
		F_OK = 8
	}
}
