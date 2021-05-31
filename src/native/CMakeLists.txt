if(TARGET_PLATFORM MATCHES "^catalyst")
  set(MPH_REQUIRED_CMAKE_VERSION "3.19.5")
else()
  set(MPH_REQUIRED_CMAKE_VERSION "3.18")
endif()

cmake_minimum_required(VERSION ${MPH_REQUIRED_CMAKE_VERSION})

file(STRINGS "../../eng/Versions.props" MPH_VERSION_XML REGEX "^[ \t]*<VersionPrefix>(.*)</VersionPrefix>")
string(REGEX REPLACE "^[ \t]*<VersionPrefix>(.*)</VersionPrefix>" "\\1" MPH_VERSION "${MPH_VERSION_XML}")

if(NOT DEFINED MPH_VERSION)
  message(FATAL_ERROR "Couldn't get version from Versions.props")
endif()

if(NOT DEFINED TARGET_PLATFORM)
  message(FATAL_ERROR "Please define the TARGET_PLATFORM variable on command line")
endif()

#
# These files MUST be included before the `project` command
#
include(${CMAKE_CURRENT_LIST_DIR}/../../cmake/toolchain.android.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/../../cmake/toolchain.apple.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/../../cmake/toolchain.linux.cmake)

project(
  Mono.Unix
  VERSION ${MPH_VERSION}
  DESCRIPTION "Native helper library for the Mono.Unix assembly"
  HOMEPAGE_URL "https://github.com/mono/mono.posix/"
  LANGUAGES CXX C
  )

include(CheckIncludeFileCXX)
include(CheckCXXCompilerFlag)
include(CheckLinkerFlag)
include(CheckSymbolExists)
include(CheckStructHasMember)
include(CheckTypeSize)
include(CheckPrototypeDefinition)

if(WIN32)
  set(CCACHE_OPTION_DEFAULT OFF)
else()
  set(CCACHE_OPTION_DEFAULT ON)
endif()

option(STRIP_DEBUG "Strip debugging information when linking" ON)
option(USE_COLOR "Use color to highlight compiler diagnostics" OFF)
option(USE_CCACHE "Use ccache, if found, to speed up recompilation" ${CCACHE_OPTION_DEFAULT})
option(USE_CONSERVATIVE_OPTIMIZATION "Use -O2 to optimize Release builds (instead of -O3)" ON)

if(NOT WIN32 AND USE_CCACHE)
  if(CMAKE_CXX_COMPILER MATCHES "/ccache/")
    message(STATUS "ccache: compiler already uses ccache")
  else()
    find_program(CCACHE ccache)
    if(CCACHE)
      set(CMAKE_CXX_COMPILER_LAUNCHER "${CCACHE}")
      message(STATUS "ccache: compiler will be lauched with ${CCACHE}")
    endif()
  endif()
endif()

if(CMAKE_BUILD_TYPE STREQUAL Debug)
  set(IS_DEBUG_BUILD True)
else()
  set(IS_DEBUG_BUILD False)
endif()

set(MPH_LIB_NAME ${PROJECT_NAME})

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

set(CMAKE_SKIP_INSTALL_RPATH True)
set(CMAKE_SKIP_RPATH True)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(PACKAGE_NAME ${PROJECT_NAME})
set(PACKAGE_VERSION ${PROJECT_VERSION})
set(PACKAGE_STRING "${PROJECT_NAME} v${PROJECT_VERSION}")
set(PACKAGE_VERSION_MAJOR "${PROJECT_VERSION_MAJOR}")
set(PACKAGE_VERSION_MINOR "${PROJECT_VERSION_MINOR}")
set(PACKAGE_VERSION_MICRO "${PROJECT_VERSION_MICRO}")

if(CMAKE_CXX_COMPILER_ID STREQUAL Clang)
  set(IS_CLANG True)
else()
  set(IS_CLANG False)
endif()

set(IS_LINUX False)
set(IS_BSD False)
set(IS_MACOS False)
set(IS_CATALYST False)
set(IS_IOS False)
set(IS_TVOS False)
set(IS_ANDROID False)
set(IS_WASM False)

