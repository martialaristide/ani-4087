# Exercice 14 — Le filtre Android

## Fichier

`Salle.jenga` ci-dessus ajoute `TargetOS.ANDROID` aux cibles du workspace
et un bloc `filter("system:Android")` avec sa propre chaîne d'outils
(`android-ndk`), ses defines (`ANDROID`, `NK_ANDROID`) et ses
bibliothèques système Android (`android`, `log`, `EGL`, `GLESv3`, `dl` —
les mêmes catégories que celles utilisées dans le vrai fichier
`Mou.jenga` du moteur Nkentseu, que j'ai consulté pour ne pas inventer
des noms de bibliothèques Android au hasard).

## Première moitié : jenga info ne montre rien

J'ai comparé deux versions du fichier — l'une avec le bloc
`filter("system:Android")` complet, l'autre avec ce bloc totalement
supprimé (mais `TargetOS.ANDROID` toujours présent dans `targetoses`) —
et lancé `jenga info` sur les deux :

Available Toolchains
Name Family Target OS Arch Env

host-clang clang Linux x86_64 gnu
host-gcc gcc Linux x86_64 gnu
clang-mingw clang Windows x86_64 mingw


Sortie strictement identique dans les deux cas — seul le nom du fichier
chargé diffère dans l'en-tête (`Entry file: ...`), rien d'autre. La
ligne `Target OSes: Windows, Android` apparaît dans les deux versions
elle aussi, puisqu'elle vient de `targetoses()`, pas du filtre. Ceci
confirme, exactement comme à l'exercice 6, que `jenga info` ne résout
jamais les blocs `filter()` — avec ou sans bibliothèques/defines Android
à l'intérieur, le résultat est byte pour byte le même.

## Deuxième moitié : comment vérifier que le filtre Android s'active vraiment

En une phrase : en lançant `jenga build --platform Android-arm64
--config Debug`, puisque c'est `jenga build` — pas `jenga info`, qui n'a
même pas d'option `--platform` du tout (vérifié avec `jenga info
--help`) — qui sélectionne réellement une plateforme cible et tente
d'y construire, ce qui force la résolution du filtre `system:Android` et
révèle immédiatement si la chaîne d'outils NDK correspondante est
présente ou non.

À titre de vérification réelle : sur ma machine, sans NDK Android
installé, cette commande échoue proprement avec :

Cannot create builder: No suitable toolchain found for Android arm64

— ce qui prouve que la commande a bien tenté de résoudre le filtre pour
Android (contrairement à `jenga info`, qui reste muet), et que l'échec
vient de l'absence de la chaîne d'outils, pas d'une erreur dans le
fichier de projet.
