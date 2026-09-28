# Chapitre 3 — Exercice 2 : la fenêtre qui ne répond pas

## Le changement

Repris du programme de l'exercice 1, en commentant le seul appel de la boucle
(plus aucun traitement de la file d'événements Windows) :

```cpp
while (fenetre.IsOpen()) {
        // NkEvents().PollEvents();  // volontairement commente : plus aucun
        // traitement de la file d'evenements Windows -> le systeme va
        // considerer la fenetre comme non-reactive au bout de quelques
        // secondes ("Ma salle (Ne repond pas)").
}
```

Fichier complet : [`00-FenetreQuiNeRepondPas/main.cpp`](00-FenetreQuiNeRepondPas/main.cpp).

Même méthode d'intégration que l'exercice 1 : nouveau projet `LaFenetreQuiNeRepondPas`
ajouté au `Tutoriels3D.jenga` existant via le helper `tutoproject()`.

## Construction

$ export JENGA_TRUST_ALL=1
$ cd /c/Users/Lenovo/Nkentseu/Nkentseu
$ jenga build --target LaFenetreQuiNeRepondPas --config Release


Accroc technique en cours de route : le cache de précompilation (`.pch`) d'un
build précédent était périmé (format non reconnu par le compilateur actuel),
et l'archiveur `llvm-ar` attendu par la toolchain `clang-mingw` n'était pas
sur le `PATH` (seul `ar.exe` de MSYS2/ucrt64 y était). Solution : vider le
cache de build (`Build/Obj/Release-Windows`) et fournir une copie de `ar.exe`
sous le nom `llvm-ar.exe`. Une fois cela fait, les 25 projets ont compilé et
lié sans erreur, produisant `LaFenetreQuiNeRepondPas.exe`.

## Lancement et chronométrage

$./Build/Bin/Release-Windows/LaFenetreQuiNeRepondPas/LaFenetreQuiNeRepondPas.exe


Chrono démarré dès l'apparition de la fenêtre, sans toucher à rien ensuite.

**Résultat : 2 min 14,86 s (134,86 s)** avant que Windows ne marque la fenêtre
« Ma salle (Ne répond pas) » dans la barre de titre, avec le contenu grisé/blanchi
— capture ci-jointe : [`capture-ne_repond_pas.png`](capture-ne_repond_pas.png).

Ce délai est nettement plus long que le seuil de détection habituel de Windows
pour une fenêtre bloquée (de l'ordre de quelques secondes dans le cas général).
Une explication plausible : `NkWindow`/`NkWin32Window` peut avoir sa propre
boucle interne ou un mécanisme qui répond encore ponctuellement à certains
messages système même sans appel explicite à `PollEvents()` côté application,
retardant d'autant le diagnostic « ne répond pas » du système — à vérifier au
besoin en inspectant `NkWin32EventSystem.cpp` / `NkWin32Window.cpp`.

## Temps passé sur l'exercice

Non chronométré précisément sur le moment ; l'essentiel du temps a été pris par
le dépannage de la toolchain (cache `.pch` périmé, `llvm-ar` manquant) plutôt
que par l'exercice lui-même, qui ne modifie qu'une ligne du programme précédent.
