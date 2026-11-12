# Install

English first. La version française suit.

## What you need

- Windows 10 or Windows 11, 64-bit.
- A GPU with a current Vulkan driver. NVIDIA or AMD.
- Microsoft Edge WebView2. It is already installed with Edge on current Windows 10 and 11. The launcher does not bundle it.
- Your own copy of MOBILE SUIT GUNDAM BATTLE OPERATION Code Fairy, PlayStation 4, title id CUSA28763, already extracted. The folder must contain `eboot.bin`, `sce_sys` and `sce_module`.
- If that folder is still game version 01.00, your own 01.03 update PKG.
- Your own three volume key PKGs (Volume 1, 2 and 3), all in one folder. They are not optional extras: each one unlocks an episode that is already in the game.

Only the game must be extracted. The update and the volume keys stay PKG files: the launcher installs them with `bin/ps4rt-pkg.exe`.

This project does not provide the game, keys, firmware, or saves.

## Tested on

windows-1.0 was played on this machine. It is not a minimum requirement.

- Windows 11 Pro for Workstations, version 26H2
- Intel Core i9-12900KF
- 128 GB DDR5, running at 4000 MT/s
- NVIDIA GeForce RTX 3090, 24 GB

## Unpack

1. Unzip the archive. The folder can keep the name it already has. Pick a place you can write to.
2. Do not run the program from inside the zip.
3. `SiegZeon.exe` is at the root of that folder. Open that file.
4. Windows SmartScreen may warn you. The executable is not signed. Choose More info, then Run anyway. Do not turn off Microsoft Defender for this.

The window is the launcher. It does not open your browser.

## Folders

Open Folders (📁):

1. Game folder. The extracted CUSA28763 directory, not a PKG.
2. Update, optional. One `.pkg` file, the 01.03 update, only if the game folder is still 01.00. The update is written into the game folder. Copy it first if you want to keep 01.00.
3. Volume keys, optional. One folder. Every `.pkg` in that folder is installed in one pass. Without them, the episodes stay locked.

Clear removes an optional path. Once installed, the update and the volume keys stay installed.

## Play

Press ▶. The launcher writes `ps4rt.ini` in `out\CUSA28763` and starts the game. A console window keeps the log. The first start prepares the game code and takes longer.

In the game, Insert, or L2 and R3, opens the settings menu. The launcher's settings (⚙) are the same ones:

- Internal resolution, 16:9 and ultrawide. Ultrawide is experimental. Applied on the next start.
- Graphics quality: Max (original), Medium or Low. Applied on the next start.
- FSR: Off, FSR 1, FSR 3.1 or FSR 4 v07, and the FSR output size. FSR 4 v07 needs a card that supports it.
- Sharpness, window mode, frame rate (60 or 30), FPS counter, vibration.
- Anisotropic filtering, applied on the next start.

## Language

English is the default. The EN button switches the launcher to French. The choice is remembered. Error boxes from `SiegZeon.exe` itself follow the Windows display language.

## Crowbar

