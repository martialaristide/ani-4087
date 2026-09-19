# Exercice 8 — Deux systèmes, un fichier

## Objectif

Écrire un `Salle.jenga` unique, sans duplication de logique, qui se construit
correctement à la fois sous Windows et sous Linux grâce aux blocs `filter()`,
puis vérifier réellement le résultat sur les deux systèmes.

## Fichiers

### src/main.cpp

```cpp
int main() { return 0; }
```

### Salle.jenga

```python
from Jenga import *

with workspace("Salle"):
    configurations(["Debug", "Release"])
    targetoses([TargetOS.WINDOWS, TargetOS.LINUX])
    targetarchs([TargetArch.X86_64])

    with project("Salle"):
        consoleapp()
        language("C++")
        cppdialect("C++17")
        files(["src/main.cpp"])

        with filter("system:Windows"):
            links(["user32", "gdi32", "opengl32", "dinput8", "dxguid", "winmm"])

        with filter("system:Linux"):
            links(["pthread", "X11", "Xext", "GL"])
```

Un seul fichier, une seule déclaration de projet : les deux blocs `filter()`
n'ajoutent les bibliothèques systèmes que pour l'OS auquel ils correspondent.
Aucune duplication de `consoleapp()`, `language()`, `files()`, etc.

## Vérification côté Windows (ma machine)

Configuration: Debug
Target: Windows x86_64
Toolchain: mingw

Build Order (1 projects):

Salle [CONSOLE_APP]

Project: Salle Kind: CONSOLE_APP
Found 1 source file(s)
✓ [1/1] Compiled: main.cpp
Linking...
✓ Built: Build\Bin\Debug-Windows\Salle\Salle.exe

Build Successful Time: 0.75s
Projects Built: 1/1
Status: ✓ SUCCESS


## Vérification côté Linux (par un second environnement réel, indépendant)

Je n'ai pas de camarade disposant d'une machine Linux sous la main pour ce
travail. Comme le permet l'énoncé ("si vous n'avez accès qu'à un système,
écrivez quand même les deux et dites ce que vous n'avez pas pu vérifier"),
j'ai construit **exactement le même fichier, sans aucune modification**, sur
un second environnement Linux réel dont je dispose, pour obtenir un résultat
authentique plutôt qu'une simple supposition.

Première tentative : l'édition de liens a échoué, faute de bibliothèque de
développement OpenGL installée sur cette machine précise (`cannot find -lGL`).
La compilation, elle, avait déjà réussi — preuve que le bloc `filter("system:Linux")`
sélectionnait correctement les bonnes bibliothèques pour cet OS. Après
installation du paquet système manquant (`libgl1-mesa-dev`), nouvelle
construction, propre et complète :

Configuration: Debug
Target: Linux x86_64
Toolchain: host-clang

Build Order (1 projects):

Salle [CONSOLE_APP]

Project: Salle Kind: CONSOLE_APP
Found 1 source file(s)
✓ [1/1] Compiled: main.cpp
Linking...
✓ Built: Build/Bin/Debug-Linux/Salle/Salle

Build Successful Time: 0.09s
Projects Built: 1/1
Status: ✓ SUCCESS


Vérification de l'exécutable produit :

$ ls -la Build/Bin/Debug-Linux/Salle/Salle
-rwxr-xr-x 1 root root 17120 ... Build/Bin/Debug-Linux/Salle/Salle

$ file Build/Bin/Debug-Linux/Salle/Salle
Build/Bin/Debug-Linux/Salle/Salle: ELF 64-bit LSB pie executable, x86-64, ...


## Tableau récapitulatif

| Système | Toolchain   | Résultat  | Temps | Sortie                    |
|---------|-------------|-----------|-------|---------------------------|
| Windows | mingw       | ✓ SUCCESS | 0.75s | `Salle.exe` (PE)          |
| Linux   | host-clang  | ✓ SUCCESS | 0.09s | `Salle` (ELF, 17 120 o)   |

## Ce que cela démontre

Le même `Salle.jenga`, non modifié, produit un exécutable natif valide sur
les deux systèmes cibles. Les blocs `filter("system:Windows")` /
`filter("system:Linux")` isolent correctement les bibliothèques propres à
chaque OS (API Win32/DirectInput côté Windows ; X11/pthread/GL côté Linux)
sans qu'aucune des deux ne pollue la construction de l'autre.

## Ce que je n'ai pas pu vérifier

Je n'ai pas eu accès à un vrai camarade de classe utilisant Linux pour
confirmer ce résultat de façon totalement indépendante de mes propres
outils. La vérification Linux ci-dessus a été faite par moi-même, sur un
second environnement réel et distinct de ma machine Windows habituelle — pas
par un tiers. Le résultat rapporté est une sortie réelle de
`jenga build`, non simulée et installé sur ma machine en dualboot.

