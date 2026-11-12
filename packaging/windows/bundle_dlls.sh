#!/usr/bin/env bash
# Copy the MSYS2 libraries ps4rt.exe needs into out/, next to the exe.
# Windows does not search C:\msys64 when the player double-clicks the program.
set -euo pipefail
cd -- "$(dirname -- "$0")/../.."
command -v ldd >/dev/null
queue=$(mktemp)
copied=$(mktemp)
trap 'rm -f "$queue" "$copied"' EXIT
: > "$copied"
printf '%s\n' out/ps4rt.exe out/ps4rt-pkg.exe out/ps4rt-setup.exe out/SiegZeon.exe > "$queue"
while [[ -s $queue ]]; do
    exe=$(head -n 1 "$queue")
    tail -n +2 "$queue" > "$queue.next"
    mv "$queue.next" "$queue"
    [[ -f $exe ]] || continue
    while IFS= read -r line; do
        [[ $line == *"=>"* ]] || continue
        if [[ $line == *"not found"* ]]; then
            echo "DLL manquante pour $exe : $line" >&2
            exit 1
        fi
        src=${line#*=> }
        src=${src%% (*}
        src=${src%"${src##*[![:space:]]}"}
        case $src in
            /c/[Ww][Ii][Nn][Dd][Oo][Ww][Ss]/*|"") continue ;;
        esac
        [[ -f $src ]] || { echo "Introuvable : $src" >&2; exit 1; }
        base=$(basename "$src")
        dest=$(cd out && pwd)/$base
        here=$(cd "$(dirname "$src")" && pwd)/$base
        if [[ $here == "$dest" ]]; then
            if ! grep -qx "$base" "$copied"; then
                echo "$base" >> "$copied"
                echo "out/$base" >> "$queue"
            fi
            continue
        fi
        if ! grep -qx "$base" "$copied"; then
            cp -f "$src" "out/$base"
            echo "$base" >> "$copied"
            echo "out/$base" >> "$queue"
            echo "DLL $base"
        fi
    done < <(ldd "$exe" 2>/dev/null || true)
done
count=$(wc -l < "$copied" | tr -d ' ')
[[ $count -gt 0 ]] || { echo "Aucune DLL copiée. ldd n'a rien vu." >&2; exit 1; }
for required in SDL3.dll libwinpthread-1.dll; do
    [[ -f out/$required ]] || { echo "Toujours manquante : $required" >&2; exit 1; }
done
if [[ ! -f out/libc++.dll && ! -f out/libstdc++-6.dll ]]; then
    echo "Runtime C++ manquant dans out/" >&2
    exit 1
fi
compgen -G "out/avcodec-*.dll" >/dev/null || { echo "avcodec manquant dans out/" >&2; exit 1; }
cp "$copied" out/bundled-dlls.txt
echo "DLL copiées : $count"
