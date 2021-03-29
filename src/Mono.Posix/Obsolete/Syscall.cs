using System;

namespace Mono.Posix
{
	[Obsolete ("Use Mono.Unix.Native.Syscall", true)]
	public class Syscall
	{
		[Obsolete ("Use Mono.Unix.Native.Syscall.exit", true)]
		public static extern int exit (int status);

		[Obsolete ("Use Mono.Unix.Native.Syscall.fork", true)]
		public static extern int fork ();

		[Obsolete ("Use Mono.Unix.Native.Syscall.read", true)]
		public unsafe static extern IntPtr read (int fileDescriptor, void *buf, IntPtr count);

		[Obsolete ("Use Mono.Unix.Native.Syscall.write", true)]
		public unsafe static extern IntPtr write (int fileDescriptor, void *buf, IntPtr count);

		[Obsolete ("Use Mono.Unix.Native.Syscall.open", true)]
		public static int open (string pathname, OpenFlags flags)
		{
			throw new NotImplementedException ();
		}

		[Obsolete ("Use Mono.Unix.Native.Syscall.open", true)]
		public static int open (string pathname, OpenFlags flags, FileMode mode)
		{
			throw new NotImplementedException ();
		}


		[Obsolete ("Use Mono.Unix.Native.Syscall.close", true)]
		public static extern int close (int fileDescriptor);

		[Obsolete ("Use Mono.Unix.Native.Syscall.waitpid", true)]
		public static int waitpid (int pid, out int status, WaitOptions options)
		{
			throw new NotImplementedException ();
		}

		[Obsolete ("Use Mono.Unix.Native.Syscall.waitpid", true)]
		public static int waitpid (int pid, WaitOptions options)
		{
			throw new NotImplementedException ();
		}

		[Obsolete ("Use Mono.Unix.Native.Syscall.WIFEXITED", true)]
		public static extern int WIFEXITED (int status);

		[Obsolete ("Use Mono.Unix.Native.Syscall.WEXITSTATUS", true)]
		public static extern int WEXITSTATUS (int status);

		[Obsolete ("Use Mono.Unix.Native.Syscall.WIFSIGNALED", true)]
		public static extern int WIFSIGNALED (int status);

		[Obsolete ("Use Mono.Unix.Native.Syscall.WTERMSIG", true)]
		public static extern int WTERMSIG (int status);

		[Obsolete ("Use Mono.Unix.Native.Syscall.WIFSTOPPED", true)]
		public static extern int WIFSTOPPED (int status);

		[Obsolete ("Use Mono.Unix.Native.Syscall.WSTOPSIG", true)]
		public static extern int WSTOPSIG (int status);

		[Obsolete ("Use Mono.Unix.Native.Syscall.creat", true)]
		public static int creat (string pathname, FileMode flags)
		{
			throw new NotImplementedException ();
		}

		[Obsolete ("Use Mono.Unix.Native.Syscall.link", true)]
		public static extern int link (string oldPath, string newPath);

		[Obsolete ("Use Mono.Unix.Native.Syscall.unlink", true)]
		public static extern int unlink (string path);

		[Obsolete ("Use Mono.Unix.Native.Syscall.symlink", true)]
		public static extern int symlink (string oldpath, string newpath);

		[Obsolete ("Use Mono.Unix.Native.Syscall.chdir", true)]
		public static extern int chdir (string path);

		[Obsolete ("Use Mono.Unix.Native.Syscall.chmod", true)]
		public static int chmod (string path, FileMode mode)
		{
			throw new NotImplementedException ();
		}

		[Obsolete ("Use Mono.Unix.Native.Syscall.chown", true)]
		public static extern int chown (string path, int owner, int group);

		[Obsolete ("Use Mono.Unix.Native.Syscall.lchown", true)]
		public static extern int lchown (string path, int owner, int group);

		[Obsolete ("Use Mono.Unix.Native.Syscall.lseek", true)]
		public static extern int lseek (int fileDescriptor, int offset, int whence);

		[Obsolete ("Use Mono.Unix.Native.Syscall.getpid", true)]
		public static extern int getpid ();

		[Obsolete ("Use Mono.Unix.Native.Syscall.setuid", true)]
		public static extern int setuid (int uid);

		[Obsolete ("Use Mono.Unix.Native.Syscall.getuid", true)]
		public static extern int getuid ();

		[Obsolete ("Use Mono.Unix.Native.Syscall.alarm", true)]
		public static extern uint alarm (uint seconds);

		[Obsolete ("Use Mono.Unix.Native.Syscall.pause", true)]
		public static extern int pause ();

