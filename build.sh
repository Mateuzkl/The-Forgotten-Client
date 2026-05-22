#!/usr/bin/env bash
set -Eeuo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="$ROOT_DIR/build-linux"
OUTPUT="$BUILD_DIR/Tibia"
BUILD_TYPE="release"
RENDERER="software"
JOBS="$(nproc 2>/dev/null || echo 2)"
INSTALL_DEPS=1
CLEAN=0
ASAN=0

usage() {
	cat <<'USAGE'
Usage: ./build.sh [options]

Options:
  --debug              Build with debug symbols and no optimization.
  --asan               Build with AddressSanitizer/UndefinedBehaviorSanitizer.
  --opengl             Enable the OpenGL/OpenGL Core renderers.
  --software           Build only the software renderer (default).
  --skip-deps          Do not try to install missing Ubuntu packages.
  --non-interactive    Same as --skip-deps when sudo needs a password.
  --clean              Remove build-linux before compiling.
  --jobs N             Parallel compile jobs.
  --output PATH        Output executable path.
  -h, --help           Show this help.
USAGE
}

while [[ $# -gt 0 ]]; do
	case "$1" in
		--debug)
			BUILD_TYPE="debug"
			;;
		--asan)
			ASAN=1
			BUILD_TYPE="debug"
			;;
		--opengl)
			RENDERER="opengl"
			;;
		--software)
			RENDERER="software"
			;;
		--skip-deps|--non-interactive)
			INSTALL_DEPS=0
			;;
		--clean)
			CLEAN=1
			;;
		--jobs)
			shift
			JOBS="${1:-}"
			;;
		--output)
			shift
			OUTPUT="${1:-}"
			;;
		-h|--help)
			usage
			exit 0
			;;
		*)
			echo "Unknown option: $1" >&2
			usage
			exit 2
			;;
	esac
	shift
done

if [[ "$(uname -s)" != "Linux" ]]; then
	echo "This build script is intended for Linux/WSL. On Windows use the Visual Studio project." >&2
	exit 1
fi

if [[ "$CLEAN" -eq 1 ]]; then
	rm -rf "$BUILD_DIR"
fi
mkdir -p "$BUILD_DIR/obj"

APT_PACKAGES=(
	build-essential
	pkg-config
	libsdl2-dev
	libcurl4-openssl-dev
	zlib1g-dev
)

if [[ "$RENDERER" == "opengl" ]]; then
	APT_PACKAGES+=(libgl1-mesa-dev libglu1-mesa-dev)
fi

install_packages() {
	if ! command -v apt-get >/dev/null 2>&1; then
		echo "apt-get was not found. Install these packages manually: ${APT_PACKAGES[*]}" >&2
		return 1
	fi

	if ! sudo -n true >/dev/null 2>&1; then
		echo "Missing dependencies may need sudo. Install manually:" >&2
		echo "  sudo apt-get update && sudo apt-get install -y ${APT_PACKAGES[*]}" >&2
		return 1
	fi

	sudo apt-get update
	sudo apt-get install -y "${APT_PACKAGES[@]}"
}

missing=0
command -v g++ >/dev/null 2>&1 || missing=1
command -v gcc >/dev/null 2>&1 || missing=1
command -v pkg-config >/dev/null 2>&1 || missing=1
if command -v pkg-config >/dev/null 2>&1; then
	pkg-config --exists sdl2 || missing=1
	pkg-config --exists libcurl || missing=1
fi

if [[ "$missing" -ne 0 ]]; then
	if [[ "$INSTALL_DEPS" -eq 1 ]]; then
		install_packages
	else
		echo "Missing build dependencies. Install:" >&2
		echo "  sudo apt-get update && sudo apt-get install -y ${APT_PACKAGES[*]}" >&2
		exit 1
	fi
fi

if ! pkg-config --exists sdl2 libcurl; then
	echo "SDL2/libcurl development packages are still missing." >&2
	exit 1
fi

CXX="${CXX:-g++}"
CC="${CC:-gcc}"

COMMON_FLAGS=(
	-Isrc
	-DTFC_DISABLE_VULKAN=1
	-DTFC_DISABLE_GLES=1
	-D_7ZIP_ST=1
	-DTFC_DISABLE_FMA3=1
	-DTFC_DISABLE_FMA4=1
	-DTFC_DISABLE_AVX=1
	-Wall
	-Wextra
	-Wno-unused-parameter
	-Wno-unused-variable
	-Wno-missing-field-initializers
	-fno-strict-aliasing
)