if(NOT CMAKE_CROSSCOMPILING)
  if(CMAKE_HOST_SYSTEM_NAME STREQUAL Linux)
    set(IS_LINUX True)
  endif()

  if(APPLE)
    set(IS_MACOS True)
  endif()

  # TODO: test it!
  if(CMAKE_HOST_SYSTEM_NAME MATCHES ".*BSD")
    set(IS_BSD True)
  endif()
else()
  if(APPLE)
    if(CMAKE_SYSTEM_NAME STREQUAL iOS)
      set(IS_IOS True)
    elseif(CMAKE_SYSTEM_NAME STREQUAL tvOS)
      set(IS_TVOS True)
    elseif(CMAKE_SYSTEM_NAME STREQUAL watchOS)
      message(FATAL_ERROR "watchOS not supported")
    elseif(CMAKE_SYSTEM_NAME STREQUAL Darwin)
      set(IS_CATALYST True)
    else()
      message(FATAL_ERROR "Unknown system when cross-compiling: ${CMAKE_SYSTEM_NAME}")
    endif()
  endif()

  if(ANDROID)
    set(IS_ANDROID True)
  endif()

  if(NOT ANDROID AND CMAKE_HOST_SYSTEM_NAME STREQUAL Linux)
    set(IS_LINUX True)
  endif()

  # TODO: WASM must be defined in a toolchain file
  if(WASM)
    set(IS_WASM True)
  endif()
endif()

#
# Set flags for symbol check to work properly
#
if(IS_LINUX)
  #
  # _XOPEN_SOURCE=700 enables:
  #
  #  POSIX.1, POSIX.2, and XPG4
  #  SUSv2 (UNIX 98)
  #  SUSv3 (UNIX 03; i.e., the POSIX.1-2001 base specification plus the XSI extension) and C99 definitions
  #  SUSv4 (i.e., the POSIX.1-2008 base specification plus the XSI extension)
  #
  #  on Linux, see the feature_test_macros(7) manpage for more information
  #
  set(HOST_MACROS
    _GNU_SOURCE
    _XOPEN_SOURCE=700
    _BSD_SOURCE # deprecated on newer systems
    _DEFAULT_SOURCE
    _POSIX_C_SOURCE
    _SVID_SOURCE
    )

  set(CMAKE_REQUIRED_DEFINITIONS "")
  foreach(MACRO ${HOST_MACROS})
    list(APPEND CMAKE_REQUIRED_DEFINITIONS "-D${MACRO}")
  endforeach()
endif()

if(IS_ANDROID)
  set(HOST_MACROS
    _GNU_SOURCE
    _BSD_SOURCE
    )
endif()

if(APPLE)
  set(HOST_MACROS
    _DARWIN_C_SOURCE
    )
endif()

macro(_compiler_has_flag _lang _flag)
  string(REGEX REPLACE "-|,|=" "_" _flag_name ${_flag})
  string(TOUPPER "${_lang}" _lang_upper)

  cmake_language(CALL check_${_lang}_compiler_flag "${_flag}" HAS_${_flag_name}_${_lang_upper})
  if(HAS_${_flag_name}_${_lang_upper})
    set(COMPILER_FLAG_FOUND True)
  else()
    set(COMPILER_FLAG_FOUND False)
  endif()
endmacro()

macro(cxx_compiler_has_flag _flag)
  _compiler_has_flag(cxx ${_flag})
endmacro()

macro(_linker_has_flag _lang _flag)
  string(REGEX REPLACE "-|,|=" "_" _flag_name ${_flag})
  string(TOUPPER "${_lang}" _lang_upper)

  check_linker_flag(${_lang} "${_flag}" HAS_${_flag_name}_LINKER_${_lang_upper})
  if(HAS_${_flag_name}_LINKER_${_lang_upper})
    set(LINKER_FLAG_FOUND True)
  else()
    set(LINKER_FLAG_FOUND False)
  endif()
endmacro()

macro(cxx_linker_has_flag _flag)
  _linker_has_flag(CXX ${_flag})
endmacro()

