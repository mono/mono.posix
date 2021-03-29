using System;

namespace Mono.Posix
{
	[Flags]
	[Obsolete ("Use Mono.Unix.Native.WaitOptions", true)]
	public enum WaitOptions
	{
		[Obsolete ("Use Mono.Unix.Native.WaitOptions.WNOHANG", true)]
		WNOHANG,

		[Obsolete ("Use Mono.Unix.Native.WaitOptions.WUNTRACED", true)]
		WUNTRACED
	}
}
