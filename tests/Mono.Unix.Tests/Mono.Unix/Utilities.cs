using System;
using Mono.Unix;
using Mono.Unix.Native;

namespace MonoTests.Mono.Unix
{
	static class Utilities
	{
		public static int FindFirstFreeRTSignal ()
		{
			// Mono uses real-time signals, so we need to find the first one that's not taken before we proceeed with
			// the rest of the test
			for (int offset = RealTimeSignum.MinValue.Offset; offset <= RealTimeSignum.MaxValue.Offset; offset++) {
				try {
					UnixSignal s = new UnixSignal (new RealTimeSignum (offset));
					return offset;
				} catch (ArgumentException) {
					if (Stdlib.GetLastError () == Errno.EADDRINUSE) {
						continue;
					}

					throw;
				}
			}

			return -1;
		}
	}
}
