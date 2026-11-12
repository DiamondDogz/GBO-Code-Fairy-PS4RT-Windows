# Credits and legal notes

This is an unofficial Windows port. It does not replace, and is not endorsed by, Bandai Namco Entertainment, Sunrise or Sony Interactive Entertainment. The game is *MOBILE SUIT GUNDAM BATTLE OPERATION Code Fairy*. The name is used to say which title this program runs.

## Runtime

- [HekamiAeon, GBO-Code-Fairy-PS4RT](https://github.com/HekamiAeon/GBO-Code-Fairy-PS4RT). The runtime. GPL-2.0-or-later. Written for Linux. This port follows upstream `main`, commit `0252c9dda14ee0e00a3688585a3e6f43fdf16ead`: the v1.0 release of 7 November 2026 (`49ea9f001022d81c2ae4fb6aaaa42684bbeff16c`) plus two commits of 8 November 2026. This is not a general PS4 emulator.
- The lineage HekamiAeon credits is kept in [docs/UPSTREAM_CREDITS.md](docs/UPSTREAM_CREDITS.md): [Felipe Prado, gow3_pc](https://github.com/FelipePrado0/gow3_pc), [deadinside28](https://github.com/deadinside28/bloodborne_pc) and [Supermedo](https://github.com/Supermedo/bloodborne_pc), bloodborne_pc, [shadPS4](https://github.com/shadps4-emu/shadPS4) and its contributors, including cuesta4, for the Vulkan renderer and the shader recompiler.
- [AzaharPlus, shadPS4Plus](https://github.com/AzaharPlus/shadPS4Plus). PKG extraction, in `third_party/ps4pkg/`. GPL-2.0-or-later.
- [DiamondDogz, Sakura-Wars-PS4RT-Windows](https://github.com/DiamondDogz/Sakura-Wars-PS4RT-Windows). The trophy catalog code in `scripts/trophy_crypto.py`, and the layout of this Windows release.

## Windows port

- Published by [DiamondDogz](https://github.com/DiamondDogz). This is the Windows build, the fixes it needed, and the launcher around the v1.0 runtime. It is not a claim on the runtime, and not a claim on the game.
- [Grok](https://grok.com), by xAI. About 90% of the Windows port code, including the first Sieg Zeon launcher, was written with Grok. The runtime was not rewritten by it. See [AI.md](AI.md).
- [Claude](https://claude.ai), by Anthropic. The crash investigation and the fixes listed below, the cleanup, the launcher languages, the Crowbar files, and these documents were written with Claude. The runtime was not rewritten by it.

Windows changes in this branch, beyond the build itself:

- `pread` keeps the file offset, as on Linux, and file operations on one descriptor run one at a time.
- At start, each `.cpk` archive of the game folder is checked against its own header. A short archive is reported before it can crash the game.
- The PKG extractor reads files over 4 GiB.
- `crowbar.json` and the trophy call from [crowbar-sdk](https://github.com/DiamondDogz/crowbar-sdk) (MIT), in `gpu/shim/crowbar.h` and `gpu/shim/_crowbar.cpp`.

## Libraries in the Windows build

- [AMD FidelityFX Super Resolution](https://gpuopen.com/fidelityfx-super-resolution/). The FSR4 v07 model assets shipped beside the runtime carry their own notice, `fsr4_shaders/LICENSE-FSR4-v07.txt`. The notice stays with the files.
- SDL3. Its license stays with the binary.
- FFmpeg and the codecs linked into this build, from MSYS2. Some are GPL-2. That is compatible with GPL-2.0-or-later. It is not compatible with a claim that the media libraries are LGPL only.
- Crypto++, zlib, Zydis, ImGui, LibAtrac9, Sirit, half, xbyak, miniz, magic_enum, DejaVu fonts. Each keeps its notice in its source folder.
- Python 3.12, embedded to run the launcher host and the setup scripts. The PSF license is `python/LICENSE.txt`.
- [webview](https://github.com/webview/webview), used by `SiegZeon.exe`. MIT.
- Microsoft Edge WebView2. Not bundled. The user installs or already has the Evergreen runtime.

## What this repository must not contain

- The game, any PKG, firmware, or saves.
- Official key art, music or any other recording from the game.
- A binary of `ps4rt.exe` published without the corresponding source of the Windows changes.

## Release

The release [windows-1.0](https://github.com/DiamondDogz/GBO-Code-Fairy-PS4RT-Windows/releases/tag/windows-1.0) is a pre-release, the player archive for this branch. The notes give the SHA-256. It starts CUSA28763 at game version 01.03. `SiegZeon.exe` is unsigned. Do not tell people to disable their antivirus.
