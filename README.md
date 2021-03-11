# WORK IN PROGRESS

Please note that this repository is a work in progress - it MAY happen that history will need
to be rewritten.  Until this notice is removed, please do not rely on the repository history
being set in stone.

Native code status:

  * Linux: **building and linking** (gcc 10.2.1, clang 11.0.1)
  * macOS: **building and linking** (Xcode 12.4, x86_64) and arm64
  * Windows: **not tested yet**
  * BSD: **not tested yet**

Managed code status:

  **not started**

# Mono.Posix standalone repository

## Source imports

Repository contains `Mono.Posix` and its native sources helper sources imported from
the https://github.com/mono/mono/ repository, commit 0bc0f6bef0c05d3367c82d2e414e9db256ae1541

The `src/Mono.Posix` directory here corresponds to `mcs/class/Mono.Posix/` in the Mono repository.

The `src/native` directory here corresponds to `support/` in the Mono repository

# TODO

  - [x] Replace Mono atomic calls with compiler intrinsics (or
        standard library calls)
  - [ ] Replace glib functions with something else (e.g. `g_error`)
  - [ ] Get rid of `void*` wherever possible
  - [x] Test build on macOS
  - [ ] Test build on {Free,Open,Net}BSD
  - [ ] Test build on Windows
  - [ ] Create CMake toolchain files for various target systems (`iOS`,
        `watchOS`, `tvOS`, `WASM`)
  - [x] Migrate to standard types (e.g. `uint32_t`) instead of the glib
        ones
  - [x] Migrate to C++ (no stdlib linking, just compiler + selected
        header files)
  - [ ] Migrate to `<limits>` instead of the glib macros
  - [ ] Review what POSIX APIs are missing
  - [ ] Add OS-specific calls (in namespace per OS)
  - [ ] Migrate managed build to a dotnet SDK project (netstandard and
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

