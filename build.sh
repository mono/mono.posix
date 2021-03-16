#!/bin/bash -e
MY_NAME=$(basename "$0")
TRUE_PATH=$(readlink "$0" || echo "$0")
MY_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )"
OS=$(uname -s)
BUILD_DIR_ROOT="${MY_DIR}/build"
NATIVE_SOURCE_DIR="${MY_DIR}/src/native"
LOG_DIR="${MY_DIR}/logs"
ALL_TARGETS="host android ios wasm managed package"
VERBOSE="no"
JOBS=""
REBUILD="no"
CONFIGURATION="Release"
USE_COLOR="ON"
CMAKE="cmake"
NINJA="ninja"
NDK_ROOT="${ANDROID_NDK_HOME:-${ANDROID_NDK_ROOT}}"

ANDROID_ABI_ARM32=armeabi-v7a
ANDROID_ABI_ARM64=arm64-v8a
ANDROID_ABI_X86=x86
ANDROID_ABI_X64=x86_64

#
# NOTE: API levels must be compatible with the currently supported Xamarin.Android versions
#
ANDROID_ABIS="${ANDROID_ABI_ARM32} ${ANDROID_ABI_ARM64} ${ANDROID_ABI_X86} ${ANDROID_ABI_X64}"
ANDROID_API_ARM32=16
ANDROID_API_ARM64=21
ANDROID_API_X86=16
ANDROID_API_X64=21


function die()
{
	echo -e "$@" >&2
	exit 1
}

function usage()
{
	cat <<EOF
Usage: ${MY_NAME} [OPTIONS] [TARGET]

Where OPTIONS are:

  -c, --configuration NAME  build in configuration 'NAME' (Release or Debug, default is ${CONFIGURATION})
  -b, --no-color            do not use color for native compiler diagnostics
  -v, --verbose             make the build verbose
  -m, --cmake PATH          use the specified cmake binary instead of the default '${CMAKE}'
  -n, --ninja PATH          use the specified ninja binary instead of the default '${NINJA}'
  -j, --jobs NUM            spin up at most NUM jobs
  -r, --rebuild             rebuild from scratch
  -a, --ndk PATH            path to the Android NDK root directory (default: ${NDK_ROOT:-UNSET})
  -p, --abi ABIS            comma-separated list of Android ABIs to build (default: ${ANDROID_ABIS})
  -h, --help                show help

And TARGET is one of:

  host                 build native library for the current OS
  android              build native library for Android
  ios                  build native library for iOS
  wasm                 build native library for WASM
  managed              build the managed library
  package              package built artifacts into nugets
  all                  build all the targets above
EOF
	exit 0
}

function get_android_api()
{
	local abi="$1"

	case "${abi}" in
		${ANDROID_ABI_ARM32}) echo ${ANDROID_API_ARM32} ;;
		${ANDROID_ABI_ARM64}) echo ${ANDROID_API_ARM64} ;;
		${ANDROID_ABI_X86}) echo ${ANDROID_API_X86} ;;
		${ANDROID_ABI_X64}) echo ${ANDROID_API_X64} ;;
		*) die "Unknown Android ABI '${abi}'"
	esac
}

function print_build_banner()
{
	echo "$@"
}

function print_build_banner_native()
{
	print_build_banner "Building native library for ${1}"
}

function do_build()
{
	local strip_debug

	if [ "${CONFIGURATION}" == "Release" ]; then
		strip_debug="ON"
	else
		strip_debug="OFF"
	fi

	"${CMAKE}" -GNinja \
			   -DCMAKE_BUILD_TYPE=${CONFIGURATION} \
			   -DCMAKE_MAKE_PROGRAM="${NINJA}" \
			   -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
			   -DSTRIP_DEBUG=${strip_debug} \
			   -DUSE_COLOR=${USE_COLOR} \
			   "$@" \
			   "${NATIVE_SOURCE_DIR}"

	local ninja_verbose=""
	local ninja_jobs=""
	if [ "${VERBOSE}" == "yes" ]; then
		ninja_verbose="-v"
	fi

	if [ -n "${JOBS}" ]; then
		ninja_jobs="-j${JOBS}"
	fi
	"${NINJA}" ${ninja_verbose} ${ninja_jobs}
}

