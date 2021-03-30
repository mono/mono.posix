/*
 * This file was automatically generated by make-map from Mono.Unix.dll.
 *
 * DO NOT MODIFY.
 */

using System;
using System.IO;
using System.Runtime.InteropServices;
using Mono.Unix.Native;
#if MONODROID
using Mono.Unix.Android;
#endif

namespace Mono.Unix.Native {

	[CLSCompliant (false)]
	public sealed /* static */ partial class NativeConvert
	{
		//
		// Non-generated exports
		//
#if MONODROID
		[DllImport (LIB, EntryPoint="Mono_Posix_FromRealTimeSignum")]
		static extern int HelperFromRealTimeSignum (Int32 offset, out Int32 rval);

		static int FromRealTimeSignum (Int32 offset, out Int32 rval)
		{
			if (!AndroidUtils.AreRealTimeSignalsSafe ())
				throw new PlatformNotSupportedException ("Real-time signals are not supported on this Android architecture");
			return HelperFromRealTimeSignum (offset, out rval);
		}
#else
		[DllImport (LIB, EntryPoint="Mono_Posix_FromRealTimeSignum")]
		private static extern int FromRealTimeSignum (Int32 offset, out Int32 rval);
#endif
		// convert a realtime signal to os signal
		public static int FromRealTimeSignum (RealTimeSignum sig)
		{
			int sigNum;
			if (FromRealTimeSignum (sig.Offset, out sigNum) == -1)
				ThrowArgumentException (sig.Offset);
			return sigNum;
		}

		// convert an offset to an rt signum
		public static RealTimeSignum ToRealTimeSignum (int offset)
		{
			return new RealTimeSignum (offset);
		}

		// convert from octal representation.
		public static FilePermissions FromOctalPermissionString (string value)
		{
			uint n = Convert.ToUInt32 (value, 8);
			return ToFilePermissions (n);
		}

		public static string ToOctalPermissionString (FilePermissions value)
		{
			string s = Convert.ToString ((int) (value & ~FilePermissions.S_IFMT), 8);
			return new string ('0', 4-s.Length) + s;
		}

		public static FilePermissions FromUnixPermissionString (string value)
		{
			if (value == null)
				throw new ArgumentNullException ("value");
			if (value.Length != 9 && value.Length != 10)
				throw new ArgumentException ("value", "must contain 9 or 10 characters");

			int i = 0;
			FilePermissions perms = new FilePermissions ();

			if (value.Length == 10) {
				perms |= GetUnixPermissionDevice (value [i]);
				++i;
			}

			perms |= GetUnixPermissionGroup (
				value [i++], FilePermissions.S_IRUSR,
				value [i++], FilePermissions.S_IWUSR,
				value [i++], FilePermissions.S_IXUSR,
				's', 'S', FilePermissions.S_ISUID);

			perms |= GetUnixPermissionGroup (
				value [i++], FilePermissions.S_IRGRP,
				value [i++], FilePermissions.S_IWGRP,
				value [i++], FilePermissions.S_IXGRP,
				's', 'S', FilePermissions.S_ISGID);

			perms |= GetUnixPermissionGroup (
				value [i++], FilePermissions.S_IROTH,
				value [i++], FilePermissions.S_IWOTH,
				value [i++], FilePermissions.S_IXOTH,
				't', 'T', FilePermissions.S_ISVTX);

			return perms;
		}

		private static FilePermissions GetUnixPermissionDevice (char value)
		{
			switch (value) {
			case 'd': return FilePermissions.S_IFDIR;
			case 'c': return FilePermissions.S_IFCHR;
			case 'b': return FilePermissions.S_IFBLK;
			case '-': return FilePermissions.S_IFREG;
			case 'p': return FilePermissions.S_IFIFO;
			case 'l': return FilePermissions.S_IFLNK;
			case 's': return FilePermissions.S_IFSOCK;
			}
			throw new ArgumentException ("value", "invalid device specification: " + 
				value);
		}

		private static FilePermissions GetUnixPermissionGroup (
			char read, FilePermissions readb, 
			char write, FilePermissions writeb, 
			char exec, FilePermissions execb,
			char xboth, char xbitonly, FilePermissions xbit)
		{
			FilePermissions perms = new FilePermissions ();
			if (read == 'r')
				perms |= readb;
			if (write == 'w')
				perms |= writeb;
			if (exec == 'x')
				perms |= execb;
			else if (exec == xbitonly)
				perms |= xbit;
			else if (exec == xboth)
				perms |= (execb | xbit);
			return perms;
		}

		// Create ls(1) drwxrwxrwx permissions display
		public static string ToUnixPermissionString (FilePermissions value)
		{
			char [] access = new char[] {
				'-',            // device
				'-', '-', '-',  // owner
				'-', '-', '-',  // group
				'-', '-', '-',  // other
			};
			bool have_device = true;
			switch (value & FilePermissions.S_IFMT) {
				case FilePermissions.S_IFDIR:   access [0] = 'd'; break;
				case FilePermissions.S_IFCHR:   access [0] = 'c'; break;
				case FilePermissions.S_IFBLK:   access [0] = 'b'; break;
				case FilePermissions.S_IFREG:   access [0] = '-'; break;
				case FilePermissions.S_IFIFO:   access [0] = 'p'; break;
				case FilePermissions.S_IFLNK:   access [0] = 'l'; break;
				case FilePermissions.S_IFSOCK:  access [0] = 's'; break;
				default:                        have_device = false; break;
			}
			SetUnixPermissionGroup (value, access, 1, 
				FilePermissions.S_IRUSR, FilePermissions.S_IWUSR, FilePermissions.S_IXUSR,
				's', 'S', FilePermissions.S_ISUID);
			SetUnixPermissionGroup (value, access, 4, 
				FilePermissions.S_IRGRP, FilePermissions.S_IWGRP, FilePermissions.S_IXGRP,
				's', 'S', FilePermissions.S_ISGID);
			SetUnixPermissionGroup (value, access, 7, 
				FilePermissions.S_IROTH, FilePermissions.S_IWOTH, FilePermissions.S_IXOTH,
				't', 'T', FilePermissions.S_ISVTX);
			return have_device 
				? new string (access)
				: new string (access, 1, 9);
		}

		private static void SetUnixPermissionGroup (FilePermissions value,
			char[] access, int index,
			FilePermissions read, FilePermissions write, FilePermissions exec,
			char both, char setonly, FilePermissions setxbit)
		{
			if (UnixFileSystemInfo.IsSet (value, read))
				access [index] = 'r';
			if (UnixFileSystemInfo.IsSet (value, write))
				access [index+1] = 'w';
			access [index+2] = GetSymbolicMode (value, exec, both, setonly, setxbit);
		}

		// Implement the GNU ls(1) permissions spec; see `info coreutils ls`,
		// section 10.1.2, the `-l' argument information.
		private static char GetSymbolicMode (FilePermissions value, 
			FilePermissions xbit, char both, char setonly, FilePermissions setxbit)
		{
			bool is_x  = UnixFileSystemInfo.IsSet (value, xbit);
			bool is_sx = UnixFileSystemInfo.IsSet (value, setxbit);
			
			if (is_x && is_sx)
				return both;
			if (is_sx)
				return setonly;
			if (is_x)
				return 'x';
			return '-';
		}

		public static readonly DateTime UnixEpoch =
			new DateTime (year:1970, month:1, day:1, hour:0, minute:0, second:0, kind:DateTimeKind.Utc);
		public static readonly DateTime LocalUnixEpoch = 
			new DateTime (1970, 1, 1);
		public static readonly TimeSpan LocalUtcOffset = 
			TimeZoneInfo.Local.GetUtcOffset (DateTime.UtcNow);

		public static DateTime ToDateTime (long time)
		{
			return FromTimeT (time);
		}

		public static DateTime ToDateTime (long time, long nanoTime)
		{
			return FromTimeT (time).AddMilliseconds (nanoTime / 1000);
		}

		public static long FromDateTime (DateTime time)
		{
			return ToTimeT (time);
		}

		public static DateTime FromTimeT (long time)
		{
			return UnixEpoch.AddSeconds (time).ToLocalTime ();
		}

		public static long ToTimeT (DateTime time)
		{
			if (time.Kind == DateTimeKind.Unspecified)
				throw new ArgumentException ("DateTimeKind.Unspecified is not supported. Use Local or Utc times.", "time");

			if (time.Kind == DateTimeKind.Local)
				time = time.ToUniversalTime ();

			return (long) (time - UnixEpoch).TotalSeconds;
		}

		public static OpenFlags ToOpenFlags (FileMode mode, FileAccess access)
		{
			OpenFlags flags = 0;
			switch (mode) {
			case FileMode.CreateNew:
				flags = OpenFlags.O_CREAT | OpenFlags.O_EXCL;
				break;
			case FileMode.Create:
				flags = OpenFlags.O_CREAT | OpenFlags.O_TRUNC;
				break;
			case FileMode.Open:
				// do nothing
				break;
			case FileMode.OpenOrCreate:
				flags = OpenFlags.O_CREAT;
				break;
			case FileMode.Truncate:
				flags = OpenFlags.O_TRUNC;
				break;
			case FileMode.Append:
				flags = OpenFlags.O_APPEND;
				break;
			default:
				throw new ArgumentException (Locale.GetText ("Unsupported mode value"), "mode");
			}

			// Is O_LARGEFILE supported?
			int _v;
			if (TryFromOpenFlags (OpenFlags.O_LARGEFILE, out _v))
				flags |= OpenFlags.O_LARGEFILE;

			switch (access) {
			case FileAccess.Read:
				flags |= OpenFlags.O_RDONLY;
				break;
			case FileAccess.Write:
				flags |= OpenFlags.O_WRONLY;
				break;
			case FileAccess.ReadWrite:
				flags |= OpenFlags.O_RDWR;
				break;
			default:
				throw new ArgumentOutOfRangeException (Locale.GetText ("Unsupported access value"), "access");
			}

			return flags;
		}

		public static string ToFopenMode (FileAccess access)
		{
			switch (access) {
				case FileAccess.Read:       return "rb";
				case FileAccess.Write:      return "wb";
				case FileAccess.ReadWrite:  return "r+b";
				default:                    throw new ArgumentOutOfRangeException ("access");
			}
		}

		public static string ToFopenMode (FileMode mode)
		{
			switch (mode) {
				case FileMode.CreateNew: case FileMode.Create:        return "w+b";
				case FileMode.Open:      case FileMode.OpenOrCreate:  return "r+b";
				case FileMode.Truncate: return "w+b";
				case FileMode.Append:   return "a+b";
				default:                throw new ArgumentOutOfRangeException ("mode");
			}
		}

		private static readonly string[][] fopen_modes = new string[][]{
			//                                         Read                       Write ReadWrite
			/*    FileMode.CreateNew: */  new string[]{"Can't Read+Create",       "wb", "w+b"},
			/*       FileMode.Create: */  new string[]{"Can't Read+Create",       "wb", "w+b"},
			/*         FileMode.Open: */  new string[]{"rb",                      "wb", "r+b"},
			/* FileMode.OpenOrCreate: */  new string[]{"rb",                      "wb", "r+b"},
			/*     FileMode.Truncate: */  new string[]{"Cannot Truncate and Read","wb", "w+b"},
			/*       FileMode.Append: */  new string[]{"Cannot Append and Read",  "ab", "a+b"},
		};

		public static string ToFopenMode (FileMode mode, FileAccess access)
		{
			int fm = -1, fa = -1;
			switch (mode) {
				case FileMode.CreateNew:    fm = 0; break;
				case FileMode.Create:       fm = 1; break;
				case FileMode.Open:         fm = 2; break;
				case FileMode.OpenOrCreate: fm = 3; break;
				case FileMode.Truncate:     fm = 4; break;
				case FileMode.Append:       fm = 5; break;
			}
			switch (access) {
				case FileAccess.Read:       fa = 0; break;
				case FileAccess.Write:      fa = 1; break;
				case FileAccess.ReadWrite:  fa = 2; break;
			}

			if (fm == -1)
				throw new ArgumentOutOfRangeException ("mode");
			if (fa == -1)
				throw new ArgumentOutOfRangeException ("access");

			string fopen_mode = fopen_modes [fm][fa];
			if (fopen_mode [0] != 'r' && fopen_mode [0] != 'w' && fopen_mode [0] != 'a')
				throw new ArgumentException (fopen_mode);
			return fopen_mode;
		}

		[DllImport (LIB, EntryPoint="Mono_Posix_FromMremapFlags")]
		private static extern int FromMremapFlags (MremapFlags value, out UInt64 rval);

		public static bool TryFromMremapFlags (MremapFlags value, out UInt64 rval)
		{
			return FromMremapFlags (value, out rval) == 0;
		}

		public static UInt64 FromMremapFlags (MremapFlags value)
		{
			UInt64 rval;
			if (FromMremapFlags (value, out rval) == -1)
				ThrowArgumentException (value);
			return rval;
		}

		[DllImport (LIB, EntryPoint="Mono_Posix_ToMremapFlags")]
		private static extern int ToMremapFlags (UInt64 value, out MremapFlags rval);

		public static bool TryToMremapFlags (UInt64 value, out MremapFlags rval)
		{
			return ToMremapFlags (value, out rval) == 0;
		}

		public static MremapFlags ToMremapFlags (UInt64 value)
		{
			MremapFlags rval;
			if (ToMremapFlags (value, out rval) == -1)
				ThrowArgumentException (value);
			return rval;
		}

		[DllImport (LIB, EntryPoint="Mono_Posix_FromStat")]
		private static extern int FromStat (ref Stat source, IntPtr destination);

		public static bool TryCopy (ref Stat source, IntPtr destination)
		{
			return FromStat (ref source, destination) == 0;
		}

		[DllImport (LIB, EntryPoint="Mono_Posix_ToStat")]
		private static extern int ToStat (IntPtr source, out Stat destination);

		public static bool TryCopy (IntPtr source, out Stat destination)
		{
			return ToStat (source, out destination) == 0;
		}

		[DllImport (LIB, EntryPoint="Mono_Posix_FromStatvfs")]
		private static extern int FromStatvfs (ref Statvfs source, IntPtr destination);

		public static bool TryCopy (ref Statvfs source, IntPtr destination)
		{
			return FromStatvfs (ref source, destination) == 0;
		}

		[DllImport (LIB, EntryPoint="Mono_Posix_ToStatvfs")]
		private static extern int ToStatvfs (IntPtr source, out Statvfs destination);

		public static bool TryCopy (IntPtr source, out Statvfs destination)
		{
			return ToStatvfs (source, out destination) == 0;
		}

		[DllImport (LIB, EntryPoint="Mono_Posix_FromInAddr")]
		private static extern int FromInAddr (ref InAddr source, IntPtr destination);

		public static bool TryCopy (ref InAddr source, IntPtr destination)
		{
			return FromInAddr (ref source, destination) == 0;
		}

		[DllImport (LIB, EntryPoint="Mono_Posix_ToInAddr")]
		private static extern int ToInAddr (IntPtr source, out InAddr destination);

		public static bool TryCopy (IntPtr source, out InAddr destination)
		{
			return ToInAddr (source, out destination) == 0;
		}

		[DllImport (LIB, EntryPoint="Mono_Posix_FromIn6Addr")]
		private static extern int FromIn6Addr (ref In6Addr source, IntPtr destination);

		public static bool TryCopy (ref In6Addr source, IntPtr destination)
		{
			return FromIn6Addr (ref source, destination) == 0;
		}

		[DllImport (LIB, EntryPoint="Mono_Posix_ToIn6Addr")]
		private static extern int ToIn6Addr (IntPtr source, out In6Addr destination);

		public static bool TryCopy (IntPtr source, out In6Addr destination)
		{
			return ToIn6Addr (source, out destination) == 0;
		}

		public static InAddr ToInAddr (System.Net.IPAddress address)
		{
			if (address == null)
				throw new ArgumentNullException ("address");
			if (address.AddressFamily != System.Net.Sockets.AddressFamily.InterNetwork)
				throw new ArgumentException ("address", "address.AddressFamily != System.Net.Sockets.AddressFamily.InterNetwork");
			return new InAddr (address.GetAddressBytes ());
		}

		public static System.Net.IPAddress ToIPAddress (InAddr address)
		{
			var bytes = new byte[4];
			address.CopyTo (bytes, 0);
			return new System.Net.IPAddress (bytes);
		}

		public static In6Addr ToIn6Addr (System.Net.IPAddress address)
		{
			if (address == null)
				throw new ArgumentNullException ("address");
			if (address.AddressFamily != System.Net.Sockets.AddressFamily.InterNetworkV6)
				throw new ArgumentException ("address", "address.AddressFamily != System.Net.Sockets.AddressFamily.InterNetworkV6");
			return new In6Addr (address.GetAddressBytes ());
		}

		public static System.Net.IPAddress ToIPAddress (In6Addr address)
		{
			var bytes = new byte[16];
			address.CopyTo (bytes, 0);
			return new System.Net.IPAddress (bytes);
		}

		[DllImport (LIB, EntryPoint="Mono_Posix_FromSockaddr")]
		private static extern unsafe int FromSockaddr (_SockaddrHeader* source, IntPtr destination);

		public static unsafe bool TryCopy (Sockaddr source, IntPtr destination)
		{
			byte[]? array = Sockaddr.GetDynamicData (source);
			if (array == null) {
				return false;
			}

			// SockaddrStorage has to be handled extra because the native code assumes that SockaddrStorage input is used in-place
			if (source.type == (SockaddrType.SockaddrStorage | SockaddrType.MustBeWrapped)) {
				Marshal.Copy (array, 0, destination, (int) source.GetDynamicLength ());
				return true;
			}
			fixed (SockaddrType* addr = &Sockaddr.GetAddress (source).type)
			fixed (byte* data = array) {
				var dyn = new _SockaddrDynamic (source, data, useMaxLength: false);
				return FromSockaddr (Sockaddr.GetNative (&dyn, addr), destination) == 0;
			}
		}

		[DllImport (LIB, EntryPoint="Mono_Posix_ToSockaddr")]
		private static extern unsafe int ToSockaddr (IntPtr source, long size, _SockaddrHeader* destination);

		public static unsafe bool TryCopy (IntPtr source, long size, Sockaddr destination)
		{
			if (destination == null)
				throw new ArgumentNullException ("destination");
			byte[]? array = Sockaddr.GetDynamicData (destination);

			fixed (SockaddrType* addr = &Sockaddr.GetAddress (destination).type)
			fixed (byte* data = Sockaddr.GetDynamicData (destination)) {
				var dyn = new _SockaddrDynamic (destination, data, useMaxLength: true);
				var r = ToSockaddr (source, size, Sockaddr.GetNative (&dyn, addr));
				dyn.Update (destination);
				// SockaddrStorage has to be handled extra because the native code assumes that SockaddrStorage input is used in-place
				if (r == 0 && destination.type == (SockaddrType.SockaddrStorage | SockaddrType.MustBeWrapped)) {
					Marshal.Copy (source, array, 0, (int) destination.GetDynamicLength ());
				}
				return r == 0;
			}
		}
	}
}

// vim: noexpandtab
// Local Variables: 
// tab-width: 4
// c-basic-offset: 4
// indent-tabs-mode: t
// End: 