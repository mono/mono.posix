# WORK IN PROGRESS

Please note that this repository is a work in progress - it MAY happen that history will need
to be rewritten.  Until this notice is removed, please do not rely on the repository history
being set in stone.

Native code status:

  * Linux: **building and linking** (gcc 10.2.1, clang 11.0.1)
  * macOS: **building and linking** (Xcode 12.4, x86_64 and arm64)
  * macCatalyst: **building and linking** ((Xcode 12.4, x86_64 and arm64)
  * iOS: **building and linking** (Xcode 12.4, arm64, armv7, armv7s,
    x86_64)
  * Windows: native code will get a separate implementation on Windows
    at some point. This is because Windows POSIX support is very
    minimal and very different to Unix systems, requiring a lot of
    workarounds, conditional code etc, making the Unix code hard to
    read and follow.
  * BSD: **not tested yet**

Managed code status:

  * `Mono.Posix` builds for
    * `net6.0`
	* `netcoreapp3.1`
	* `netstandard2.0`
  * `Mono.Posix.Test` builds for
    * `net6.0`
    * `netcoreapp3.1`

A number of tests fail at this point.

# Mono.Posix standalone repository

## Build requirements

  * All platforms
    * `dotnet` (6.*)
	* `cmake` (3.18 at the minimum, 3.19.5 if building for macCatalyst
      on macOS)
    * `ninja` (any version supported by `cmake`)
	* Android NDK (r21 or newer)
	* C++ compiler with support for the C++17 standard
  * macOS
    * Xcode 12 (with support for Apple silicon)
  * Linux
    * `gcc` or `clang` compilers

## Source imports

Repository contains `Mono.Posix` and its native sources helper sources imported from
the https://github.com/mono/mono/ repository, commit 0bc0f6bef0c05d3367c82d2e414e9db256ae1541

The `src/Mono.Posix` directory here corresponds to `mcs/class/Mono.Posix/` in the Mono repository.

The `src/native` directory here corresponds to `support/` in the Mono repository

# TODO

  - [x] Replace Mono atomic calls with compiler intrinsics (or
        standard library calls)
  - [x] Replace glib functions with something else (e.g. `g_error`)
  - [X] Get rid of `void*` wherever possible
  - [x] Test build on macOS
  - [ ] Test build on {Free,Open,Net}BSD
  - [X] Test build on Windows (not working atm)
  - [X] Create CMake toolchain files for various target systems (`iOS`,
        `watchOS`, `tvOS`, `WASM`)
  - [x] Migrate to standard types (e.g. `uint32_t`) instead of the glib
        ones
  - [x] Migrate to C++ (no stdlib linking, just compiler + selected
        header files)
  - [x] Migrate to `<limits>` instead of the glib macros
  - [ ] Review what POSIX APIs are missing
  - [ ] Add OS-specific calls (in namespace per OS)
  - [X] Migrate managed build to a dotnet SDK project (netstandard and
        netcore)
  - [ ] Write new map generation utility to replace
        `create-native-map`, possibly based on
        [`cppsharp`](https://github.com/mono/CppSharp) or
        [`clangsharp`](https://github.com/Microsoft/ClangSharp)
        although using them could pose problems when generating
        managed versions of structures (they are heavily annotated in
        managed code, sometimes augmented with code and sometimes they
        become classes).  Also, managed enumerations are often
        composed of members which are non-enum macros in the native
        code.  It might be easier to write a simpler version of
        `create-native-map` which caters exactly to the needs of
        `Mono.Posix` without having to think about compatibility with
        3rd party code.
  - [ ] Write a tool to verify `Mono.Posix.dll` against
        `libMonoPosixHelper.so` to make sure that all p/invokes are
        there.
  - [ ] Add guard objects to `free(3)` allocated objects automatically
  - [ ] If a given function/syscall etc isn't available on the host
        OS, we should still make the corresponding p/invoke in the
        helper lib available, but set `errno` to `ENOSYS`

