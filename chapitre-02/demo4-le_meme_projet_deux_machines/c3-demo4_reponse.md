# Demo4 — Le même projet, deux machines

## Précision honnête
Je n'ai qu'un seul PC (Windows) à disposition, pas un second ordinateur physique sous un
autre OS. J'ai donc démontré la même chose avec ce qui est réellement disponible : le même
fichier `.jenga`, **jamais modifié**, construit une fois pour la machine hôte (Windows) et
une fois en cross-compilation pour Android — puis vérifié que le résultat de chaque
construction diffère réellement, exactement comme l'exercice le demande ("c'est le résultat
de la construction qu'il faut faire parler").

## Le projet (`24_all_platforms.jenga`, exemple fourni par Jenga, non modifié)
Le fichier déclare un seul projet `AllPlatformsApp`, avec `consoleapp()` par défaut, puis
des filtres `system:Windows` / `system:Android` qui changent le genre d'appli, le toolchain
et les defines — sans qu'une seule ligne ne soit touchée entre les deux builds.

## Build 1 — Windows (machine hôte)
```bash
jenga build --platform Windows --config Release --jenga-file 24_all_platforms.jenga
```
```
Configuration: Release
Target:        Windows x86_64
Toolchain:     clang-mingw

Build Order (1 projects):
  1. AllPlatformsApp [CONSOLE_APP]
...
✓ Built: Build\Bin\Release-Windows\AllPlatformsApp\AllPlatformsApp.exe
```

Exécution :
```bash
$ ./Build/Bin/Release-Windows/AllPlatformsApp/AllPlatformsApp.exe
Hello from Jenga — Windows!
```

## Build 2 — Android (cross-compilation, même fichier, aucune ligne changée)
```bash
jenga build --platform Android-arm64 --config Release --jenga-file 24_all_platforms.jenga
```
```
Configuration: Release
Target:        Android arm64
Toolchain:     android-ndk

Build Order (1 projects):
  1. AllPlatformsApp [WINDOWED_APP]
...
✓ Built: Build\Bin\Release-Android\AllPlatformsApp\libAllPlatformsApp.so
```

## Le résultat parle : deux binaires réellement différents

| | Windows | Android |
|---|---|---|
| `Kind` annoncé au build | `CONSOLE_APP` | `WINDOWED_APP` |
| Toolchain | `clang-mingw` | `android-ndk` |
| Fichier produit | `AllPlatformsApp.exe` | `libAllPlatformsApp.so` |
| Point d'entrée compilé | `main` | `android_main` |

Vérification directe des symboles dans chaque binaire :
```bash
$ nm Build/Bin/Release-Windows/AllPlatformsApp/AllPlatformsApp.exe | grep -w main
00000001400014b0 T main

$ llvm-nm.exe Build/Bin/Release-Android/AllPlatformsApp/libAllPlatformsApp.so | grep -w android_main
0000000000002ee4 T android_main
```

Le fichier source (`main.cpp`) contient les deux fonctions dans des blocs
`#if defined(__ANDROID__) ... #elif ... #else ... main() ... #endif` : seul le
`define __ANDROID__`, injecté automatiquement par le toolchain choisi via le filtre
`system:Android`, décide laquelle des deux est compilée. C'est la preuve irréfutable que
les filtres appliqués ne sont pas les mêmes selon la plateforme — visible uniquement dans
ce que le build produit, pas dans une commande d'inspection.

## Ce que `jenga info` montre (et ne montre pas)
```bash
$ jenga info --jenga-file 24_all_platforms.jenga
...
Projects
------------------------------------------------------------
Name              Kind         Language   Test   External
=========================================================
AllPlatformsApp   ConsoleApp   C++        No     No
...
```
`jenga info` affiche `Kind: ConsoleApp` — la valeur déclarée **hors filtre**, au niveau du
projet — et ne change jamais, quelle que soit la plateforme visée. Il ne montre donc pas
`WindowedApp` pour Android, alors que c'est pourtant bien ce que le build produit
(`Kind: WINDOWED_APP` apparaît dans la sortie de `jenga build`, jamais dans celle de
`jenga info`). Exactement l'avertissement de l'énoncé : `jenga info` inspecte la
déclaration statique du projet, pas le résultat de l'évaluation des filtres — cette
évaluation n'a lieu qu'au moment du build, pour une plateforme donnée.
