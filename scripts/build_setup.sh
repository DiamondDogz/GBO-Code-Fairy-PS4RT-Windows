#!/usr/bin/env bash
# SPDX-License-Identifier: GPL-2.0-or-later
# Rebuild only the standalone PS4RT player setup UI.
set -euo pipefail
cd -- "$(dirname -- "$0")/.."
case $(uname -s) in
MINGW*|MSYS*)
    # shellcheck source=msys2-env.sh
    source ./msys2-env.sh
    ;;
esac

pkg-config --exists sdl3 libavcodec libavutil libswscale || {
    echo "SDL3/FFmpeg development files are missing (already required by a source build of PS4RT)." >&2
    exit 1
}
mkdir -p out
read -r -a setup_cflags <<< "$(pkg-config --cflags sdl3 libavcodec libavutil libswscale)"
read -r -a setup_libs <<< "$(pkg-config --libs sdl3 libavcodec libavutil libswscale)"

portable_libs=()
if [[ ${PS4RT_PORTABLE_BUILD:-0} == 1 ]]; then
    portable_libs=(-L/usr/lib/x86_64-linux-gnu
        -Wl,-rpath-link,/opt/ps4rt-deps/lib
        -Wl,-rpath-link,/usr/lib/x86_64-linux-gnu
        -Wl,-rpath-link,/lib/x86_64-linux-gnu)
fi
setup_out=out/ps4rt-setup
setup_pthread=(-pthread)
case $(uname -s) in
MINGW*|MSYS*) setup_out=out/ps4rt-setup.exe; setup_pthread=(); portable_libs=() ;;
esac
"${CXX:-c++}" -std=c++17 -O2 -Wall -Wextra "${setup_pthread[@]}" \
    "${setup_cflags[@]}" -Igpu/third_party/imgui -Igpu/third_party/imgui/backends -Igpu/shim \
    scripts/ps4rt_setup_native.cpp gpu/shim/ps4rt_png.cpp \
    gpu/third_party/imgui/imgui.cpp gpu/third_party/imgui/imgui_draw.cpp \
    gpu/third_party/imgui/imgui_tables.cpp gpu/third_party/imgui/imgui_widgets.cpp \
    gpu/third_party/imgui/backends/imgui_impl_sdl3.cpp \
    gpu/third_party/imgui/backends/imgui_impl_sdlrenderer3.cpp \
    "${setup_libs[@]}" "${portable_libs[@]}" -o "$setup_out"
echo "PS4RT setup executable: out/ps4rt-setup"
