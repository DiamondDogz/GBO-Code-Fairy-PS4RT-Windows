#!/usr/bin/env bash
# Builds out/SiegZeon.exe, the WebView2 window of the SiegZeon launcher.
# Run from an MSYS2 CLANG64 shell, after the runtime build. Header-only sources
# land in out/ and are not part of the source tree:
#   webview (MIT) at WEBVIEW_REF
#   Microsoft.Web.WebView2 (NuGet) at WEBVIEW2_VERSION
set -euo pipefail
export GIT_TERMINAL_PROMPT=0
exec </dev/null
cd -- "$(dirname -- "$0")/../.."
if [[ -f ./msys2-env.sh ]]; then
    # shellcheck disable=SC1091
    source ./msys2-env.sh
fi
WEBVIEW_REF=${WEBVIEW_REF:-0.12.0}
WEBVIEW2_VERSION=${WEBVIEW2_VERSION:-1.0.3179.45}
mkdir -p out
if [[ ! -f out/webview/core/include/webview/webview.h ]]; then
    rm -rf out/webview
    timeout 600 git clone -q --depth 1 --branch "$WEBVIEW_REF" https://github.com/webview/webview out/webview ||
        timeout 600 git clone -q --depth 1 https://github.com/webview/webview out/webview
fi
if [[ ! -f out/webview2/build/native/include/WebView2.h ]]; then
    rm -rf out/webview2
    mkdir -p out/webview2
    curl -fsSL --retry 3 --max-time 300 \
        -o out/webview2.nupkg \
        "https://www.nuget.org/api/v2/package/Microsoft.Web.WebView2/$WEBVIEW2_VERSION"
    python - out/webview2.nupkg out/webview2 <<'PY'
import sys, zipfile
archive, dest = sys.argv[1], sys.argv[2]
with zipfile.ZipFile(archive) as package:
    for name in package.namelist():
        if name.startswith("build/native/include/"):
            package.extract(name, dest)
PY
fi
if [[ ! -f out/webview2/build/native/include/EventToken.h ]]; then
    cat > out/webview2/build/native/include/EventToken.h <<'HDR'
#pragma once
#if defined(__has_include_next) && __has_include_next(<eventtoken.h>)
#include_next <eventtoken.h>
#else
typedef struct EventRegistrationToken { __int64 value; } EventRegistrationToken;
#endif
HDR
fi
rc=$(command -v llvm-windres || command -v windres)
(cd launcher/siegzeon && "$rc" siegzeon.rc -O coff -o ../../out/siegzeon-res.o)
if ! clang++ -std=c++17 -O2 -static -mwindows \
    -Iout/webview/core/include -Iout/webview2/build/native/include \
    launcher/siegzeon/siegzeon.cc out/siegzeon-res.o -o out/SiegZeon.exe \
    -lws2_32 -lole32 -lshell32 -lshlwapi -luser32 -lversion -luuid -ladvapi32
then
    if [[ -f out/SiegZeon.exe ]]; then
        echo "SiegZeon.exe est ouvert : l'ancien exe reste, les DLL sont quand même copiées." >&2
    else
        exit 1
    fi
fi
if command -v llvm-strip >/dev/null 2>&1; then
    llvm-strip out/SiegZeon.exe || true
fi
py=""
for candidate in \
    "${MSYS2_ROOT:-/c/msys64}/usr/bin/python.exe" \
    "${MSYS2_ROOT:-/c/msys64}/usr/bin/python3.exe" \
    "${MSYS2_ROOT:-/c/msys64}/clang64/bin/python.exe"
do
    if [[ -f $candidate ]]; then
        py=$candidate
        break
    fi
done
if [[ -z $py ]]; then
    echo "Dépendance manquante : python.exe dans ${MSYS2_ROOT:-/c/msys64}/usr/bin" >&2
    exit 1
fi
win=$(cygpath -w "$py")
if [[ ! -f $win ]]; then
    echo "Dépendance manquante : $win" >&2
    exit 1
fi
rm -rf out/siegzeon
mkdir -p out/siegzeon
cp launcher/siegzeon/launcher_host.py \
    launcher/siegzeon/index.html \
    launcher/siegzeon/artwork.jpg \
    launcher/siegzeon/artwork.mp4 \
    launcher/siegzeon/crowbar.png \
    launcher/siegzeon/siegzeon.ico \
    out/siegzeon/
printf '%s' "$win" > out/siegzeon/python.txt
test -f out/siegzeon/launcher_host.py
test -f out/siegzeon/index.html
test -s out/siegzeon/python.txt
echo "Python: $win"
echo "Built $PWD/out/SiegZeon.exe"
