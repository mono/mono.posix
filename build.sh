#!/bin/bash -e
MY_NAME=$(basename "$0")
TRUE_PATH=$(readlink "$0" || echo "$0")
MY_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )"
OS=$(uname -s)
OS_LOWER=$(echo ${OS} | tr A-Z a-z)
BUILD_DIR_ROOT="${MY_DIR}/artifacts"
NATIVE_SOURCE_DIR="${MY_DIR}/src/native"
LOG_DIR="${BUILD_DIR_ROOT}/log"
ALL_TARGETS="host android ios tvos catalyst wasm managed test package"
VERBOSE="no"
JOBS=""
REBUILD="no"
PACKAGE_ALL_NATIVE_LIBS="no"
MANAGED_VERBOSITY="quiet"
MANAGED_VERBOSITY_VERBOSE="detailed"
CONFIGURATION="Release"
USE_COLOR="ON"
NO_BUILD="no"
CMAKE="cmake"
NINJA="ninja"
NDK_ROOT="${ANDROID_NDK_HOME:-${ANDROID_NDK_ROOT}}"

ANDROID_ABIS="arm32 arm64 x86 x64"
IOS_ABIS="armv7 armv7s arm64 simx64 simarm64"
TVOS_ABIS="arm64 simx64"
CATALYST_ABIS="arm64 x64"
MONO_POSIX_TEST_FRAMEWORKS="netcoreapp3.1 net6.0"

MANAGED_BUILT="no"
HOST_BUILT="no"
HOST_BUILD_NAME="host-${OS_LOWER}"
MANAGED_OUTPUT_DIR="${BUILD_DIR_ROOT}/bin"

# The color block is pilfered from the dotnet installer script
#
# Setup some colors to use. These need to work in fairly limited shells, like the Ubuntu Docker container where there are only 8 colors.
# See if stdout is a terminal
#
BRIGHT_YELLOW=""
BRIGHT_GREEN=""
BRIGHT_BLUE=""
BRIGHT_CYAN=""
BRIGHT_WHITE=""
NORMAL=""

if [ -t 1 ] && command -v tput > /dev/null; then
	# see if it supports colors
	ncolors=$(tput colors)
	if [ -n "$ncolors" ] && [ $ncolors -ge 8 ]; then
		BRIGHT_YELLOW="$(tput bold || echo)$(tput setaf 3 || echo)"
		BRIGHT_GREEN="$(tput bold || echo)$(tput setaf 2 || echo)"
		BRIGHT_BLUE="$(tput bold || echo)$(tput setaf 4 || echo)"
		BRIGHT_CYAN="$(tput bold || echo)$(tput setaf 6 || echo)"
		BRIGHT_WHITE="$(tput bold || echo)$(tput setaf 7 || echo)"
		NORMAL="$(tput sgr0 || echo)"
	fi
fi

function die()
{
	echo -e "$@" >&2
	exit 1
}

function usage()
{
	cat <<EOF
Usage: ${BRIGHT_WHITE}${MY_NAME}${NORMAL} ${BRIGHT_CYAN}[OPTIONS]${NORMAL} ${BRIGHT_CYAN}[TARGETS]${NORMAL}

Where OPTIONS is one or more of:

  -c, --configuration NAME  build in configuration 'NAME' (Release or Debug, default is ${CONFIGURATION})
  -b, --no-color            do not use color for native compiler diagnostics
  -v, --verbose             make the build verbose
  -V, --managed-verbosity   managed build/test verbosity. Defaults to '${MANAGED_VERBOSITY}' for normal builds
                            and '${MANAGED_VERBOSITY_VERBOSE}' for verbose ('-v') builds. Available values:

                               q[uiet], m[inimal], n[ormal], d[etailed], and diag[nostic]

  -m, --cmake PATH          use the specified cmake binary instead of the default '${CMAKE}'
  -n, --ninja PATH          use the specified ninja binary instead of the default '${NINJA}'
  -j, --jobs NUM            spin up at most NUM jobs
  -r, --rebuild             rebuild from scratch
  -a, --ndk PATH            path to the Android NDK root directory (default: ${NDK_ROOT:-UNSET})
  -p, --abi ABIS            comma-separated list of target platform ABIs to build:
                               Android default: ${ANDROID_ABIS}
                               iOS default: ${IOS_ABIS}
                               tvOS default: ${TVOS_ABIS}
                               catalyst default: ${CATALYST_ABIS}
  -x, --no-build            do not build native runtime (for the 'test' target) or the managed code
                            (for the 'package' target). The necessary native and managed libraries must
                            be present in their correct location.  This optin is mostly useful for CI.
  -e, --pack-all            package all the native libraries when running the 'package' target, not just
                            those built locally. Default: ${PACKAGE_ALL_NATIVE_LIBS}
  -h, --help                show help

And TARGETS is one or more of:

  host                 build native library for the current OS
  android              build native library for Android
  ios                  build native library for iOS
  tvos                 build native library for tvOS
  catalyst             build native library for macOS Catalyst
  wasm                 build native library for WASM
  managed              build the managed library
  test                 run tests on the host OS
  package              package built artifacts into nugets
  all                  build all the targets above
EOF
	exit 0
}

