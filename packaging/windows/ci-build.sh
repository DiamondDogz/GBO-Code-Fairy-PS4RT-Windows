#!/usr/bin/env bash
# Runs on the self-hosted Windows runner, inside MSYS2, from the repo root.
set -euo pipefail
cd -- "$(dirname -- "$0")/../.."
export MSYSTEM=CLANG64
export PATH="/clang64/bin:/usr/bin:${PATH:-}"
export PKG_CONFIG_PATH="/clang64/lib/pkgconfig${PKG_CONFIG_PATH:+:$PKG_CONFIG_PATH}"
export PS4RT_BUILD_JOBS="$(nproc)"
bash build.sh
bash packaging/windows/build_siegzeon.sh
bash packaging/windows/bundle_dlls.sh
bash packaging/windows/stage_player.sh
echo "Contenu de out :"
ls -la out
fail=0
need() {
    if [[ ! -e $1 ]]; then
        echo "Manquant : $1" >&2
        fail=1
    fi
}
need out/ps4rt.exe
need out/ps4rt-pkg.exe
need out/SiegZeon.exe
need out/siegzeon/launcher_host.py
need out/siegzeon/python.txt
if [[ ! -e out/ps4rt-setup.exe && ! -e out/ps4rt-setup ]]; then
    echo "Manquant : out/ps4rt-setup.exe" >&2
    fail=1
fi
[[ $fail -eq 0 ]]
echo BUILD_OK
