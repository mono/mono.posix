#if NETCOREAPP3_0_OR_GREATER
using System;
using System.IO;
using System.Reflection;
using System.Runtime.InteropServices;
using System.Text;

namespace Mono.Unix.Native
{
	static class NativeLibraryInitializer
	{
		const string MonoUnixLibraryName = "Mono.Unix";

		static readonly object initLock = new object();

		static bool initialized;
		static string monoUnixLibraryRuntimesPath = String.Empty;
		static string monoUnixLibrarySiblingPath = String.Empty;

		static IntPtr DllImportResolver(string libraryName, Assembly assembly, DllImportSearchPath? searchPath)
		{
			// Mostly for tests running in-repository
			if (File.Exists (monoUnixLibrarySiblingPath)) {
				return NativeLibrary.Load (monoUnixLibrarySiblingPath);
			}

			// We could check if the file exists, but let's instead `Load` throw an exception here
			if (String.Compare (libraryName, MonoUnixLibraryName, StringComparison.OrdinalIgnoreCase) == 0) {
				return NativeLibrary.Load (monoUnixLibraryRuntimesPath);
			}

			// Let the runtime find this library using its own algorithm
			return IntPtr.Zero;
		}

		public static void Init ()
		{
			if (initialized) {
				return;
			}

			lock (initLock) {
				DoInit ();
				initialized = true;
			}
		}

		static void DoInit ()
		{
			// If anything goes wrong, fail quietly, let other code yell it doesn't work
			var sb = new StringBuilder ();
			string extension;

			if (RuntimeInformation.IsOSPlatform (OSPlatform.OSX)) {
				sb.Append ("osx-");
				if (!AppendOSXArchitecture (sb)) {
					return;
				}
				extension = "dylib";
			} else if (RuntimeInformation.IsOSPlatform (OSPlatform.Linux)) {
				sb.Append ("linux-");
				if (!AppendLinuxArchitecture (sb)) {
					return;
				}
				extension = "so";
			} else {
				return;
			}

			Assembly asm = typeof(NativeLibraryInitializer).Assembly;
			string asmDir = Path.GetDirectoryName (asm.Location)!;
			monoUnixLibraryRuntimesPath = Path.Combine (asmDir, "runtimes", sb.ToString (), $"lib{MonoUnixLibraryName}.{extension}");
			monoUnixLibrarySiblingPath = Path.Combine (asmDir, Path.GetFileName (monoUnixLibraryRuntimesPath));

			NativeLibrary.SetDllImportResolver (asm, DllImportResolver);
		}

		static bool AppendLinuxArchitecture (StringBuilder sb)
		{
			if (RuntimeInformation.OSArchitecture == Architecture.X64) {
					sb.Append ("x64");
					return true;
			}

			return false;
		}

		static bool AppendOSXArchitecture (StringBuilder sb)
		{
			switch (RuntimeInformation.OSArchitecture) {
				case Architecture.X64:
					sb.Append ("x64");
					return true;

				case Architecture.Arm64:
					sb.Append ("arm64");
					return true;
			}

			return false;
		}
	}
}
#endif // NETCOREAPP3_0_OR_GREATER
