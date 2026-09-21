# Exercice 9 — Le chemin qui ne désigne rien

## Objectif

Ajouter à `files` un motif qui ne correspond à aucun fichier, et à
`includedirs` un dossier qui n'existe pas. Observer ce que `jenga info` et
`jenga build` en disent chacun, et comparer.

## Salle.jenga (avec les deux pièges)

```python
from Jenga import *

with workspace("Salle"):
    configurations(["Debug", "Release"])
    targetoses([TargetOS.WINDOWS])
    targetarchs([TargetArch.X86_64])

    with project("Salle"):
        consoleapp()
        language("C++")
        cppdialect("C++17")
        files(["src/main.cpp", "src/**.introuvable"])
        includedirs(["include", "chemin/qui/n_existe_pas"])
```

- `src/**.introuvable` : motif glob qui ne correspond à aucun fichier réel du
  projet.
- `chemin/qui/n_existe_pas` : dossier passé à `includedirs` alors qu'il
  n'existe nulle part sur le disque.

## src/main.cpp

```cpp
int main() { return 0; }
```

## Sortie réelle de `jenga info`
Location: C:\Users\Lenovo\ani-4087\chapitre-02\exo9-le_chemin_qui_ne_designe_rien
Entry file: C:\Users\Lenovo\ani-4087\chapitre-02\exo9-le_chemin_qui_ne_designe_rien\Salle.jenga
Configurations: Debug, Release
Platforms: Windows
Target OSes: Windows
Target Architectures: x86_64

Projects
Name Kind Language Test External

Salle ConsoleApp C++ No No

Available Toolchains
Name Family Target OS Arch Env
host-gcc gcc Windows x86_64 mingw
msvc msvc Windows x86_64 msvc
mingw gcc Windows x86_64 mingw
zig-linux-x86_64 clang Linux x86_64 gnu
zig-windows-x86_64 clang Windows x86_64 mingw
...
Daemon

Status: Not running

Aucune mention de `files`, d'`includedirs`, ni d'un quelconque avertissement.
`jenga info` n'affiche que le nom du projet, son type, son langage — jamais
la liste résolue des fichiers sources ni des dossiers d'en-têtes.

## Sortie réelle de `jenga build --config Debug`
Configuration: Debug
Target: Windows x86_64
Toolchain: mingw

Build Order (1 projects):

Salle [CONSOLE_APP]

Project: Salle Kind: CONSOLE_APP
ℹ Found 1 source file(s)
✓ [1/1] Compiled: main.cpp
ℹ Linking...
✓ Built: Build\Bin\Debug-Windows\Salle\Salle.exe

Build Successful Time: 0.25s
Projects Built: 1/1
Status: ✓ SUCCESS


`Found 1 source file(s)` : seul `src/main.cpp` a été retenu. Le motif
`src/**.introuvable` n'a produit ni fichier ni erreur — il a simplement
contribué zéro fichier à la liste, en silence. Le dossier
`chemin/qui/n_existe_pas` passé à `includedirs` n'apparaît nulle part non
plus : la construction réussit intégralement.

## Comparaison : lequel vous aurait fait gagner du temps ?

**Aucun des deux**, dans ce cas précis — et c'est ça, le vrai constat de
l'exercice. J'ai vérifié directement dans le code source de Jenga (Api.py) :

- Un motif `files()` qui ne correspond à rien passe par `Path.glob(pattern)`,
  qui renvoie simplement une liste vide s'il n'y a aucune correspondance —
  ce n'est traité nulle part comme une erreur ou un avertissement.
- `includedirs()` stocke la chaîne telle quelle, sans jamais vérifier que le
  dossier existe. Elle devient un simple drapeau `-I` passé au compilateur ;
  un compilateur C++ n'échoue pas sur un `-I` pointant vers un dossier
  inexistant, tant qu'aucun `#include` n'a besoin d'y chercher un en-tête.

Résultat : ni `jenga info`, ni `jenga build` (même avec `--verbose`, testé
aussi) ne révèlent ces deux erreurs tant qu'elles n'ont pas d'effet visible
sur la compilation. La seule façon de les détecter est la relecture
attentive du fichier `Salle.jenga` lui-même — ou le jour où le fichier
manquant/le dossier manquant est réellement nécessaire, et où l'erreur
n'apparaît alors que beaucoup plus tard, sous une autre forme (un fichier
qu'on croyait inclus dans le build mais qui ne l'est pas, ou un en-tête
introuvable au moment où on en a enfin besoin).
