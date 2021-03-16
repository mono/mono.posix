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
XCODEBUILD="xcodebuild"
NDK_ROOT="${ANDROID_NDK_HOME:-${ANDROID_NDK_ROOT}}"

ANDROID_ABIS="arm32 arm64 x86 x64"
IOS_ABIS="armv7 armv7s arm64 simx86 simarm64"

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
  -x, --xcodebuild PATH     use the specified xcodebuild binary instead of the default '${XCODEBUILD}'
  -j, --jobs NUM            spin up at most NUM jobs
  -r, --rebuild             rebuild from scratch
  -a, --ndk PATH            path to the Android NDK root directory (default: ${NDK_ROOT:-UNSET})
  -p, --abi ABIS            comma-separated list of target platform ABIs to build:
                               Android default: ${ANDROID_ABIS}
                               iOS default: ${IOS_ABIS}

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
	local generator="${CMAKE_GENERATOR:-Ninja}"
	local make_program="${CMAKE_MAKE:-${NINJA}}"
	
	if [ "${CONFIGURATION}" == "Release" ]; then
		strip_debug="ON"
	else
		strip_debug="OFF"
	fi

	"${CMAKE}" -G${generator} \
			   -DCMAKE_BUILD_TYPE=${CONFIGURATION} \
			   -DCMAKE_MAKE_PROGRAM="${make_program}" \
			   -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
			   -DSTRIP_DEBUG=${strip_debug} \
			   -DUSE_COLOR=${USE_COLOR} \
			   "$@" \
			   "${NATIVE_SOURCE_DIR}"

	local verbose=""
	local jobs=""

	if [ "${make_program}" == "${NINJA}" ]; then
		if [ "${VERBOSE}" == "yes" ]; then
			verbose="-v"
		fi

		if [ -n "${JOBS}" ]; then
			jobs="-j${JOBS}"
		fi
	elif [ "${make_program}" == "${xcodebuild}" ]; then
		if [ "${VERBOSE}" == "yes" ]; then
			verbose="-verbose"
		fi

		if [ -n "${JOBS}" ]; then
			jobs="-jobs ${JOBS}"
		fi
	fi
	"${make_program}" ${verbose} ${jobs}
}

function build_common()
{
	local build_dir_name="${1}"

	if [ -z "${build_dir_name}" ]; then
		die build_common needs name of the build directory
	fi

	shift

	local config_lower=$(echo ${CONFIGURATION} | tr A-Z a-z)
	local build_dir="${BUILD_DIR_ROOT}/${build_dir_name}-${config_lower}"
	if [ "${REBUILD}" == "yes" -a -d "${build_dir}" ]; then
		rm -rf "${build_dir}"
	fi
	install -d -m 755 "${build_dir}"
	(cd "${build_dir}"; do_build "$@")
}

function __build_host()
{
	local OS_LOWER=$(echo ${OS} | tr A-Z a-z)
	print_build_banner_native "${OS}"
	build_common host -DTARGET_PLATFORM=host-${OS_LOWER}
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
		build_common android-${abi} \
					 -DTARGET_PLATFORM=android \
					 -DMPH_ANDROID_ABI=${abi} \
					 -DANDROID_NDK="${NDK_ROOT}"
	done
}

function __build_ios()
{
	print_build_banner_native iOS

	local abi_lower
	for abi in ${ABIS:-${IOS_ABIS}}; do
		abi_lower=$(echo ${abi} | tr A-Z a-z)
		build_common ios-${abi_lower} -DTARGET_PLATFORM=ios-${abi_lower}
	done
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
ABIS=""
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

		-x|--xcodebuild)
			if [ -n "$2" ] && [ ${2:0:1} != "-" ]; then
				XCODEBUILD="$2";
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
				ABIS="$(echo $2 | tr ',' ' ')";
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