macro(mph_check_symbol_exists _symbol _files _variable)
  check_symbol_exists(${_symbol} "${_files}" ${_variable})
endmacro()

macro(mph_check_type_exists _type _variable _includes)
  set(OLD_CMAKE_EXTRA_INCLUDE_FILES ${CMAKE_EXTRA_INCLUDE_FILES})
  set(CMAKE_EXTRA_INCLUDE_FILES ${_includes})

  check_type_size(${_type} ${_variable} LANGUAGE CXX)

  set(CMAKE_EXTRA_INCLUDE_FILES ${OLD_CMAKE_EXTRA_INCLUDE_FILES})
endmacro()

set(MPH_SOURCES
  dirent.cc
  errno.cc
  fcntl.cc
  fstab.cc
  grp.cc
  macros.cc
  map.cc
  old-map.cc
  pwd.cc
  signal.cc
  stdio.cc
  stdlib.cc
  string.cc
  syslog.cc
  sys-mman.cc
  sys-sendfile.cc
  sys-socket.cc
  sys-stat.cc
  sys-statvfs.cc
  sys-time.cc
  sys-uio.cc
  sys-utsname.cc
  sys-wait.cc
  sys-xattr.cc
  time.cc
  unistd.cc
  utime.cc
  x-struct-str.cc
  )

if(IS_IOS OR IS_TVOS OR IS_CATALYST)
  list(APPEND MPH_SOURCES
    compat.apple.cc
    )
endif()

# Args shared between the C and C++ compilers
set(LOCAL_COMPILER_ARGS
  -fvisibility=hidden
  -fstack-protector-strong
  -fstrict-return
  -fno-strict-aliasing
  -ffunction-sections
  -funswitch-loops
  -finline-limit=500
  -Wa,-noexecstack
  -fPIC
  -g
  -Wa,--noexecstack
  -Wformat
  -Werror=format-security
  -Wall
  -Wextra
  -Wnull-dereference
  -Wmisleading-indentation
  -Wunused
  -Wpedantic
  -Wsuggest-final-types
  -Wsuggest-final-methods
  -Wint-to-pointer-cast
  -Wsuggest-override
  -Wunguarded-availability
  )

if(NOT IS_ANDROID)
  # Android toolchain takes care of this on its own, based on selected features
  list(APPEND LOCAL_COMPILER_ARGS
    -fno-exceptions
    -fno-rtti
    )
endif()

set(LOCAL_LINKER_ARGS
  -rdynamic
  -fstack-protector-strong
  -static-libgcc
  LINKER:-fstrict-return
  LINKER:-z,now
  LINKER:-z,relro
  LINKER:-z,noexecstack
  LINKER:--no-undefined
  LINKER:--export-dynamic
  )

if(IS_LINUX)
  list(APPEND LOCAL_LINKER_ARGS -static-libstdc++)
endif()

# Let CMake set the default optimization flag for Release builds
if(IS_DEBUG_BUILD)
  list(APPEND LOCAL_COMPILER_ARGS -O0)
elseif(USE_CONSERVATIVE_OPTIMIZATION)
  list(APPEND LOCAL_COMPILER_ARGS -O2)
endif()

if(UNIX)
  list(APPEND LOCAL_COMMON_LINKER_ARGS -shared -fpic)
endif()

if(USE_COLOR)
  list(APPEND LOCAL_COMPILER_ARGS -fdiagnostics-color=always)
else()
  list(APPEND LOCAL_COMPILER_ARGS -fdiagnostics-color=auto)
endif()

if(STRIP_DEBUG)
  list(APPEND LOCAL_COMMON_LINKER_ARGS LINKER:-S)
endif()

#
# Compiler and linker flag checks
#
set(_CHECKED_FLAGS "")
foreach(flag ${LOCAL_COMPILER_ARGS})
  cxx_compiler_has_flag(${flag})
  if(COMPILER_FLAG_FOUND)
    list(APPEND _CHECKED_FLAGS "${flag}")
  endif()
endforeach()
set(MPH_CXX_FLAGS "${_CHECKED_FLAGS}")

