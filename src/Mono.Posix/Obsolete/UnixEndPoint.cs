using System;
using System.Net;
using System.Net.Sockets;

namespace Mono.Posix
{
	[Serializable]
	[Obsolete ("Use Mono.Unix.UnixEndPoint", true)]
	public class UnixEndPoint : EndPoint
	{
		[Obsolete ("Use Mono.Unix.UnixEndPoint.Filename", true)]
		public string Filename {
			get => throw new NotImplementedException ();
			set => throw new NotImplementedException ();
		}

		public override AddressFamily AddressFamily => throw new NotImplementedException ();

		[Obsolete ("Use Mono.Unix.UnixEndPoint.ctor", true)]
		public UnixEndPoint (string filename)
		{
			throw new NotImplementedException ();
		}

		public override EndPoint Create (SocketAddress socketAddress)
		{
			throw new NotImplementedException ();
		}

		public override SocketAddress Serialize ()
		{
			throw new NotImplementedException ();
		}

		public override string ToString ()
		{
			throw new NotImplementedException ();
		}

		public override int GetHashCode ()
		{
			throw new NotImplementedException ();
		}

		public override bool Equals (object o)
		{
			throw new NotImplementedException ();
		}
	}
}
