# WORK IN PROGRESS

Please note that this repository is a work in progress - it MAY happen that history will need
to be rewritten.  Until this notice is removed, please do not rely on the repository history
being set in stone.

STATUS: **NOT BUILDING**

# Mono.Posix standalone repository

## Source imports

Repository contains `Mono.Posix` and its native sources helper sources imported from
the https://github.com/mono/mono/ repository, commit 0bc0f6bef0c05d3367c82d2e414e9db256ae1541

The `src/Mono.Posix` directory here corresponds to `mcs/class/Mono.Posix/` in the Mono repository.

The `src/native` directory here corresponds to `support/` in the Mono repository

# TODO

  - [ ] Create CMake toolchain files for various target systems (`iOS`,
        `watchOS`, `tvOS`)
  - [ ] Migrate to standard types (e.g. `uint32_t`) instead of the glib
        ones
  - [ ] Migrate to C++ (no stdlib linking, just compiler + select
        header files)
  - [ ] Migrate to `<limits>` instead of the glib macros
  - [ ] Review what POSIX APIs are missing
  - [ ] Add OS-specific calls (in namespace per OS)
  - [ ] Replace glib functions with something else (e.g. `g_error`)
  