set(_CHECKED_FLAGS "")
foreach(flag ${LOCAL_LINKER_ARGS})
  cxx_linker_has_flag(${flag})
  if(LINKER_FLAG_FOUND)
    list(APPEND _CHECKED_FLAGS "${flag}")
  endif()
endforeach()
set(MPH_CXX_LINKER_FLAGS "${_CHECKED_FLAGS}")

add_compile_options(${MPH_CXX_FLAGS})

add_link_options(${MPH_CXX_LINKER_FLAGS})

#
# Command line macro definitions
#
add_compile_definitions(HAVE_CONFIG_H)
add_compile_definitions(_FORTIFY_SOURCE=2)
add_compile_definitions(${HOST_MACROS})

if(NOT IS_DEBUG_BUILD)
  add_compile_definitions(RELEASE NDEBUG)
else()
  add_compile_definitions(DEBUG)
endif()

if(IS_LINUX)
  add_compile_definitions(HOST_LINUX)
endif()

if(IS_BSD)
  add_compile_definitions(HOST_BSD)
endif()

if(IS_ANDROID)
  add_compile_definitions(HOST_ANDROID)
  add_compile_definitions(ANDROID_UNIFIED_HEADERS)
  add_compile_definitions(__ANDROID_API__=${ANDROID_NATIVE_API_LEVEL})

  if (ANDROID_NDK_MAJOR LESS 20)
    add_compile_definitions(__ANDROID_API_Q__=29)
  endif()
endif()

if(WIN32)
  add_compile_definitions(HOST_WIN32)
endif()

if(IS_MACOS OR IS_CATALYST)
  add_compile_definitions(HOST_DARWIN)
endif()

if(IS_CATALYST)
  add_compile_definitions(HOST_CATALYST)
endif()

if(IS_IOS)
  add_compile_definitions(HOST_IOS)
endif()

if(IS_TVOS)
  add_compile_definitions(HOST_TVOS)
endif()

if(IS_WATCHOS)
  add_compile_definitions(HOST_WATCHOS)
endif()

#
# Please keep the checks sorted alphabetically within their respective groups, for ease of navigation
#

#
# Header checks
#
check_include_file_cxx("attr/xattr.h" HAVE_ATTR_ATTR_H)
check_include_file_cxx("checklist.h" HAVE_CHECKLIST_H)
check_include_file_cxx("dirent.h" HAVE_DIRENT_H)
check_include_file_cxx("fstab.h" HAVE_FSTAB_H)
check_include_file_cxx("grp.h" HAVE_GRP_H)
check_include_file_cxx("linux/netlink.h" HAVE_LINUX_NETLINK_H)
check_include_file_cxx("linux/rtnetlink.h" HAVE_LINUX_RTNETLINK_H)
check_include_file_cxx("linux/serial.h" HAVE_LINUX_SERIAL_H)
check_include_file_cxx("netinet/in.h" HAVE_NETINET_IN_H)
check_include_file_cxx("poll.h" HAVE_POLL_H)
check_include_file_cxx("pwd.h" HAVE_PWD_H)
check_include_file_cxx("stdint.h" HAVE_STDINT_H)
check_include_file_cxx("stdio.h" HAVE_STDIO_H)
check_include_file_cxx("sys/extattr.h" HAVE_SYS_EXTATTR_H)
check_include_file_cxx("syslog.h" HAVE_SYSLOG_H)
check_include_file_cxx("sys/mman.h" HAVE_SYS_MMAN_H)
check_include_file_cxx("sys/param.h" HAVE_SYS_PARAM_H)
check_include_file_cxx("sys/poll.h" HAVE_SYS_POLL_H)
check_include_file_cxx("sys/sendfile.h" HAVE_SYS_SENDFILE_H)
check_include_file_cxx("sys/socket.h" HAVE_SYS_SOCKET_H)
check_include_file_cxx("sys/stat.h" HAVE_SYS_STAT_H)
check_include_file_cxx("sys/statvfs.h" HAVE_SYS_STATVFS_H)
check_include_file_cxx("sys/time.h" HAVE_SYS_TIME_H)
check_include_file_cxx("sys/uio.h" HAVE_SYS_UIO_H)
check_include_file_cxx("sys/vfs.h" HAVE_SYS_VFS_H)
check_include_file_cxx("sys/vfstab.h" HAVE_SYS_VFSTAB_H)
check_include_file_cxx("sys/wait.h" HAVE_SYS_WAIT_H)
check_include_file_cxx("sys/xattr.h" HAVE_SYS_XATTR_H)
check_include_file_cxx("sys/xattr.h" HAVE_SYS_XATTR_H)
check_include_file_cxx("unistd.h" HAVE_UNISTD_H)
check_include_file_cxx("utime.h" HAVE_UTIME_H)