case "$(uname -m)" in
	x86_64|i?86)
		COMMON_FLAGS+=(-mssse3 -msse4.1 -msse4.2)
		;;
esac

CXXFLAGS=(-std=c++17 "${COMMON_FLAGS[@]}")
CFLAGS=(-std=c99 "${COMMON_FLAGS[@]}")
LDFLAGS=()

if [[ "$BUILD_TYPE" == "debug" ]]; then
	CXXFLAGS+=(-O0 -g)
	CFLAGS+=(-O0 -g)
else
	CXXFLAGS+=(-O2 -DNDEBUG)
	CFLAGS+=(-O2 -DNDEBUG)
fi

if [[ "$ASAN" -eq 1 ]]; then
	CXXFLAGS+=(-fsanitize=address,undefined -fno-omit-frame-pointer)
	CFLAGS+=(-fsanitize=address,undefined -fno-omit-frame-pointer)
	LDFLAGS+=(-fsanitize=address,undefined)
fi

if [[ "$RENDERER" == "opengl" ]]; then
	CXXFLAGS+=(-DSDL_VIDEO_RENDER_OGL=1)
	CFLAGS+=(-DSDL_VIDEO_RENDER_OGL=1)
	LDFLAGS+=(-lGL)
else
	CXXFLAGS+=(-DTFC_DISABLE_OPENGL=1)
	CFLAGS+=(-DTFC_DISABLE_OPENGL=1)
fi

readarray -d '' ALL_SOURCES < <(find "$ROOT_DIR/src" -type f \( -name '*.cpp' -o -name '*.c' \) -print0 | sort -z)
SOURCES=()
for source in "${ALL_SOURCES[@]}"; do
	rel="${source#$ROOT_DIR/}"
	case "$rel" in
		src/elfbot_compat.cpp|\
		src/elfbot_shadow.cpp|\
		src/lzma/LzFindMt.c|\
		src/lzma/Threads.c|\
		src/surfaceDirect3D9.cpp|\
		src/surfaceDirect3D11.cpp|\
		src/surfaceDirectDraw.cpp|\
		src/surfaceOpengles.cpp|\
		src/surfaceOpengles2.cpp|\
		src/surfaceVulkan.cpp)
			continue
			;;
		src/surfaceOpengl.cpp|src/surfaceOpenglCore.cpp)
			[[ "$RENDERER" == "opengl" ]] || continue
			;;
	esac
	SOURCES+=("$source")
done

SDL_FLAGS="$(pkg-config --cflags --libs sdl2)"
CURL_FLAGS="$(pkg-config --cflags --libs libcurl)"

OBJECTS=()
compile_one() {
	local source="$1"
	local rel="${source#$ROOT_DIR/}"
	local object="$BUILD_DIR/obj/${rel%.*}.o"
	mkdir -p "$(dirname "$object")"

	if [[ "$source" == *.c ]]; then
		"$CC" "${CFLAGS[@]}" $SDL_FLAGS $CURL_FLAGS -c "$source" -o "$object"
	else
		"$CXX" "${CXXFLAGS[@]}" $SDL_FLAGS $CURL_FLAGS -c "$source" -o "$object"
	fi
}

echo "Building The Forgotten Client ($BUILD_TYPE, renderer=$RENDERER)"
echo "Sources: ${#SOURCES[@]}"

running=0
for source in "${SOURCES[@]}"; do
	rel="${source#$ROOT_DIR/}"
	object="$BUILD_DIR/obj/${rel%.*}.o"
	OBJECTS+=("$object")
	if [[ "$source" -nt "$object" ]]; then
		compile_one "$source" &
		((++running))
		if (( running >= JOBS )); then
			wait -n
			((--running))
		fi
	fi
done
wait

"$CXX" "${OBJECTS[@]}" $SDL_FLAGS $CURL_FLAGS "${LDFLAGS[@]}" -lz -lm -pthread -o "$OUTPUT"

echo "Built: $OUTPUT"
echo "Run from the repository root so data/, assets/, and config files resolve correctly."
