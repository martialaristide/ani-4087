# Exercice 4 — La dependance retiree

## Montage

- ModuleB definit `int GetValue()`.
- ModuleA depend de ModuleB et appelle `GetValue()` dans `UseModuleB()`.
- Salle depend de ModuleA seulement : ModuleB a ete retire de `links()` et
  `dependson()` de Salle, alors qu'il reste necessaire a ModuleA.

## Message d'erreur exact, en entier

\`\`\`
C:/msys64/ucrt64/bin/../lib/gcc/x86_64-w64-mingw32/16.1.0/../../../../x86_64-w64-mingw32/bin/ld.exe: Build\Lib\Debug-Windows\ModuleA\ModuleA.lib(src_ModuleA.obj): in function `UseModuleB()':
C:/Users/Lenovo/ani-4087/chapitre-02/exo4-la_dependance_retiree/ModuleA/src/ModuleA.cpp:3:(.text+0x9): undefined reference to `GetValue()'
collect2.exe: error: ld returned 1 exit status
\`\`\`

## Etape de la chaine de construction

Edition de liens (la 4e etape, apres preprocesseur, compilation et
assemblage). Le preprocesseur et la compilation de ModuleA.cpp reussissent
sans probleme : le compilateur n'a besoin que de la declaration de
`GetValue()` (le .hpp) pour compiler l'appel dans `UseModuleB()`, pas de
son corps. La preuve est dans le message lui-meme : l'erreur ne vient pas
du compilateur mais de `ld.exe`, l'editeur de liens que `g++` invoque via
`collect2.exe`. C'est seulement au moment d'assembler l'executable final
Salle que `ld` cherche le code machine reel de `GetValue()`, present dans
ModuleB.lib, et ne le trouve pas : ce fichier ne lui a jamais ete transmis,
la commande de liaison ne passant que `ModuleA.lib` (verifie avec
`jenga build --verbose`, qui affiche la commande complete). ModuleA sait
qu'il a besoin de ModuleB ; Salle, lui, ne le sait pas. C'est exactement le
piege du chapitre 3 : une dependance transitive ne se propage pas
automatiquement d'un module a l'executable final qui en depend
indirectement.

## Note methodologique

Le meme message n'apparaissait pas via `jenga build` ni `jenga build
--verbose` lances depuis Git Bash : seule la derniere ligne
(`collect2.exe: error: ld returned N exit status`, avec un code d'erreur
variable selon les tentatives) etait visible, un probleme d'affichage connu
entre le pseudo-terminal de Git Bash/MSYS et les executables Windows natifs
comme `ld.exe`. Le message complet et fiable a ete obtenu en relancant la
meme commande de liaison directement dans l'invite de commandes Windows
(CMD).