This folder is [Crowbar](https://github.com/DiamondDogz/crowbar) compatible: `crowbar.json` is at the root. Crowbar can start the game from it and records the trophies the game unlocks. You can still use `SiegZeon.exe` without Crowbar.

## If it fails

- "WebView2" missing: install the Evergreen WebView2 Runtime from Microsoft, then start `SiegZeon.exe` again.
- "SiegZeon.exe is alone in this folder": the zip was not unpacked in full. `launcher`, `python` and `bin` must be next to it.
- "That folder has no eboot.bin": choose the extracted CUSA28763 folder itself, not its parent and not a PKG.
- Update refused: the file must be the real 01.03 `.pkg` of CUSA28763.
- "v01.00 cannot launch directly": the game folder is still 01.00. Set the 01.03 update PKG in Folders.
- An update stopped halfway: select the same 01.03 PKG again. The launcher resumes in place.
- `ARCHIVE INCOMPLETE` in the log: a `.cpk` file in the game folder is shorter than its own header says. The game would crash later. Most often `game_ss.cpk`, which comes from the base game, not from the update: extract it again from a complete copy of the base game. An extraction that stopped early, or a base game split in several parts with one missing, gives this.
- The game closes at once, and the log names a Vulkan error: update the GPU driver.

---

# Installation

## Il vous faut

- Windows 10 ou Windows 11, 64 bits.
- Une carte graphique avec un pilote Vulkan récent. NVIDIA ou AMD.
- Microsoft Edge WebView2. Il est déjà là avec Edge sur les Windows 10 et 11 actuels. Le lanceur ne l’embarque pas.
- Votre propre copie de MOBILE SUIT GUNDAM BATTLE OPERATION Code Fairy, PlayStation 4, identifiant CUSA28763, déjà extraite. Le dossier doit contenir `eboot.bin`, `sce_sys` et `sce_module`.
- Si ce dossier est encore en version 01.00, votre propre PKG de mise à jour 01.03.
- Vos propres trois PKG de clés de volume (Volume 1, 2 et 3), tous dans un même dossier. Ce ne sont pas des bonus : chacun débloque un épisode déjà présent dans le jeu.

Seul le jeu doit être extrait. La mise à jour et les clés de volume restent des fichiers PKG : le lanceur les installe avec `bin/ps4rt-pkg.exe`.

Ce projet ne fournit ni le jeu, ni les clés, ni le firmware, ni les sauvegardes.

## Machine de test

windows-1.0 a été joué sur cette machine. Ce n’est pas un minimum.

- Windows 11 Professionnel pour les Stations de travail, version 26H2
- Intel Core i9-12900KF
- 128 Go de DDR5, à 4000 MT/s
- NVIDIA GeForce RTX 3090, 24 Go

## Décompresser

1. Décompressez l’archive. Le dossier peut garder le nom qu’il a déjà. Choisissez un endroit où vous pouvez écrire.
2. Ne lancez pas le programme depuis l’intérieur du zip.
3. `SiegZeon.exe` est à la racine. Ouvrez ce fichier.
4. SmartScreen peut prévenir. L’exécutable n’est pas signé. Choisissez Informations complémentaires, puis Exécuter quand même. Ne désactivez pas Microsoft Defender pour cela.

La fenêtre est le lanceur. Elle n’ouvre pas le navigateur.

## Dossiers

Ouvrez Dossiers (📁) :

1. Dossier du jeu. Le dossier CUSA28763 extrait, pas un PKG.
2. Mise à jour, facultatif. Un seul fichier `.pkg`, la mise à jour 01.03, seulement si le dossier du jeu est encore en 01.00. La mise à jour est écrite dans le dossier du jeu. Copiez-le d’abord si vous voulez garder la 01.00.
3. Clés de volume, facultatif. Un dossier. Chaque `.pkg` qu’il contient est installé en une fois. Sans elles, les épisodes restent verrouillés.

Effacer retire un chemin facultatif. Une fois installées, la mise à jour et les clés de volume le restent.

## Jouer

Appuyez sur ▶. Le lanceur écrit `ps4rt.ini` dans `out\CUSA28763` et démarre le jeu. Une fenêtre de console garde le journal. Le premier démarrage prépare le code du jeu et prend plus de temps.

En jeu, Insert, ou L2 et R3, ouvre le menu des réglages. Les réglages du lanceur (⚙) sont les mêmes :

- Résolution interne, 16:9 et ultra-large. L’ultra-large est expérimental. Appliquée au démarrage suivant.
- Qualité graphique : Max (d’origine), Moyenne ou Basse. Appliquée au démarrage suivant.
- FSR : Désactivé, FSR 1, FSR 3.1 ou FSR 4 v07, et la taille de sortie FSR. FSR 4 v07 demande une carte qui le supporte.
- Netteté, mode de fenêtre, images par seconde (60 ou 30), compteur d’images, vibration.
- Filtre anisotrope, appliqué au démarrage suivant.

## Langue

L’anglais est la langue de départ. Le bouton EN passe le lanceur en français. Le choix est retenu. Les fenêtres d’erreur de `SiegZeon.exe` lui-même suivent la langue d’affichage de Windows.

## Crowbar

Ce dossier est compatible [Crowbar](https://github.com/DiamondDogz/crowbar) : `crowbar.json` est à la racine. Crowbar peut lancer le jeu depuis ce dossier et enregistre les trophées que le jeu débloque. `SiegZeon.exe` reste utilisable sans Crowbar.

## Si ça échoue

- WebView2 absent : installez le runtime Evergreen WebView2 chez Microsoft, puis relancez `SiegZeon.exe`.
- « SiegZeon.exe est seul dans ce dossier » : l’archive n’a pas été décompressée en entier. `launcher`, `python` et `bin` doivent être à côté.
- « Ce dossier ne contient pas eboot.bin » : choisissez le dossier CUSA28763 extrait lui-même, pas son parent ni un PKG.
- Mise à jour refusée : le fichier doit être le vrai `.pkg` 01.03 de CUSA28763.
- « v01.00 cannot launch directly » : le dossier du jeu est encore en 01.00. Indiquez le PKG de mise à jour 01.03 dans Dossiers.
- Une mise à jour s’est arrêtée au milieu : resélectionnez le même PKG 01.03. Le lanceur reprend sur place.
- `ARCHIVE INCOMPLETE` dans le journal : un fichier `.cpk` du dossier du jeu est plus court que ce que dit son propre en-tête. Le jeu planterait plus tard. Le plus souvent `game_ss.cpk`, qui vient du jeu de base et non de la mise à jour : extrayez-le à nouveau depuis une copie complète du jeu de base. Une extraction interrompue, ou un jeu de base découpé en plusieurs parties dont une manque, donne ce message.
- Le jeu se ferme tout de suite, et le journal parle de Vulkan : mettez à jour le pilote de la carte graphique.
