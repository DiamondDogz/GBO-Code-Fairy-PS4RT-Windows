# Use of AI

AI was used. The assistant was [Claude](https://claude.ai), by Anthropic. This file says where, and where it was not.

## The program

The Windows build of the runtime, the fixes it needed to run Code Fairy, the Sieg Zeon launcher, its English and French texts, the Crowbar files, and these documents were written with Claude, then checked against what the build actually does. The Windows port is published by [DiamondDogz](https://github.com/DiamondDogz).

The long crash at the first battle was traced to the player's own `game_ss.cpk`, cut short by an interrupted extraction. The fix was a complete copy of that file, not a change to the game. The investigation code was removed before this release. What stayed is the check that reports a short archive at start.

The runtime is not an AI rewrite of GBO-Code-Fairy-PS4RT. It is HekamiAeon's program, built for Windows. Credit for that program stays with HekamiAeon and the people named in [CREDITS.md](CREDITS.md).

## What AI was not allowed to supply

- The game, or any instruction for obtaining it illegally.
- Key art, music or any other material from the game.
- A claim that a feature exists when it is not in the source this branch builds.
