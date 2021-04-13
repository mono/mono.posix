#!/bin/bash -e
MY_NAME=$(basename "$0")
TRUE_PATH=$(readlink "$0" || echo "$0")
MY_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )"
OS=$(uname -s)
OS_LOWER=$(echo ${OS} | tr A-Z a-z)
BUILD_DIR_ROOT="${MY_DIR}/build"
NATIVE_SOURCE_DIR="${MY_DIR}/src/native"
LOG_DIR="${MY_DIR}/build/logs"
ALL_TARGETS="host android ios tvos catalyst wasm managed test package"
VERBOSE="no"
JOBS=""
REBUILD="no"
MANAGED_VERBOSITY="quiet"
MANAGED_VERBOSITY_VERBOSE="detailed"
CONFIGURATION="Release"
USE_COLOR="ON"
USE_NET6="no"
CMAKE="cmake"
NINJA="ninja"
XCODEBUILD="xcodebuild"
NDK_ROOT="${ANDROID_NDK_HOME:-${ANDROID_NDK_ROOT}}"

ANDROID_ABIS="arm32 arm64 x86 x64"
IOS_ABIS="armv7 armv7s arm64 simx64 simarm64"
TVOS_ABIS="arm64 simx64"
CATALYST_ABIS="arm64 x64"
MONO_POSIX_FRAMEWORKS="netstandard2.1 netcoreapp3.1"
MONO_POSIX_TEST_FRAMEWORKS="netcoreapp3.1"

MANAGED_BUILT="no"
HOST_BUILT="no"
HOST_BUILD_NAME="host-${OS_LOWER}"
MANAGED_OUTPUT_DIR="${BUILD_DIR_ROOT}/managed"

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
  -x, --xcodebuild PATH     use the specified xcodebuild binary instead of the default '${XCODEBUILD}'
  -j, --jobs NUM            spin up at most NUM jobs
  -r, --rebuild             rebuild from scratch
  -a, --ndk PATH            path to the Android NDK root directory (default: ${NDK_ROOT:-UNSET})
  -p, --abi ABIS            comma-separated list of target platform ABIs to build:
                               Android default: ${ANDROID_ABIS}
                               iOS default: ${IOS_ABIS}
                               tvOS default: ${TVOS_ABIS}
                               catalyst default: ${CATALYST_ABIS}
  -6, --net6                support NET6 (default: ${USE_NET6})

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
		build_common ${HOST_BUILD_NAME} -DTARGET_PLATFORM=host-${OS_LOWER}
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

	local use_net6

	if [ "${USE_NET6}" == "yes" ]; then
		use_net6="/p:UseNet6=True"
	fi

	for framework in ${MONO_POSIX_FRAMEWORKS}; do
		print_build_banner Building managed library for framework ${framework}
		dotnet build \
			   -f "${framework}" \
			   --configuration "${CONFIGURATION}" \
			   --verbosity "${MANAGED_BUILD_VERBOSITY}" \
			   Mono.Unix.sln \
			   "/bl:LogFile=${LOG_DIR}/Mono.Unix-build-${framework}.binlog" ${use_net6}
	done

	for framework in ${MONO_POSIX_TEST_FRAMEWORKS}; do
		print_build_banner Building managed tests for framework ${framework}
		dotnet build \
			   -f "${framework}" \
			   --configuration "${CONFIGURATION}" \
			   --verbosity "${MANAGED_BUILD_VERBOSITY}" \
			   Mono.Unix.Test.sln \
			   "/bl:LogFile=${LOG_DIR}/Mono.Unix.Test-build-${framework}.binlog" ${use_net6}
	done

	MANAGED_BUILT="yes"
}

function __build_test()
{
	if [ "${HOST_BUILT}" == "no" ]; then
		__build_host
	fi

	if [ "${MANAGED_BUILT}" == "no" ]; then
		__build_managed
	fi

	local verbose=""
	if [ "${VERBOSE}" == "yes" ]; then
		verbose="-v d"
	fi

	local use_net6

	if [ "${USE_NET6}" == "yes" ]; then
		use_net6="/p:UseNet6=True"
	fi

	local arch=""
	local filter=""
	if [ "${OS}" == "Darwin" ]; then
		filter="--filter \"Category!=NotOnMac\""
		case $(arch) in
			i386) arch="-x64" ;;
			arm64) arch="-arm64" ;;
			*) die "Unknown host architecture: $(arch)" ;;
		esac
	fi

	local host_build_dir="$(get_build_dir "${HOST_BUILD_NAME}${arch}")"
	local managed_top_dir="${MANAGED_OUTPUT_DIR}/${CONFIGURATION}"
	local something_failed=no
	for framework in ${MONO_POSIX_TEST_FRAMEWORKS}; do
		print_build_banner Running tests for framework ${framework}
		cp "${host_build_dir}/lib"/libMono.Unix.* "${managed_top_dir}/${framework}"
		set -x
		dotnet test ${verbose} ${filter} \
			   -f "${framework}" \
			   --verbosity "${MANAGED_BUILD_VERBOSITY}" \
			   --configuration "${CONFIGURATION}" \
			   --logger:"console;verbosity=detailed" \
			   --logger "trx;LogFileName=${LOG_DIR}/Mono.Unix.Test-${framework}.trx" \
			   Mono.Unix.Test.sln ${use_net6} || something_failed=yes
		set +x
	done

	if [ "${something_failed}" == "yes" ]; then
		die Some tests failed
	fi
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

		-6|--net6) USE_NET6="yes" ;;

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

if [ "${USE_NET6}" == "yes" ]; then
	MONO_POSIX_FRAMEWORKS="${MONO_POSIX_FRAMEWORKS} net6.0"
	MONO_POSIX_TEST_FRAMEWORKS="${MONO_POSIX_TEST_FRAMEWORKS} net6.0"
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
