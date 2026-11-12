#!/usr/bin/env bash
# SPDX-License-Identifier: GPL-2.0-or-later
# Maintainer/build-time only: PS4RT-owned native PKG extractor.
# Players receive the binary and bundled Crypto++ runtime, with no installs.
set -euo pipefail
cd -- "$(dirname -- "$0")/.."
case $(uname -s) in
MINGW*|MSYS*)
    # shellcheck source=msys2-env.sh
    source ./msys2-env.sh
    ;;
esac

# Fedora names the pkg-config module libcryptopp; Debian/Ubuntu use libcrypto++.
crypto_pc=libcryptopp
if ! pkg-config --exists "$crypto_pc"; then
    crypto_pc='libcrypto++'
fi
missing=()
if ! pkg-config --exists "$crypto_pc"; then
    missing+=(Crypto++ )
fi
if ! pkg-config --exists zlib; then
    missing+=(zlib )
fi
if (( ${#missing[@]} )); then
    echo "Missing native PKG build dependency: ${missing[*]}" >&2
    echo "Check each with: pkg-config --modversion libcryptopp zlib" >&2
    echo "Fedora packages: cryptopp-devel (Crypto++), zlib-devel (zlib)." >&2
    echo "These are build-only dependencies; player releases bundle runtime libraries." >&2
    exit 1
fi

portable_cflags=()
portable_libs=()
if [[ ${PS4RT_PORTABLE_BUILD:-0} == 1 ]]; then
    # The Conda GCC 14 toolchain uses its own glibc sysroot. Crypto++ and zlib
    # were installed by apt into the builder's /usr/include, not that sysroot.
    # Search those package headers last to retain the controlled sysroot first.
    portable_cflags=(-idirafter /usr/include)
    portable_libs=(-L/usr/lib/x86_64-linux-gnu
        -Wl,-rpath-link,/opt/ps4rt-deps/lib
        -Wl,-rpath-link,/usr/lib/x86_64-linux-gnu
        -Wl,-rpath-link,/lib/x86_64-linux-gnu)
fi
mkdir -p out/lib
case $(uname -s) in
MINGW*|MSYS*)
    "${CXX:-c++}" -std=c++23 -O2 -Wall -Wextra \
        $(pkg-config --cflags "$crypto_pc" zlib) -Ithird_party/ps4pkg \
        scripts/ps4rt_pkg_native.cpp \
        third_party/ps4pkg/core/file_format/pkg.cpp \
        third_party/ps4pkg/core/file_format/pkg_type.cpp \
        third_party/ps4pkg/core/crypto/crypto.cpp \
        $(pkg-config --libs "$crypto_pc" zlib) -o out/ps4rt-pkg.exe
    shopt -s nullglob
    crypt_dlls=(/clang64/bin/libcryptopp*.dll /clang64/bin/libcrypto++*.dll)
    if (( ${#crypt_dlls[@]} == 0 )); then
        echo "Could not locate the Crypto++ DLL to bundle." >&2
        exit 1
    fi
    cp -f "${crypt_dlls[@]}" out/
    echo "PS4RT native PKG extractor: out/ps4rt-pkg.exe"
    echo "Bundled Crypto++ runtime: out/$(basename "${crypt_dlls[0]}")"
    exit 0
    ;;
esac
"${CXX:-c++}" -std=c++23 -O2 -Wall -Wextra -pthread \
    $(pkg-config --cflags "$crypto_pc" zlib) "${portable_cflags[@]}" -Ithird_party/ps4pkg \
    scripts/ps4rt_pkg_native.cpp \
    third_party/ps4pkg/core/file_format/pkg.cpp \
    third_party/ps4pkg/core/file_format/pkg_type.cpp \
    third_party/ps4pkg/core/crypto/crypto.cpp \
    $(pkg-config --libs "$crypto_pc" zlib) "${portable_libs[@]}" '-Wl,-rpath,$ORIGIN/lib' -o out/ps4rt-pkg

# Bundle the exact linked SONAME; do not force players to install Crypto++.
libpath="$(ldd out/ps4rt-pkg | awk '$1 ~ /^libcrypto(\+\+|pp)\.so/ && $2 == "=>" {print $3; exit}')"
soname="$(ldd out/ps4rt-pkg | awk '$1 ~ /^libcrypto(\+\+|pp)\.so/ {print $1; exit}')"
if [[ -z "$libpath" || ! -f "$libpath" || -z "$soname" ]]; then
    echo "Could not locate linked Crypto++ runtime to bundle." >&2
    rm -f out/ps4rt-pkg
    exit 1
fi
# When rerunning in the same developer checkout, the executable's existing
# $ORIGIN/lib RUNPATH may already resolve to the bundled copy. Avoid copying
# the library onto itself; clean container builds start with an empty out/.
target="out/lib/$soname"
if [[ ! "$libpath" -ef "$target" ]]; then
    cp -L -- "$libpath" "$target"
fi
echo "PS4RT native PKG extractor: out/ps4rt-pkg"
echo "Bundled Crypto++ runtime: out/lib/$soname"
echo "Release must include vendor source and third-party license notices."
