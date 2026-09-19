# Exercice 3 — Les deux configurations

## Les quatre nombres

| Configuration | Temps de construction | Taille de l'executable |
|----------------|------------------------|--------------------------|
| Debug          | 0,26 s                 | 134 482 octets           |
| Release        | 0,27 s                 | 134 482 octets           |

## Sortie brute

### `jenga build --config Debug` (apres `jenga clean --config Debug`, pour une mesure comparable)
\`\`\`
Loading workspace...

Configuration: Debug
Target:        Windows x86_64
Toolchain:     mingw

Build Order (1 projects):
  1. Salle [CONSOLE_APP]

Project: Salle                                                           Kind: CONSOLE_APP

Found 1 source file(s)
[1/1] Compiled: main.cpp
Linking...
Built: Build\Bin\Debug-Windows\Salle\Salle.exe

Build Successful                                                             Time: 0.26s

BUILD COMPLETED
Projects Built:  1/1
Time:           0.26s
Status:         SUCCESS
\`\`\`

### `jenga build --config Release`
\`\`\`
Loading workspace...

Configuration: Release
Target:        Windows x86_64
Toolchain:     mingw

Build Order (1 projects):
  1. Salle [CONSOLE_APP]

Project: Salle                                                           Kind: CONSOLE_APP

Found 1 source file(s)
[1/1] Compiled: main.cpp
Linking...
Built: Build\Bin\Release-Windows\Salle\Salle.exe

Build Successful                                                             Time: 0.27s

BUILD COMPLETED
Projects Built:  1/1
Time:           0.27s
Status:         SUCCESS
\`\`\`

## Commentaire

Les deux executables font exactement la meme taille (134 482 octets), et les
temps de construction sont quasi identiques (0,26 s contre 0,27 s, un ecart
qui releve du bruit de mesure plutot que d'un signal reel). Ce resultat ne
confirme pas l'idee intuitive qu'un build Release serait plus petit ou plus
long a construire (a cause de l'optimisation) : pour un programme aussi
minimal que `int main() { return EXIT_SUCCESS; }`, il n'y a litteralement
rien a optimiser, ni de symboles de debogage significatifs a ajouter ou
retirer. L'ecart entre Debug et Release ne deviendra visible que sur du
vrai code, avec de la logique a optimiser et des structures de donnees a
instrumenter en debug. Important de ne pas fabriquer un ecart qui n'existe
pas simplement parce qu'on s'y attendait.

Remarque methodologique : ma premiere mesure du temps Debug (0,04 s) etait
faussee car Jenga avait reutilise un exécutable deja construit lors de
l'exercice 1 ("All files up to date") au lieu de recompiler. J'ai relance
un `jenga clean --config Debug` suivi d'un `jenga build --config Debug`
pour obtenir une vraie mesure de compilation, comparable a celle de
Release.
