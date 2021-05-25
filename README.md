# Build status

[![Build Status](https://dev.azure.com/dnceng/public/_apis/build/status/mono/mono-mono.posix?branchName=main)](https://dev.azure.com/dnceng/public/_build/latest?definitionId=966&branchName=main)

<!-- markdown-toc start - Don't edit this section. Run M-x markdown-toc-refresh-toc -->
**Table of Contents**

- [Build status](#build-status)
- [Mono.Posix standalone repository](#monoposix-standalone-repository)
    - [Supported operating systems](#supported-operating-systems)
    - [Supported managed target frameworks](#supported-managed-target-frameworks)
    - [Build requirements](#build-requirements)
    - [Source imports](#source-imports)
    - [Source changes relative to Mono sources](#source-changes-relative-to-mono-sources)
        - [Native code](#native-code)
        - [Managed code](#managed-code)
- [TODO](#todo)

<!-- markdown-toc end -->

# Mono.Posix standalone repository

This repository contains `Mono.Posix` sources imported from the
[Mono](https://github.com/mono/mono) repository in order to continue
active maintenance and development of the assembly and its support
library outside Mono proper.  The goal is to make the assembly
available as a universal nuget that works across a number of Unix and
Unix-like systems.

This repository should be treated as the "canonical" source of
`Mono.Posix` - all new code, bug fixes etc should be implemented here.
Mono copy of the code will not receive any new updates or fixes.

## Supported operating systems

While the native and managed code in this repository should work fine
across the entire spectrum of Unix and Unix-like systems, the only
supported operating systems are the following:

  * **Linux** (binaries are built on Ubuntu 18.04 currently, but they
    **should** work on older distributions, including ones not derived
    from Ubuntu or Debian)
	* Supported architectures: `x86-64`
  * **macOS** (binaries are built on macOS 10.16, targetting macOS 10.9 at
    the minimum)
	* Supported architectures: `x86-64`, `arm64`
  * **iOS** builds target version 10.0 at the minimum
    * Supported architectures: `armv7`, `armv7s`, `arm64`, `simulator
      x86-64`, `simulator arm64`
  * **tvOS** builds target version 9.0 at the minimum
    * Supported architectures: `arm64`, `simulator x86-64`
  * **macOS Catalyst** builds target version 13.1 at the minimum
	* Supported architectures: `x86-64`, `arm64`
	
Builds for Linux and macOS produce static and dynamic libraries, all
the other builds produce only static libraries.  Dynamic libraries do
not have any additional dependencies except for the C library.

The `*BSD` family of Unix operating system is not officially supported
because we have no infrastructure to automatically run and test
`Mono.Posix` on these systems.  However, we will gladly accept any PRs
which fix issues with these systems, implement new functionality
specific to these systems etc.

## Supported managed target frameworks

  * `Mono.Posix` and `Mono.Unix` support
    * `net6.0`
	* `netcoreapp3.1`
	* `netstandard2.1`
	* `netstandard2.0`
	* `net45`
  * `Mono.Unix.Tests` supports
    * `net6.0`
    * `netcoreapp3.1`

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

## Source changes relative to Mono sources

### Native code

All the native code was migrated to C++ (using C++17 standard) and all
functions not used by the managed code were removed.  As the result of
this migration, the native code uses stronger typing and stricter
compile-time error checking.

### Managed code

`Mono.Posix.dll` is now obsolete, with all the code that used to be in
it moved to the new `Mono.Unix.dll` assembly.  `Mono.Posix.dll`
contains only type forwarders for the moved types and a handful of
classes which were previously marked obsolete - they will now produce
build errors instead of warnings, if used by the application
referencing the new `Mono.Posix`.

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

