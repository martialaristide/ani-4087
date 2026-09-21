# Exercice 13 — L'inventaire de la chaîne

## Sortie réelle de `jenga info -v`
Available Toolchains
Name Family Target OS Arch Env

host-gcc gcc Windows x86_64 mingw
msvc msvc Windows x86_64 msvc
mingw gcc Windows x86_64 mingw
zig-linux-x86_64 clang Linux x86_64 gnu
zig-linux-x64 clang Linux x86_64 gnu
zig-windows-x86_64 clang Windows x86_64 mingw
zig-windows-x64 clang Windows x86_64 mingw
zig-macos-x86_64 clang macOS x86_64 gnu
zig-macos-arm64 clang macOS arm64 gnu
zig-ios-arm64 clang iOS arm64
zig-tvos-arm64 clang tvOS arm64
zig-watchos-arm64 clang watchOS arm64
zig-android-arm64 clang Android arm64 android
zig-web-wasm32 clang Web wasm32


Système hôte (même sortie) :

System

Host OS: Windows
Host Architecture: x86_64
Host Environment: msvc
Host Triple: x86_64-pc-windows-msvc
Python: 3.12.10 (tags/v3.12.10:0cc8128, Apr 8 2025, 12:21:36) [MSC v.1943 64 bit (AMD64)]
Jenga version: 2.8.0


## Ce qui est présent

Jenga ne liste dans ce tableau que les chaînes d'outils qu'il a
effectivement **trouvées sur le disque et exécutées avec succès** pour
vérifier leur version — je l'ai confirmé en lisant le code source de
Jenga lui-même (`Toolchains.py`), pas en le supposant. Les 14 entrées
sont donc toutes réellement disponibles sur ma machine :

- **`host-gcc` et `mingw`** — un GCC MinGW-w64 (probablement via MSYS2),
  utilisé pour tous les builds Windows de ce cours jusqu'ici.
- **`msvc`** — le compilateur Visual Studio est également détecté et
  fonctionnel, alors qu'aucun exercice de ce cours ne l'a encore utilisé
  (tous mes builds sont passés par `mingw`).
- **Sept variantes `zig-*`** — Zig est installé sur ma machine et peut
  cross-compiler vers Linux, Windows, macOS, iOS, tvOS, watchOS, Android
  et le Web (wasm32), grâce à ses propres bibliothèques/en-têtes embarqués
  (Zig n'a pas besoin des vrais SDK Apple/Android installés localement
  pour cross-compiler).

## Ce qui manque

Rien de listé n'est « présent mais cassé » — mais plusieurs cibles
naturelles n'apparaissent **pas du tout**, faute d'outil détecté :

- **Aucun compilateur natif Linux** (un vrai `gcc`/`clang` qui tourne
  *sous* Linux) — logique, la machine hôte est Windows ; seul le
  cross-compilateur `zig-linux-*` peut viser Linux depuis ici.
- **Aucun toolchain natif macOS/iOS/tvOS/watchOS** (Xcode, `clang-native`)
  — également logique hors d'un Mac ; seules les variantes `zig-*`
  couvrent ces cibles, en cross-compilation.
- **Aucun NDK Android natif** (`host-android` ou équivalent) — seul le
  cross-compilateur `zig-android-arm64` est disponible ; pas de chaîne
  Android « officielle » (Android Studio/NDK) installée.
- **Aucun toolchain de consoles** (Xbox, PlayStation, Switch, etc.) —
  attendu, ce sont des SDK propriétaires non installés ici.

## Conclusion

Sur cette machine, la construction Windows est doublement couverte
(`mingw`/`host-gcc` ET `msvc`), et sept cibles supplémentaires sont
atteignables uniquement grâce à Zig en cross-compilation — sans lui, je
n'aurais que Windows comme cible construisible. Si je voulais un jour
compiler et **exécuter** nativement sur Linux ou macOS (pas seulement
produire un binaire cross-compilé), il faudrait soit une vraie machine
sous cet OS, soit une machine virtuelle/un conteneur avec son propre
toolchain natif installé — ce que ce tableau, à lui seul, ne fournit
pas.
