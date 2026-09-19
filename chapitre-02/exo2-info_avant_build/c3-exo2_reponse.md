# Exercice 2 — Info avant build

## Sortie de `jenga info`

\`\`\`
============================ Jenga Workspace: Salle ============================

Location: C:\Users\Lenovo\ani-4087\chapitre-02\exo1-le_projet_minimal
Entry file: C:\Users\Lenovo\ani-4087\chapitre-02\exo1-le_projet_minimal\Salle.jenga
Configurations: Debug, Release
Platforms: Windows
Target OSes: Windows
Target Architectures: x86_64


Projects
------------------------------------------------------------
Name    Kind         Language   Test   External
===============================================
Salle   ConsoleApp   C++        No     No


Available Toolchains
------------------------------------------------------------
Name                 Family   Target OS   Arch     Env
==========================================================
host-gcc             gcc      Windows     x86_64   mingw
msvc                 msvc     Windows     x86_64   msvc
mingw                gcc      Windows     x86_64   mingw
zig-linux-x86_64     clang    Linux       x86_64   gnu
zig-linux-x64        clang    Linux       x86_64   gnu
zig-windows-x86_64   clang    Windows     x86_64   mingw
zig-windows-x64      clang    Windows     x86_64   mingw
zig-macos-x86_64     clang    macOS       x86_64   gnu
zig-macos-arm64      clang    macOS       arm64    gnu
zig-ios-arm64        clang    iOS         arm64
zig-tvos-arm64       clang    tvOS        arm64
zig-watchos-arm64    clang    watchOS     arm64
zig-android-arm64    clang    Android     arm64    android
zig-web-wasm32       clang    Web         wasm32


Daemon
------------------------------------------------------------
Status: Not running
\`\`\`

## Ce que cette sortie apprend, que le fichier de projet ne disait pas

Mon `Salle.jenga` ne declare qu'une intention etroite : cibler Windows
x86_64. `jenga info` revele une realite bien plus large que ca :

- **Trois toolchains Windows natifs coexistent sur ma machine** (host-gcc,
  msvc, mingw) et le fichier de projet ne dit jamais lequel sera choisi. A
  l'exercice 1, `jenga build` a choisi `mingw` (mon installation MSYS2
  UCRT64) sans que j'aie eu a le preciser dans le fichier.

- **Ma machine peut deja cross-compiler vers huit plateformes que je n'ai
  jamais mentionnees** : Linux, macOS (x86_64 et arm64), iOS, tvOS, watchOS,
  Android (arm64) et le Web (wasm32), via des toolchains Zig deja detectes.
  Rien dans `Salle.jenga` ne le laissait deviner : le fichier ne cible que
  `TargetOS.WINDOWS`, mais la capacite de construire pour Android ou pour le
  Web existe deja sur cette machine, prete a etre activee en ajoutant
  simplement ces OS a `targetoses([...])`. C'est une capacite de l'outil et
  de la machine, pas une propriete du fichier de projet.

- **L'existence d'un demon de build** (`Daemon: Not running`), une
  fonctionnalite entiere de Jenga (utilisee par `jenga watch`) jamais
  mentionnee dans le fichier de projet.

- **Les attributs implicites du projet** (`Test: No`, `External: No`) sont
  rendus explicites, alors que le fichier ne fait que les sous-entendre par
  l'absence d'un appel a `testsuite()` ou d'un marquage "externe".
