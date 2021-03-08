# CONTINUE IN: sys-socket.c
cmake_minimum_required(VERSION 3.18)

set(MONO_POSIX_HELPER_VERSION_MAJOR 6)
set(MONO_POSIX_HELPER_VERSION_MINOR 13)
set(MONO_POSIX_HELPER_VERSION_MICRO 0)

set(MONO_POSIX_HELPER_VERSION "${MONO_POSIX_HELPER_VERSION_MAJOR}.${MONO_POSIX_HELPER_VERSION_MINOR}.${MONO_POSIX_HELPER_VERSION_MICRO}")

project(
  MonoPosixHelper
  VERSION ${MONO_POSIX_HELPER_VERSION}
  DESCRIPTION "Native helper library for the Mono.Posix assembly"
  HOMEPAGE_URL "https://github.com/mono/mono.posix/"
  LANGUAGES C CXX
  )

option(STRIP_DEBUG "Strip debugging information when linking" ON)

if(CMAKE_BUILD_TYPE STREQUAL Debug)
  set(IS_DEBUG_BUILD True)
else()
  set(IS_DEBUG_BUILD False)
endif()

set(MPH_LIB_NAME ${PROJECT_NAME})

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

set(CMAKE_BUILD_RPATH "${CMAKE_BINARY_DIR}/lib")

set(CMAKE_SKIP_INSTALL_RPATH TRUE)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

set(PACKAGE_NAME ${PROJECT_NAME})
set(PACKAGE_VERSION ${PROJECT_VERSION})
set(PACKAGE_STRING "${PROJECT_NAME} v${PROJECT_VERSION}")
set(PACKAGE_VERSION_MAJOR "${MONO_POSIX_HELPER_VERSION_MAJOR}")
set(PACKAGE_VERSION_MINOR "${MONO_POSIX_HELPER_VERSION_MINOR}")
set(PACKAGE_VERSION_MICRO "${MONO_POSIX_HELPER_VERSION_MICRO}")

if(NOT ANDROID AND CMAKE_HOST_SYSTEM_NAME STREQUAL Linux)
  set(IS_LINUX True)
else()
  set(IS_LINUX False)
endif()

# TODO: implement check for *BSD
set(IS_BSD False)

if(APPLE)
  set(IS_IOS OFF)
  set(IS_WATCHOS OFF)
  set(IS_TVOS OFF)
  set(IS_MACOS OFF)

  if(CMAKE_SYSTEM_NAME STREQUAL iOS)
    set(IS_IOS ON)
  elseif(CMAKE_SYSTEM_NAME STREQUAL tvOS)
    set(IS_TVOS ON)
  elseif(CMAKE_SYSTEM_NAME STREQUAL watchOS)
    set(IS_WATCHOS ON)
  else()
    set(IS_MACOS ON)
  endif()
endif()

