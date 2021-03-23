namespace Mono.Posix
{
	static class Utilities
	{
		public static int SafeGetHashCode (string? s)
		{
			return s?.GetHashCode () ?? 0;
		}
	}
}
