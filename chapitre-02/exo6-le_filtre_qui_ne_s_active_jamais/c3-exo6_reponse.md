# Exercice 6 — Le filtre qui ne s'active jamais

## Fichier de projet

\`\`\`python
from Jenga import *

with workspace("Salle"):
    configurations(["Debug", "Release"])
    targetoses([TargetOS.WINDOWS])
    targetarchs([TargetArch.X86_64])

    with project("Salle"):
        consoleapp()
        language("C++")
        cppdialect("C++17")
        files(["src/main.cpp"])

        with filter("system:Windows"):
            defines(["FILTRE_ACTIF"])
\`\`\`
(la seule ligne modifiee entre les deux etapes est `filter("system:Linux")`
devenu `filter("system:Windows")`)

## Sortie de `jenga info` — condition fausse (filter("system:Linux"), sur Windows)

\`\`\`
============================ Jenga Workspace: Salle ============================

Location: C:\Users\Lenovo\ani-4087\chapitre-02\exo6-le_filtre_qui_ne_s_active_jamais
Entry file: C:\Users\Lenovo\ani-4087\chapitre-02\exo6-le_filtre_qui_ne_s_active_jamais\Salle.jenga
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

## Sortie de `jenga info` — condition vraie (filter("system:Windows"), sur Windows)

\`\`\`
============================ Jenga Workspace: Salle ============================

Location: C:\Users\Lenovo\ani-4087\chapitre-02\exo6-le_filtre_qui_ne_s_active_jamais
Entry file: C:\Users\Lenovo\ani-4087\chapitre-02\exo6-le_filtre_qui_ne_s_active_jamais\Salle.jenga
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

## Constat

Les deux sorties sont strictement identiques, caractere pour caractere,
malgre le passage d'une condition fausse (`system:Linux`, sur une machine
Windows) a une condition vraie (`system:Windows`). Le tableau `Projects`
affiche toujours `Kind: ConsoleApp`, sans aucune trace du `define` pose a
l'interieur du filtre.

Ce n'est pas parce que le define n'a pas d'effet : c'est parce que
`jenga info` ne resout JAMAIS les blocs `filter()`, quelle que soit leur
condition. Il n'affiche que la declaration de base du projet, telle
qu'ecrite en dehors de tout filtre. La resolution reelle des filtres — le
`-DFILTRE_ACTIF` effectivement transmis au compilateur — n'a lieu que
pendant un `jenga build`, pour une cible precise (configuration + OS +
architecture), jamais pendant un `jenga info`.

Consequence pratique : `jenga info` ne permet pas de verifier si un filtre
s'active ou non. La seule facon fiable de le savoir est de lancer
`jenga build --verbose` et de lire la ligne de commande de compilation
reellement executee, pour y chercher le define. C'est une illustration
supplementaire du theme du chapitre 1 : une faute (ou ici, une simple
absence d'information) ne se voit pas forcement dans l'outil qu'on
croirait le mieux place pour la reveler.
