using System;
using System.Net.Sockets;

namespace Mono.Posix
{
	[Obsolete ("Use Mono.Unix.PeerCred", true)]
	public class PeerCred
	{
		public int ProcessID => throw new NotImplementedException ();
		public int UserID => throw new NotImplementedException ();
		public int GroupID => throw new NotImplementedException ();

		public PeerCred (Socket sock)
		{
			throw new NotImplementedException ();
		}
	}
}