#
# Set flags for symbol check to work properly
#
if(IS_LINUX)
  set(HOST_MACROS
    _GNU_SOURCE
    _XOPEN_SOURCE
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

include(CheckIncludeFiles)
include(CheckCCompilerFlag)
include(CheckCXXCompilerFlag)
include(CheckCCompilerFlag)
include(CheckLinkerFlag)
include(CheckSymbolExists)
include(CheckStructHasMember)
include(CheckTypeSize)

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

macro(c_compiler_has_flag _flag)
  _compiler_has_flag(c ${_flag})
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

macro(c_linker_has_flag _flag)
  _linker_has_flag(C ${_flag})
endmacro()

macro(mph_check_symbol_exists _symbol _files _variable)
  check_symbol_exists(${_symbol} "${_files}" ${_variable})
endmacro()

macro(mph_check_type_exists _type _variable _includes)
  set(OLD_CMAKE_EXTRA_INCLUDE_FILES ${CMAKE_EXTRA_INCLUDE_FILES})
  set(CMAKE_EXTRA_INCLUDE_FILES ${_includes})

  check_type_size(${_type} ${_variable})

  set(CMAKE_EXTRA_INCLUDE_FILES ${OLD_CMAKE_EXTRA_INCLUDE_FILES})
endmacro()

set(MPH_SOURCES
  dirent.c
  errno.c
  fcntl.c
  fstab.c
  grp.c
  macros.c
  map.c
  nl.c
  old-map.c
  pwd.c
  serial.c
  signal.c
  stdio.c
  stdlib.c
  string.c
  support-heap.c
  supportw.c
  syslog.c
  sys-mman.c
  sys-sendfile.c
  sys-socket.c
  sys-stat.c
  sys-statvfs.c
  sys-time.c
  sys-uio.c
  sys-utsname.c
  sys-wait.c
  sys-xattr.c
  time.c
  unistd.c
  utime.c
  x-struct-str.c
  )

set(LOCAL_COMPILER_ARGS
  -fvisibility=hidden
  -fstack-protector-strong
  -fstrict-return
  -fno-strict-aliasing
  -ffunction-sections
  -funswitch-loops
  -finline-limit=300
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
  -Wsuggest-override
  )

set(LOCAL_LINKER_ARGS
  -rdynamic
  -fstack-protector-strong
  LINKER:-fstrict-return
  LINKER:-z,now
  LINKER:-z,relro
  LINKER:-z,noexecstack
  LINKER:--no-undefined
  LINKER:--export-dynamic
  )

if(IS_DEBUG_BUILD)
  list(APPEND LOCAL_COMPILER_ARGS -O0)
else()
  list(APPEND LOCAL_COMPILER_ARGS -O2)
endif()

if(UNIX)
  list(APPEND LOCAL_COMMON_LINKER_ARGS -shared -fpic)
endif()

if(STRIP_DEBUG)
  list(APPEND LOCAL_COMMON_LINKER_ARGS LINKER:-S)
endif()

if(NOT IS_DEBUG_BUILD)
  add_compile_definitions(RELEASE NDEBUG)
else()
  add_compile_definitions(DEBUG)
endif()

#
# Compiler and linker flag checks
#
set(_CHECKED_FLAGS "")
foreach(flag ${LOCAL_COMPILER_ARGS})
  c_compiler_has_flag(${flag})
  if(COMPILER_FLAG_FOUND)
    list(APPEND _CHECKED_FLAGS "${flag}")
  endif()
endforeach()
set(MPH_C_FLAGS "${_CHECKED_FLAGS}")

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
  c_linker_has_flag(${flag})
  if(LINKER_FLAG_FOUND)
    list(APPEND _CHECKED_FLAGS "${flag}")
  endif()
endforeach()
set(MPH_C_LINKER_FLAGS "${_CHECKED_FLAGS}")

set(_CHECKED_FLAGS "")
foreach(flag ${LOCAL_LINKER_ARGS})
  cxx_linker_has_flag(${flag})
  if(LINKER_FLAG_FOUND)
    list(APPEND _CHECKED_FLAGS "${flag}")
  endif()
endforeach()
set(MPH_CXX_LINKER_FLAGS "${_CHECKED_FLAGS}")

add_compile_options("$<$<COMPILE_LANGUAGE:CXX>:${MPH_CXX_FLAGS}>")
add_compile_options("$<$<COMPILE_LANGUAGE:C>:${MPH_C_FLAGS}>")

add_link_options("$<$<COMPILE_LANGUAGE:CXX>:${MPH_CXX_LINKER_FLAGS}>")
add_link_options("$<$<COMPILE_LANGUAGE:C>:${MPH_C_LINKER_FLAGS}>")

#
# Command line macro definitions
#
add_compile_definitions(HAVE_CONFIG_H)
add_compile_definitions(${HOST_MACROS})

if(IS_LINUX)
  add_compile_definitions(HOST_LINUX)
endif()

if(IS_BSD)
  add_compile_definitions(HOST_BSD)
endif()

if(ANDROID)
  add_compile_definitions(HOST_ANDROID)
  add_compile_definitions(ANDROID_UNIFIED_HEADERS)
endif()

if(WIN32)
  add_compile_definitions(HOST_WIN32)
endif()

if(IS_MACOS)
  add_compile_definitions(HOST_DARWIN)
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
# Header checks
#
check_include_files("unistd.h" HAVE_UNISTD_H)
check_include_files("checklist.h" HAVE_CHECKLIST_H)
check_include_files("fstab.h" HAVE_FSTAB_H)
check_include_files("sys/vfstab.h" HAVE_SYS_VFSTAB_H)
check_include_files("sys/param.h" HAVE_SYS_PARAM_H)
check_include_files("netinet/in.h" HAVE_NETINET_IN_H)
check_include_files("sys/time.h" HAVE_SYS_TIME_H)
check_include_files("sys/poll.h" HAVE_SYS_POLL_H)
check_include_files("sys/wait.h" HAVE_SYS_WAIT_H)
check_include_files("sys/statvfs.h" HAVE_SYS_STATVFS_H)
check_include_files("sys/xattr.h" HAVE_SYS_XATTR_H)
check_include_files("sys/mman.h" HAVE_SYS_MMAN_H)
check_include_files("sys/socket.h" HAVE_SYS_SOCKET_H)
check_include_files("sys/uio.h" HAVE_SYS_UIO_H)
check_include_files("poll.h" HAVE_POLL_H)
check_include_files("grp.h" HAVE_GRP_H)
check_include_files("syslog.h" HAVE_SYSLOG_H)
check_include_files("dirent.h" HAVE_DIRENT_H)
check_include_files("utime.h" HAVE_UTIME_H)
check_include_files("sys/stat.h" HAVE_SYS_STAT_H)
check_include_files("stdint.h" HAVE_STDINT_H)
check_include_files("stdio.h" HAVE_STDIO_H)
check_include_files("pwd.h" HAVE_PWD_H)
check_include_files("linux/netlink.h" HAVE_LINUX_NETLINK_H)
check_include_files("linux/rtnetlink.h" HAVE_LINUX_RTNETLINK_H)
check_include_files("linux/serial.h" HAVE_LINUX_SERIAL_H)
check_include_files("sys/sendfile.h" HAVE_SYS_SENDFILE_H)

#
# Type checks
#
mph_check_type_exists(blkcnt_t BLKCNT_T "sys/stat.h")
mph_check_type_exists(blksize_t BLKSIZE_T "sys/stat.h")
mph_check_type_exists(suseconds_t SUSECONDS_T "sys/time.h")
mph_check_type_exists(off_t OFF_T "sys/types.h;limits.h")

if(HAVE_OFF_T AND ${OFF_T} GREATER_EQUAL 8)
  set(HAVE_LARGE_FILE_SUPPORT ON)
else()
  set(HAVE_LARGE_FILE_SUPPORT OFF)
endif()

#
# Function checks
#
mph_check_symbol_exists(seekdir "dirent.h" HAVE_SEEKDIR)
mph_check_symbol_exists(telldir "dirent.h" HAVE_TELLDIR)
mph_check_symbol_exists(strerror_r "string.h" HAVE_STRERROR_R)
mph_check_symbol_exists(posix_fadvise "fcntl.h" HAVE_POSIX_FADVISE)
mph_check_symbol_exists(posix_fallocate "fcntl.h" HAVE_POSIX_FALLOCATE)
mph_check_symbol_exists(getgrnam_r "sys/types.h;grp.h" HAVE_GETGRNAM_R)
mph_check_symbol_exists(getgrgid_r "sys/types.h;grp.h" HAVE_GETGRGID_R)
mph_check_symbol_exists(getgrent "sys/types.h;grp.h" HAVE_GETGRENT)
mph_check_symbol_exists(setgrent "sys/types.h;grp.h" HAVE_SETGRENT)
mph_check_symbol_exists(fgetgrent "stdio.h;sys/types.h;grp.h" HAVE_FGETGRENT)
mph_check_symbol_exists(setgroups "sys/types.h;unistd.h;grp.h" HAVE_SETGROUPS)
mph_check_symbol_exists(getpwnam_r "sys/types.h;pwd.h" HAVE_GETPWNAM_R)
mph_check_symbol_exists(getpwuid_r "sys/types.h;pwd.h" HAVE_GETPWUID_R)
mph_check_symbol_exists(getpwent "sys/types.h;pwd.h" HAVE_GETPWENT)
mph_check_symbol_exists(fgetpwent "sys/types.h;pwd.h" HAVE_FGETPWENT)
mph_check_symbol_exists(setpwent "sys/types.h;pwd.h" HAVE_SETPWENT)
mph_check_symbol_exists(endpwent "sys/types.h;pwd.h" HAVE_ENDPWENT)
mph_check_symbol_exists(psignal "signal.h" HAVE_PSIGNAL)
mph_check_symbol_exists(signal "signal.h" HAVE_SIGNAL)
mph_check_symbol_exists(vsnprintf "stdio.h" HAVE_VSNPRINTF)
mph_check_symbol_exists(mlock "sys/mman.h" HAVE_MLOCK)
mph_check_symbol_exists(munlock "sys/mman.h" HAVE_MUNLOCK)
mph_check_symbol_exists(mremap "sys/mman.h" HAVE_MREMAP)
mph_check_symbol_exists(mincore "unistd.h;sys/mman.h" HAVE_MINCORE)
mph_check_symbol_exists(posix_madvise "sys/mman.h" HAVE_POSIX_MADVISE)
mph_check_symbol_exists(remap_file_pages "sys/mman.h" HAVE_REMAP_FILE_PAGES)
mph_check_symbol_exists(sendfile "sys/sendfile.h" HAVE_SENDFILE)

#
# Structure checks
#
check_struct_has_member("struct dirent" d_off "dirent.h" HAVE_STRUCT_DIRENT_D_OFF)
check_struct_has_member("struct dirent" d_reclen "dirent.h" HAVE_STRUCT_DIRENT_D_RECLEN)
check_struct_has_member("struct dirent" d_type "dirent.h" HAVE_STRUCT_DIRENT_D_TYPE)
check_struct_has_member("struct cmsghdr" cmsg_len "sys/socket.h" HAVE_STRUCT_CMSGHDR)
check_struct_has_member("struct flock" l_type "fcntl.h" HAVE_STRUCT_FLOCK)
check_struct_has_member("struct iovec" iov_base "sys/uio.h" HAVE_STRUCT_IOVEC)
check_struct_has_member("struct linger" l_onoff "sys/socket.h" HAVE_STRUCT_LINGER)
check_struct_has_member("struct pollfd" fd "poll.h" HAVE_STRUCT_POLLFD)
check_struct_has_member("struct sockaddr_in" sin_family "sys/socket.h;netinet/in.h" HAVE_STRUCT_SOCKADDR_IN)
check_struct_has_member("struct sockaddr_in6" sin6_family "sys/socket.h;netinet/in.h" HAVE_STRUCT_SOCKADDR_IN6)
check_struct_has_member("struct timespec" tv_sec "time.h" HAVE_STRUCT_TIMESPEC)
check_struct_has_member("struct timeval" tv_sec "time.h" HAVE_STRUCT_TIMEVAL)
check_struct_has_member("struct timezone" tz_minuteswest "sys/time.h" HAVE_STRUCT_TIMEZONE)
check_struct_has_member("struct utimbuf" actime "utime.h" HAVE_STRUCT_UTIMBUF)
check_struct_has_member("struct passwd" pw_gecos "pwd.h" HAVE_STRUCT_PASSWD_PW_GECOS)

#
# Include directories
#
include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/compat)

if(APPLE)
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
  if(SDK_SUPPORTS_ARM64)
    message(STATUS "SDK at ${XCODE_DEVELOPER_PATH} supports creation of ARM64 binaries")
    list(APPEND MPH_OSX_ARCHITECTURES "arm64")
  endif()

  if(SDK_SUPPORTS_X86_64)
    message(STATUS "SDK at ${XCODE_DEVELOPER_PATH} supports creation of X86_64 binaries")
    list(APPEND MPH_OSX_ARCHITECTURES "x86_64")
  endif()
endif()

add_library(
  ${MPH_LIB_NAME}
  SHARED
  ${MPH_SOURCES}
  )

if(APPLE)
  set_target_properties(
    ${MPH_LIB_NAME}
    PROPERTIES
    OSX_ARCHITECTURES "${MPH_OSX_ARCHITECTURES}"
    )
endif()

configure_file(config.h.in config.h @ONLY)
