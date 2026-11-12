#!/usr/bin/env python3
"""Builds the player archive of a Windows release from the tracked files of this branch.

Usage: python3 packaging/windows/release_zip.py VERSION OUTPUT_DIR
Writes GBO-Code-Fairy-PS4RT-Windows-VERSION.zip and its .sha256. The archive holds what a
player runs (SiegZeon.exe, bin, python, the launcher, the scripts, FSR4 assets, the documents),
never the game, a PKG, or the build sources.
"""
import hashlib
import pathlib
import subprocess
import sys
import zipfile

KEEP = ("SiegZeon.exe", "bin/", "python/", "launcher/siegzeon/", "scripts/", "gpu/assets/",
        "fsr4_shaders/", "port_features.json", "crowbar.json", "LICENSE", "README.md",
        "README.fr.md", "INSTALL.md", "CREDITS.md", "AI.md", "docs/UPSTREAM_CREDITS.md")
SKIP = {"launcher/siegzeon/siegzeon.cc", "launcher/siegzeon/siegzeon.rc",
        "scripts/ps4rt_pkg_native.cpp", "scripts/build_pkg_native.sh",
        "scripts/build_setup.sh", "scripts/ps4rt_setup_native.cpp"}


def main():
    version, out = sys.argv[1], pathlib.Path(sys.argv[2])
    out.mkdir(parents=True, exist_ok=True)
    name = f"GBO-Code-Fairy-PS4RT-Windows-{version}"
    files = subprocess.run(["git", "ls-files"], capture_output=True, text=True, check=True).stdout.splitlines()
    chosen = [f for f in files if f.startswith(KEEP) and f not in SKIP]
    for f in chosen:
        lower = f.lower()
        if lower.endswith(".pkg") or pathlib.PurePosixPath(lower).name in ("eboot.bin", "param.sfo"):
            sys.exit(f"refusing game file in the archive: {f}")
    for required in ("SiegZeon.exe", "bin/ps4rt.exe", "bin/ps4rt-pkg.exe", "python/python.exe", "crowbar.json"):
        if required not in chosen:
            sys.exit(f"missing from the archive: {required}")
    archive = out / f"{name}.zip"
    with zipfile.ZipFile(archive, "w", zipfile.ZIP_DEFLATED, compresslevel=9) as z:
        for f in chosen:
            z.write(f, f"{name}/{f}")
    digest = hashlib.sha256(archive.read_bytes()).hexdigest()
    (out / f"{name}.zip.sha256").write_text(f"{digest}  {archive.name}\n", encoding="ascii")
    print(f"{archive} {len(chosen)} files sha256 {digest}")


if __name__ == "__main__":
    main()
