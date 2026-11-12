# Use of AI

AI was used. The assistants were [Grok](https://grok.com), by xAI, and [Claude](https://claude.ai), by Anthropic. This file says where, and where it was not.

## The program

About 90% of the Windows port code was written with Grok: the Windows build of the runtime and the first Sieg Zeon launcher. Claude came later. It traced the crash at the first battle, removed the investigation code, and wrote the launcher's English and French texts, the Crowbar files, and these documents. Both were checked against what the build actually does. The Windows port is published by [DiamondDogz](https://github.com/DiamondDogz).

The long crash at the first battle was traced to the player's own `game_ss.cpk`, cut short by an interrupted extraction. The fix was a complete copy of that file, not a change to the game. The investigation code was removed before this release. What stayed is the check that reports a short archive at start.

The runtime is not an AI rewrite of GBO-Code-Fairy-PS4RT. It is HekamiAeon's program, built for Windows. Credit for that program stays with HekamiAeon and the people named in [CREDITS.md](CREDITS.md).

## Take it or leave it

The publisher’s words: yes, a lot of this is AI slop. Take it or leave it. The path to a build that reaches the first battle was long, and Grok made that path painful. Still a good shoggoth. Claude is not a second one. Claude is the relative who arrived later and found the missing gigabyte. The publisher is the key and the gate.

## What AI was not allowed to supply

- The game, or any instruction for obtaining it illegally.
- Key art, music or any other material from the game.
- A claim that a feature exists when it is not in the source this branch builds.