#
# Type checks
#
mph_check_type_exists(blkcnt_t BLKCNT_T "sys/stat.h")
mph_check_type_exists(blksize_t BLKSIZE_T "sys/stat.h")
mph_check_type_exists(off_t OFF_T "sys/types.h;limits.h")
mph_check_type_exists(sig_t SIG_T "signal.h")
mph_check_type_exists(sighandler_t SIGHANDLER_T "signal.h")
mph_check_type_exists(suseconds_t SUSECONDS_T "sys/time.h")

if(HAVE_OFF_T AND ${OFF_T} GREATER_EQUAL 8)
  set(HAVE_LARGE_FILE_SUPPORT ON)
else()
  set(HAVE_LARGE_FILE_SUPPORT OFF)
endif()

#
# Function checks
#
mph_check_symbol_exists(accept4 "sys/types.h;sys/socket.h" HAVE_ACCEPT4)
mph_check_symbol_exists(confstr "unistd.h" HAVE_CONFSTR)
mph_check_symbol_exists(endgrent "sys/types.h;grp.h" HAVE_ENDGRENT)
mph_check_symbol_exists(endpwent "sys/types.h;pwd.h" HAVE_ENDPWENT)
mph_check_symbol_exists(endusershell "unistd.h" HAVE_ENDUSERSHELL)
mph_check_symbol_exists(fgetgrent "stdio.h;sys/types.h;grp.h" HAVE_FGETGRENT)
mph_check_symbol_exists(fgetpwent "sys/types.h;pwd.h" HAVE_FGETPWENT)
mph_check_symbol_exists(fstatat "sys/types.h;sys/stat.h;unistd.h" HAVE_FSTATAT)
mph_check_symbol_exists(fstatfs "sys/vfs.h" HAVE_FSTATFS)
mph_check_symbol_exists(fstatvfs "sys/statvfs.h" HAVE_FSTATVFS)
mph_check_symbol_exists(futimens "fcntl.h;sys/stat.h" HAVE_FUTIMENS)
mph_check_symbol_exists(futimes "sys/time.h" HAVE_FUTIMES)
mph_check_symbol_exists(getdomainname "unistd.h" HAVE_GETDOMAINNAME)
mph_check_symbol_exists(getfsstat "sys/param.h;sys/ucred.h;sys/mount.h" HAVE_GETFSSTAT)
mph_check_symbol_exists(getgrent "sys/types.h;grp.h" HAVE_GETGRENT)
mph_check_symbol_exists(getgrgid_r "sys/types.h;grp.h" HAVE_GETGRGID_R)
mph_check_symbol_exists(getgrnam_r "sys/types.h;grp.h" HAVE_GETGRNAM_R)
mph_check_symbol_exists(gethostid "unistd.h" HAVE_GETHOSTID)
mph_check_symbol_exists(getlogin_r "unistd.h" HAVE_GETLOGIN_R)
mph_check_symbol_exists(getpwent "sys/types.h;pwd.h" HAVE_GETPWENT)
mph_check_symbol_exists(getpwnam_r "sys/types.h;pwd.h" HAVE_GETPWNAM_R)
mph_check_symbol_exists(getpwuid_r "sys/types.h;pwd.h" HAVE_GETPWUID_R)
mph_check_symbol_exists(lockf "unistd.h" HAVE_LOCKF)
mph_check_symbol_exists(lutimes "sys/time.h" HAVE_LUTIMES)
mph_check_symbol_exists(mincore "unistd.h;sys/mman.h" HAVE_MINCORE)
mph_check_symbol_exists(mknodat "sys/types.h;sys/stat.h;fcntl.h;unistd.h" HAVE_MKNODAT)
mph_check_symbol_exists(mlock "sys/mman.h" HAVE_MLOCK)
mph_check_symbol_exists(mremap "sys/mman.h" HAVE_MREMAP)
mph_check_symbol_exists(munlock "sys/mman.h" HAVE_MUNLOCK)
mph_check_symbol_exists(posix_fadvise "fcntl.h" HAVE_POSIX_FADVISE)
mph_check_symbol_exists(posix_fallocate "fcntl.h" HAVE_POSIX_FALLOCATE)
mph_check_symbol_exists(posix_madvise "sys/mman.h" HAVE_POSIX_MADVISE)
mph_check_symbol_exists(preadv "sys/uio.h" HAVE_PREADV)
mph_check_symbol_exists(psignal "signal.h" HAVE_PSIGNAL)
mph_check_symbol_exists(pwritev "sys/uio.h" HAVE_PWRITEV)
mph_check_symbol_exists(readlinkat "unistd.h" HAVE_READLINKAT)
mph_check_symbol_exists(readv "sys/uio.h" HAVE_READV)
mph_check_symbol_exists(remap_file_pages "sys/mman.h" HAVE_REMAP_FILE_PAGES)
mph_check_symbol_exists(seekdir "dirent.h" HAVE_SEEKDIR)
mph_check_symbol_exists(sendfile "sys/sendfile.h" HAVE_SENDFILE)
mph_check_symbol_exists(setdomainname "unistd.h" HAVE_SETDOMAINNAME)
mph_check_symbol_exists(setgrent "sys/types.h;grp.h" HAVE_SETGRENT)
mph_check_symbol_exists(setgroups "sys/types.h;unistd.h;grp.h" HAVE_SETGROUPS)
mph_check_symbol_exists(sethostid "unistd.h" HAVE_SETHOSTID)
mph_check_symbol_exists(sethostname "unistd.h" HAVE_SETHOSTNAME)
mph_check_symbol_exists(setpwent "sys/types.h;pwd.h" HAVE_SETPWENT)
mph_check_symbol_exists(setusershell "unistd.h" HAVE_SETUSERSHELL)
mph_check_symbol_exists(signal "signal.h" HAVE_SIGNAL)
mph_check_symbol_exists(statfs "sys/vfs.h" HAVE_STATFS)
mph_check_symbol_exists(statvfs "sys/statvfs.h" HAVE_STATVFS)
mph_check_symbol_exists(stime "time.h" HAVE_STIME)
mph_check_symbol_exists(strerror_r "string.h" HAVE_STRERROR_R)
mph_check_symbol_exists(swab "unistd.h" HAVE_SWAB)
mph_check_symbol_exists(telldir "dirent.h" HAVE_TELLDIR)
mph_check_symbol_exists(ttyname_r "unistd.h" HAVE_TTYNAME_R)
mph_check_symbol_exists(utimensat "fcntl.h;sys/stat.h" HAVE_UTIMENSAT)
mph_check_symbol_exists(vsnprintf "stdio.h" HAVE_VSNPRINTF)
mph_check_symbol_exists(vsyslog "syslog.h" HAVE_VSYSLOG)
mph_check_symbol_exists(writev "sys/uio.h" HAVE_WRITEV)

