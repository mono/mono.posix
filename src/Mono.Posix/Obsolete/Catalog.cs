using System;

namespace Mono.Posix
{
	[Obsolete ("Use Mono.Unix.Catalog", true)]
	public class Catalog
	{
		public static void Init (String package, String localedir)
		{
			throw new NotImplementedException ();
		}

		public static string GetString (String s)
		{
			throw new NotImplementedException ();
		}

		public static string GetPluralString (String s, String p, Int32 n)
		{
			throw new NotImplementedException ();
		}
	}
}