function build_common()
{
	local build_dir_name="${1}"

	if [ -z "${build_dir_name}" ]; then
		die build_common needs name of the build directory
	fi

	shift

	local build_dir="${BUILD_DIR_ROOT}/${build_dir_name}-${CONFIGURATION}"
	if [ "${REBUILD}" == "yes" -a -d "${build_dir}" ]; then
		rm -rf "${build_dir}"
	fi
	install -d -m 755 "${build_dir}"
	(cd "${build_dir}"; do_build "$@")
}

function __build_host()
{
	print_build_banner_native "${OS}"
	build_common host
}

function __build_android()
{
	print_build_banner_native Android

	if [ -z "${NDK_ROOT}" ]; then
		die "Android NDK root not specified, please set the ANDROID_NDK_ROOT or ANDROID_NDK_HOME environment variable\nOr pass NDK path to the script using the '--ndk' parameter"
	fi

	if [ ! -d "${NDK_ROOT}" ]; then
		die "Android NDK directory '${NDK_ROOT}' does not exist"
	fi

	local api_level
	for abi in ${ANDROID_ABIS}; do
		api_level=$(get_android_api ${abi})
		build_common android-${abi} \
					 -DCMAKE_TOOLCHAIN_FILE="${NDK_ROOT}/build/cmake/android.toolchain.cmake" \
					 -DANDROID_STL="none" \
					 -DANDROID_CPP_FEATURES="no-rtti no-exceptions" \
					 -DANDROID_NDK="${NDK_ROOT}" \
					 -DANDROID_NATIVE_API_LEVEL=${api_level} \
					 -DANDROID_PLATFORM=android-${api_level} \
					 -DANDROID_ABI=${abi}
	done
}

function __build_ios()
{
	print_build_banner_native iOS
}

function __build_wasm()
{
	print_build_banner_native WASM
}

function __build_managed()
{
	print_build_banner Building managed library
}

function __build_package()
{
	print_build_banner Packaging
}

function missing_argument()
{
	die "Option '$1' requires an argument"
}

POSITIONAL_ARGS=""
while (( "$#" )); do
    case "$1" in
		-n|--ninja)
			if [ -n "$2" ] && [ ${2:0:1} != "-" ]; then
				NINJA="$2";
				shift 2
			else
				missing_argument "$1"
			fi
			;;

		-m|--cmake)
			if [ -n "$2" ] && [ ${2:0:1} != "-" ]; then
				CMAKE="$2";
				shift 2
			else
				missing_argument "$1"
			fi
			;;

		-j|--jobs)
			if [ -n "$2" ] && [ ${2:0:1} != "-" ]; then
				JOBS="$2";
				shift 2
			else
				missing_argument "$1"
			fi
			;;

		-c|--configuration)
			if [ -n "$2" ] && [ ${2:0:1} != "-" ]; then
				CONFIGURATION="$2";
				shift 2
			else
				missing_argument "$1"
			fi
			;;

		-p|--abi)
			if [ -n "$2" ] && [ ${2:0:1} != "-" ]; then
				ANDROID_ABIS="$(echo $2 | tr ',' ' ')";
				shift 2
			else
				missing_argument "$1"
			fi
			;;

		-v|--verbose) VERBOSE="yes"; shift ;;

		-b|--no-color) USE_COLOR=OFF; shift ;;

		-r|--rebuild) REBUILD="yes"; shift;;

        -h|--help) usage; shift ;;

        -*|--*=) die "Error: Unsupported flag $1";;

		*) POSITIONAL_ARGS="${POSITIONAL_ARGS} $1"; shift;;
    esac
done

eval set -- "${POSITIONAL_ARGS}"

if [ -z "${CMAKE}" ]; then
	die cmake binary must be specified
fi

if [ -z "${NINJA}" ]; then
	die ninja binary must be specified
fi

if [ $# -eq 0 ]; then
	TARGET="host"
else
	TARGET=${1}
	if [ "${TARGET}" == "all" ]; then
		TARGET="${ALL_TARGETS}"
	fi
fi

for T in $TARGET; do
	TARGET_LOWER="$(echo $T | tr A-Z a-z)"
	func_name="__build_${TARGET_LOWER}"
	if [ "$(builtin type -type $func_name)" = "function" ]; then
		eval $func_name
	else
		die "Target '$T' unknown"
	fi
done