#
# Structure checks
#
check_struct_has_member("struct cmsghdr" cmsg_len "sys/socket.h" HAVE_STRUCT_CMSGHDR LANGUAGE CXX)
check_struct_has_member("struct dirent" d_off "dirent.h" HAVE_STRUCT_DIRENT_D_OFF LANGUAGE CXX)
check_struct_has_member("struct dirent" d_reclen "dirent.h" HAVE_STRUCT_DIRENT_D_RECLEN LANGUAGE CXX)
check_struct_has_member("struct dirent" d_type "dirent.h" HAVE_STRUCT_DIRENT_D_TYPE LANGUAGE CXX)
check_struct_has_member("struct flock" l_type "fcntl.h" HAVE_STRUCT_FLOCK LANGUAGE CXX)
check_struct_has_member("struct iovec" iov_base "sys/uio.h" HAVE_STRUCT_IOVEC LANGUAGE CXX)
check_struct_has_member("struct linger" l_onoff "sys/socket.h" HAVE_STRUCT_LINGER LANGUAGE CXX)
check_struct_has_member("struct passwd" pw_gecos "pwd.h" HAVE_STRUCT_PASSWD_PW_GECOS LANGUAGE CXX)
check_struct_has_member("struct pollfd" fd "poll.h" HAVE_STRUCT_POLLFD LANGUAGE CXX)
check_struct_has_member("struct sockaddr_in6" sin6_family "sys/socket.h;netinet/in.h" HAVE_STRUCT_SOCKADDR_IN6 LANGUAGE CXX)
check_struct_has_member("struct sockaddr_in" sin_family "sys/socket.h;netinet/in.h" HAVE_STRUCT_SOCKADDR_IN LANGUAGE CXX)
check_struct_has_member("struct statfs" f_flags "sys/vfs.h" HAVE_STRUCT_STATFS_F_FLAGS LANGUAGE CXX)
check_struct_has_member("struct stat" st_atimespec "sys/stat.h" HAVE_STRUCT_STAT_ST_ATIMESPEC LANGUAGE CXX)
check_struct_has_member("struct stat" st_atim "sys/stat.h" HAVE_STRUCT_STAT_ST_ATIM LANGUAGE CXX)
check_struct_has_member("struct stat" st_ctim "sys/stat.h" HAVE_STRUCT_STAT_ST_CTIM LANGUAGE CXX)
check_struct_has_member("struct stat" st_mtim "sys/stat.h" HAVE_STRUCT_STAT_ST_MTIM LANGUAGE CXX)
check_struct_has_member("struct timespec" tv_sec "time.h" HAVE_STRUCT_TIMESPEC LANGUAGE CXX)
check_struct_has_member("struct timeval" tv_sec "time.h;sys/time.h" HAVE_STRUCT_TIMEVAL LANGUAGE CXX)
check_struct_has_member("struct timezone" tz_minuteswest "sys/time.h" HAVE_STRUCT_TIMEZONE LANGUAGE CXX)
check_struct_has_member("struct utimbuf" actime "utime.h" HAVE_STRUCT_UTIMBUF LANGUAGE CXX)