function print_build_banner()
{
	echo ${NORMAL}
	echo ${BRIGHT_GREEN}"*************************************************************"
	echo ${BRIGHT_YELLOW}"$@"
	echo ${BRIGHT_GREEN}"*************************************************************"
	echo ${NORMAL}
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
	fi
	"${make_program}" ${verbose} ${jobs}
}

function get_build_dir()
{
	local build_dir_name="${1}"
	local config_lower=$(echo ${CONFIGURATION} | tr A-Z a-z)

	echo "${BUILD_DIR_ROOT}/${build_dir_name}-${config_lower}"
}

function build_common()
{
	local build_dir_name="${1}"

	if [ -z "${build_dir_name}" ]; then
		die build_common needs name of the build directory
	fi

	shift

	local build_dir="$(get_build_dir "${build_dir_name}")"
	if [ "${REBUILD}" == "yes" -a -d "${build_dir}" ]; then
		rm -rf "${build_dir}"
	fi
	install -d -m 755 "${build_dir}"
	(cd "${build_dir}"; do_build "$@")
}

function __build_host()
{
	print_build_banner_native "${OS}"

	if [ "${OS}" == "Darwin" ]; then
		build_common ${HOST_BUILD_NAME}-x64 -DTARGET_PLATFORM=host-${OS_LOWER}-x64
		build_common ${HOST_BUILD_NAME}-arm64 -DTARGET_PLATFORM=host-${OS_LOWER}-arm64
	else
		build_common ${HOST_BUILD_NAME}-x64 -DTARGET_PLATFORM=host-${OS_LOWER}-x64
		build_common ${HOST_BUILD_NAME}-arm64 -DTARGET_PLATFORM=host-${OS_LOWER}-arm64
		build_common ${HOST_BUILD_NAME}-arm32 -DTARGET_PLATFORM=host-${OS_LOWER}-arm32
	fi

	HOST_BUILT="yes"
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

function __build_tvos()
{
	print_build_banner_native tvOS

	local abi_lower
	for abi in ${ABIS:-${TVOS_ABIS}}; do
		abi_lower=$(echo ${abi} | tr A-Z a-z)
		build_common tvos-${abi_lower} -DTARGET_PLATFORM=tvos-${abi_lower}
	done
}

function __build_catalyst()
{
	print_build_banner_native "macOS Catalyst"

	local abi_lower
	for abi in ${ABIS:-${CATALYST_ABIS}}; do
		abi_lower=$(echo ${abi} | tr A-Z a-z)
		build_common catalyst-${abi_lower} -DTARGET_PLATFORM=catalyst-${abi_lower}
	done
}

function __build_wasm()
{
	print_build_banner_native WASM
}

function __build_managed()
{
	local verbosity

	if [ "${VERBOSE}" == "yes" ]; then
		verbosity="detailed"
	else
		verbosity="quiet"
	fi

	print_build_banner Building managed library
	${MY_DIR}/eng/common/build.sh \
			--restore \
			--build \
			--configuration "${CONFIGURATION}" \
			--verbosity "${MANAGED_BUILD_VERBOSITY}" \
			--projects ${MY_DIR}/Mono.Unix.sln \
			"/bl:LogFile=${LOG_DIR}/${CONFIGURATION}/Mono.Unix-build.binlog"

	print_build_banner Building managed tests
	${MY_DIR}/eng/common/build.sh \
			--restore \
			--build \
			--configuration "${CONFIGURATION}" \
			--verbosity "${MANAGED_BUILD_VERBOSITY}" \
			--projects ${MY_DIR}/Mono.Unix.Tests.sln \
			"/bl:LogFile=${LOG_DIR}/${CONFIGURATION}/Mono.Unix.Tests-build.binlog"

	MANAGED_BUILT="yes"
}

function build_if_necessary()
{
	if [ "${NO_BUILD}" == "no" ]; then
		if [ "${HOST_BUILT}" == "no" ]; then
			__build_host
		fi

		if [ "${MANAGED_BUILT}" == "no" ]; then
			__build_managed
		fi
	fi
}

function __build_test()
{
	build_if_necessary

	local verbose=""
	if [ "${VERBOSE}" == "yes" ]; then
		verbose="-v d"
	fi

	local arch=""
	if [ "${OS}" == "Darwin" ]; then
		case $(arch) in
			i386) arch="-x64" ;;
			arm64) arch="-arm64" ;;
			*) die "Unknown macOS host architecture: $(arch)" ;;
		esac
	elif [ "${OS}" == "Linux" ]; then
		case $(arch) in
			i686) die 32-bit x86 Linux build not supported ;;
			x86_64) arch="-x64" ;;
			armv7l) arch="-arm32" ;;
			arm64) arch="-arm64" ;;
			*) die "Unknown Linux host architecture: $(arch)" ;;
		esac
	fi

	local host_build_dir="$(get_build_dir "${HOST_BUILD_NAME}${arch}")"
	local something_failed=no
	print_build_banner Running tests
	set -x
	for framework in ${MONO_POSIX_TEST_FRAMEWORKS}; do
		local managed_build_dir="${MANAGED_OUTPUT_DIR}/Mono.Unix.Tests/${CONFIGURATION}/${framework}"
		mkdir -p "${managed_build_dir}"
		cp "${host_build_dir}/lib"/libMono.Unix.* "${managed_build_dir}"
	done

	${MY_DIR}/eng/common/build.sh \
			--restore \
			--test \
			--configuration "${CONFIGURATION}" \
			--projects ${MY_DIR}/Mono.Unix.Tests.sln || something_failed=yes
	set +x

	if [ "${something_failed}" == "yes" ]; then
		die Some tests failed
	fi
}

