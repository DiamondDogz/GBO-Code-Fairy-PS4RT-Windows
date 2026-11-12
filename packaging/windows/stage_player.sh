#!/usr/bin/env bash
# Collects the Windows binaries of a player release into out/player:
#   SiegZeon.exe
#   bin/ps4rt.exe, bin/ps4rt-pkg.exe (update and volume-key PKGs), bin/ps4rt-setup.exe and the DLLs bundle_dlls.sh found for them
# The source, the scripts, Python and the documents are added by the public repository.
set -euo pipefail
cd -- "$(dirname -- "$0")/../.."
rm -rf out/player
mkdir -p out/player/bin
cp -f out/SiegZeon.exe out/player/
cp -f out/ps4rt.exe out/player/bin/
cp -f out/ps4rt-pkg.exe out/player/bin/
[[ ! -f out/ps4rt-setup.exe ]] || cp -f out/ps4rt-setup.exe out/player/bin/
while IFS= read -r dll; do
    [[ -n $dll && -f out/$dll && $dll == *.dll ]] && cp -f "out/$dll" out/player/bin/
done < out/bundled-dlls.txt
ls -la out/player out/player/bin | head -20
echo "Player files: $(find out/player -type f | wc -l)"