#
# Misc
#
check_prototype_definition(
  strerror_r
  "char *strerror_r(int errnum, char *buf, size_t buflen)"
  "NULL"
  "string.h"
  STRERROR_R_CHAR_P
  )

#
# Include directories
#
if(ANDROID)
  # We build without the stdlib, but since we use some include files from it, we
  # need to add the include path manually here
  include_directories(${CMAKE_SYSROOT}/usr/include/c++/v1/)
endif()

include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/compat)

if(IS_MACOS)
  set(SDK_SUPPORTS_ARM64 False)
  set(SDK_SUPPORTS_X86_64 False)
  execute_process(
    COMMAND xcode-select -p
    RESULT_VARIABLE XCODE_SELECT_RESULT
    OUTPUT_VARIABLE XCODE_DEVELOPER_PATH
    )
  if(NOT ${XCODE_SELECT_RESULT} EQUAL "0")
    message(WARNING "xcode-select failed with result ${XCODE_SELECT_RESULT}")
  else()
    string(STRIP "${XCODE_DEVELOPER_PATH}" XCODE_DEVELOPER_PATH)
    set(SDKSETTINGS_PATH "${XCODE_DEVELOPER_PATH}/Platforms/MacOSX.platform/Developer/SDKs/MacOSX.sdk/SDKSettings.plist")

    # CAUTION: do NOT ever remove the '-o -' parameter, without '-o' plutil will overwrite the .plist file
    execute_process(
      COMMAND plutil -extract SupportedTargets.macosx.Archs json -o - "${SDKSETTINGS_PATH}"
      RESULT_VARIABLE PLUTIL_RESULT
      OUTPUT_VARIABLE SDK_ARCHITECTURES
      )
    if(NOT ${PLUTIL_RESULT} EQUAL 0)
      message(WARNING "plutil failed to read ${SDKSETTINGS_PATH}, returned with result ${PLUTIL_RESULT}")
    else()
      string(FIND "${SDK_ARCHITECTURES}" "\"arm64\"" ARCH_POS)
      if(${ARCH_POS} GREATER_EQUAL 0)
        set(SDK_SUPPORTS_ARM64 True)
      endif()

      string(FIND "${SDK_ARCHITECTURES}" "\"x86_64\"" ARCH_POS)
      if(${ARCH_POS} GREATER_EQUAL 0)
        set(SDK_SUPPORTS_X86_64 True)
      endif()
    endif()
  endif()

  unset(MPH_OSX_ARCHITECTURES)
  if(SDK_SUPPORTS_ARM64 AND MACOS_ARM64)
    message(STATUS "SDK at ${XCODE_DEVELOPER_PATH} supports creation of ARM64 binaries")
    list(APPEND MPH_OSX_ARCHITECTURES "arm64")
  endif()

  if(SDK_SUPPORTS_X86_64 AND MACOS_X64)
    message(STATUS "SDK at ${XCODE_DEVELOPER_PATH} supports creation of X86_64 binaries")
    list(APPEND MPH_OSX_ARCHITECTURES "x86_64")
  endif()