function __build_package()
{
	print_build_banner Packaging

	build_if_necessary

	local build_action="--build"
	if [ "${NO_BUILD}" == "yes" ]; then
		build_action=""
	fi

	if [ "${PACKAGE_ALL_NATIVE_LIBS}" == "yes" ]; then
		export PackageAllNativeLibs="true"
	else
		export PackageAllNativeLibs="false"
	fi

	${MY_DIR}/eng/common/build.sh \
			--restore \
			${build_action} \
			--pack \
			--configuration "${CONFIGURATION}" \
			--projects ${MY_DIR}/Mono.Unix.sln \
			"/bl:LogFile=${LOG_DIR}/${CONFIGURATION}/Mono.Unix-package.binlog"
}

function missing_argument()
{
	die "Option '$1' requires an argument"
}

POSITIONAL_ARGS=""
ABIS=""
MANAGED_BUILD_VERBOSITY=""
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
				ABIS="$(echo $2 | tr ',' ' ')";
				shift 2
			else
				missing_argument "$1"
			fi
			;;

		-a|--ndk)
			if [ -n "$2" ] && [ ${2:0:1} != "-" ]; then
				NDK_ROOT="$2"
				shift 2
			else
				missing_argument "$1"
			fi
			;;

		-V|--managed-verbosity)
			if [ -n "$2" ] && [ ${2:0:1} != "-" ]; then
				MANAGED_BUILD_VERBOSITY="$(echo $2 | tr ',' ' ')";
				shift 2
			else
				missing_argument "$1"
			fi
			;;

		-x|--no-build) NO_BUILD="yes"; shift ;;

		-e|--pack-all) PACKAGE_ALL_NATIVE_LIBS="yes"; shift ;;

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

if [ -z "${MANAGED_BUILD_VERBOSITY}" ]; then
	if [ "${VERBOSE}" == "no" ]; then
		MANAGED_BUILD_VERBOSITY="${MANAGED_VERBOSITY}"
	else
		MANAGED_BUILD_VERBOSITY="${MANAGED_VERBOSITY_VERBOSE}"
	fi
fi

if [ $# -eq 0 ]; then
	TARGET="host"
else
	TARGET=""
	for t in "${@}"; do
		if [ "${t}" == "all" ]; then
			TARGET="${ALL_TARGETS}"
			break
		fi

		TARGET="${TARGET} ${t}"
	done
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