		[Obsolete ("Use Mono.Unix.Native.Syscall.access", true)]
		public static int access (string pathname, AccessMode mode)
		{
			throw new NotImplementedException ();
		}

		[Obsolete ("Use Mono.Unix.Native.Syscall.nice", true)]
		public static extern int nice (int increment);

		[Obsolete ("Use Mono.Unix.Native.Syscall.mkdir", true)]
		public static int mkdir (string pathname, FileMode mode)
		{
			throw new NotImplementedException ();
		}

		[Obsolete ("Use Mono.Unix.Native.Syscall.rmdir", true)]
		public static extern int rmdir (string path);

		[Obsolete ("Use Mono.Unix.Native.Syscall.dup", true)]
		public static extern int dup (int fileDescriptor);

		[Obsolete ("Use Mono.Unix.Native.Syscall.setgid", true)]
		public static extern int setgid (int gid);

		[Obsolete ("Use Mono.Unix.Native.Syscall.getgid", true)]
		public static extern int getgid ();

		[Obsolete ("Use Mono.Unix.Native.Syscall.sighandler_t", true)]
		public delegate void sighandler_t (int v);

		[Obsolete ("Use Mono.Unix.Native.Syscall.signal", true)]
		public static extern int signal (int signum, sighandler_t handler);

		[Obsolete ("Use Mono.Unix.Native.Syscall.geteuid", true)]
		public static extern int geteuid ();

		[Obsolete ("Use Mono.Unix.Native.Syscall.getegid", true)]
		public static extern int getegid ();

		[Obsolete ("Use Mono.Unix.Native.Syscall.setpgid", true)]
		public static extern int setpgid (int pid, int pgid);

		[Obsolete ("Use Mono.Unix.Native.Syscall.umask", true)]
		public static extern int umask (int umask);

		[Obsolete ("Use Mono.Unix.Native.Syscall.chroot", true)]
		public static extern int chroot (string path);

		[Obsolete ("Use Mono.Unix.Native.Syscall.dup2", true)]
		public static extern int dup2 (int oldFileDescriptor, int newFileDescriptor);

		[Obsolete ("Use Mono.Unix.Native.Syscall.getppid", true)]
		public static extern int getppid ();

		[Obsolete ("Use Mono.Unix.Native.Syscall.getpgrp", true)]
		public static extern int getpgrp ();

		[Obsolete ("Use Mono.Unix.Native.Syscall.setsid", true)]
		public static extern int setsid ();

		[Obsolete ("Use Mono.Unix.Native.Syscall.setreuid", true)]
		public static extern int setreuid (int ruid, int euid);

		[Obsolete ("Use Mono.Unix.Native.Syscall.setregid", true)]
		public static extern int setregid (int rgid, int egid);

		[Obsolete ("Use Mono.Unix.Native.Syscall.getusername", true)]
		public static string getusername (int uid)
		{
			throw new NotImplementedException ();
		}

		[Obsolete ("Use Mono.Unix.Native.Syscall.getgroupname", true)]
		public static string getgroupname (int gid)
		{
			throw new NotImplementedException ();
		}

		[Obsolete ("Use Mono.Unix.Native.Syscall.GetHostName", true)]
		public static string GetHostName ()
		{
			throw new NotImplementedException ();
		}

		[Obsolete ("Use Mono.Unix.Native.Syscall.gethostname", true)]
		public static string gethostname ()
		{
			throw new NotImplementedException ();
		}

		[Obsolete ("Use Mono.Unix.Native.Syscall.isatty", true)]
		public static bool isatty (int desc)
		{
			throw new NotImplementedException ();
		}

		[Obsolete ("Use Mono.Unix.Native.Syscall.stat", true)]
		public static int stat (string filename, out Stat stat)
		{
			throw new NotImplementedException ();
		}

		[Obsolete ("Use Mono.Unix.Native.Syscall.lstat", true)]
		public static int lstat (string filename, out Stat stat)
		{
			throw new NotImplementedException ();
		}

		[Obsolete ("Use Mono.Unix.Native.Syscall.readlink", true)]
		public static string readlink (string path)
		{
			throw new NotImplementedException ();
		}

		[Obsolete ("Use Mono.Unix.Native.Syscall.strerror", true)]
		public static string strerror (int errnum)
		{
			throw new NotImplementedException ();
		}

		[Obsolete ("Use Mono.Unix.Native.Syscall.opendir", true)]
		public static extern IntPtr opendir (string path);

		[Obsolete ("Use Mono.Unix.Native.Syscall.closedir", true)]
		public static extern int closedir (IntPtr dir);

		[Obsolete ("Use Mono.Unix.Native.Syscall.readdir", true)]
		public static extern string readdir (IntPtr dir);

	}
}
