# Sourced by the Windows build scripts: puts the MSYS2 CLANG64 toolchain first on PATH.
# MSYS2_ROOT overrides the default C:/msys64.
msys2_root=${MSYS2_ROOT:-/c/msys64}
if [[ -d $msys2_root/clang64/bin ]]; then
    export PATH="$msys2_root/clang64/bin:$msys2_root/usr/bin:$PATH"
    export MSYSTEM=CLANG64
    export PKG_CONFIG_PATH="${msys2_root}/clang64/lib/pkgconfig${PKG_CONFIG_PATH:+:$PKG_CONFIG_PATH}"
    export CC="${CC:-clang}"
    export CXX="${CXX:-clang++}"
fi