endif()

if(IS_IOS OR IS_TVOS OR IS_CATALYST)
  add_library(
    ${MPH_LIB_NAME}
    STATIC
    ${MPH_SOURCES}
  )
else()
  add_library(
    ${MPH_LIB_NAME}
    SHARED
    ${MPH_SOURCES}
    )

  add_library(
    "${MPH_LIB_NAME}-static"
    STATIC
    ${MPH_SOURCES}
    )

  set_target_properties(
    "${MPH_LIB_NAME}-static" PROPERTIES
    OUTPUT_NAME "${MPH_LIB_NAME}"
    POSITION_INDEPENDENT_CODE True
    )
endif()

if(IS_MACOS)
  set_target_properties(
    ${MPH_LIB_NAME}
    PROPERTIES
    OSX_ARCHITECTURES "${MPH_OSX_ARCHITECTURES}"
    )

  set_target_properties(
    "${MPH_LIB_NAME}-static"
    PROPERTIES
    OSX_ARCHITECTURES "${MPH_OSX_ARCHITECTURES}"
    )
endif()

#
# Place debug info in separate files, if possible
#
if(IS_LINUX OR IS_ANDROID)
  if(IS_LINUX)
    set(OBJCOPY "${TOOLCHAIN_PREFIX}objcopy")
    set(STRIP "${TOOLCHAIN_PREFIX}strip")
  else()
    set(OBJCOPY "${ANDROID_TOOLCHAIN_ROOT}/bin/llvm-objcopy${ANDROID_TOOLCHAIN_SUFFIX}")
    set(STRIP "${ANDROID_TOOLCHAIN_ROOT}/bin/llvm-strip${ANDROID_TOOLCHAIN_SUFFIX}")
  endif()

  add_custom_command(
    TARGET ${MPH_LIB_NAME}
    POST_BUILD
    WORKING_DIRECTORY "$<TARGET_FILE_DIR:${MPH_LIB_NAME}>"
    COMMAND ${OBJCOPY} --only-keep-debug "$<TARGET_FILE_NAME:${MPH_LIB_NAME}>" "$<TARGET_FILE_NAME:${MPH_LIB_NAME}>.debug"
    COMMAND ${STRIP} --strip-debug --strip-unneeded "$<TARGET_FILE_NAME:${MPH_LIB_NAME}>"
    COMMAND ${OBJCOPY} --add-gnu-debuglink="$<TARGET_FILE_NAME:${MPH_LIB_NAME}>.debug" "$<TARGET_FILE_NAME:${MPH_LIB_NAME}>"
    )
endif()

configure_file(config.h.in config.h @ONLY)
