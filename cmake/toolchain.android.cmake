if(TARGET_PLATFORM STREQUAL android)
  include(${CMAKE_CURRENT_LIST_DIR}/versions.android.cmake)

  #
  # MPH_ANDROID_ABI, one of (each line is a set of alternative names for the given ABI):
  #
  #   arm64-v8a, arm64, aarch64
  #   armeabi-v7a, arm32
  #   x86
  #   x86_64, x64
  #

  if(NOT DEFINED MPH_ANDROID_ABI)
    message(FATAL_ERROR "Please provide Android ABI by setting the MPH_ANDROID_ABI property on command line")
  endif()

  if(NOT ANDROID_NDK)
    message(FATAL_ERROR "Please provide path to the Android NDK by setting the ANDROID_NDK property on command line")
  endif()

  if(MPH_ANDROID_ABI MATCHES "^arm64-v8a$|^arm64$|^aarch64$")
    set(ANDROID_ABI "arm64-v8a")
  elseif(MPH_ANDROID_ABI MATCHES "^armeabi-v7a$|^arm32$")
    set(ANDROID_ABI "armeabi-v7a")
  elseif(MPH_ANDROID_ABI MATCHES "^x86$")
    set(ANDROID_ABI "x86")
  elseif(MPH_ANDROID_ABI MATCHES "^x86_64$|^x64$")
    set(ANDROID_ABI "x86_64")
  else()
    message(FATAL "Unknown Android ABI name: ${MPH_ANDROID_ABI}")
  endif()

  set(MPH_API_LEVEL "${ANDROID_API_${ANDROID_ABI}}")
  set(ANDROID_STL "none")
  set(ANDROID_CPP_FEATURES "no-rtti no-exceptions")
  set(ANDROID_NATIVE_API_LEVEL "${MPH_API_LEVEL}")
  set(ANDROID_PLATFORM "android-${MPH_API_LEVEL}")

  include(${ANDROID_NDK}/build/cmake/android.toolchain.cmake)
endif()
