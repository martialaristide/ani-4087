# Demo2 — Le symbole non résolu

## Première tentative (piège) : retirer EGL
J'ai d'abord retiré `EGL` de `links([...])` dans `Salle.jenga`, en m'attendant à une erreur
de link (le code appelle bien `eglGetDisplay`, `eglInitialize`, etc.). Le build a **réussi**
quand même. En inspectant le code source de Jenga
(`Core/Builders/Android.py`, fonction `_GetLinkerFlags`), j'ai trouvé pourquoi : pour toute
application non-console (`windowedapp`), le builder Android force lui-même
`-llog -landroid -lEGL -lGLESv2` dans les flags de link, **indépendamment** de ce que dit
`links()` dans le fichier de projet. Retirer `EGL` de `links()` n'a donc aucun effet — c'est
une découverte en soi, digne d'être signalée : le fichier de projet ne peut pas désactiver
ces bibliothèques de base.

## Vraie tentative : retirer une dépendance qui compte
J'ai ajouté un appel à `crc32()` (zlib) dans `main.cpp`, avec `z` dans `links()` — ça build
sans problème. Puis j'ai retiré `z` de `links()` (en gardant l'appel à `crc32()` dans le
code) et relancé le build :

```bash
jenga clean --platform Android-arm64 --config Release
jenga build --platform Android-arm64 --config Release --no-cache
```

## L'erreur d'édition de liens, en entier, sans troncature

```
Loading workspace...

Configuration: Release
Target:        Android arm64
Toolchain:     android-ndk

Build Order (1 projects):
  1. Salle [WINDOWED_APP]


╔══════════════════════════════════════════════════════════════════════════════════════════════╗
║  Project: Salle                                                          Kind: WINDOWED_APP  ║
╚══════════════════════════════════════════════════════════════════════════════════════════════╝

ℹ Found 1 source file(s)
✓   [1/1] Compiled: main.cpp
ℹ Linking...
╔══════════════════════════════════════════════════════════════════════════════════════════════╗
║                                Compilation Error: Link Failed                                ║
╠══════════════════════════════════════════════════════════════════════════════════════════════╣
║ ld.lld: error: undefined symbol: crc32                                                       ║
║ >>> referenced by main.cpp                                                                   ║
║ >>>                                                                                          ║
║ C:\Users\Lenovo\ani-4087\chapitre-02\exo16-le_paquet_vide\Build\Obj\Release-Android\Salle\sr ║
║ c_main.o:(android_main)                                                                      ║
║ clang++: error: linker command failed with exit code 1 (use -v to see invocation)            ║
╚══════════════════════════════════════════════════════════════════════════════════════════════╝

┌──────────────────────────────────────────────────────────────────────────────────────────────┐
│  ✗ Build Failed                                                                 Time: 4.17s  │
│ Errors: 2  | Failed files: 1                                                                 │
└──────────────────────────────────────────────────────────────────────────────────────────────┘

════════════════════════════════════════════════════════════════════════════════
                                  BUILD FAILED
════════════════════════════════════════════════════════════════════════════════
Projects Built:  0/1
Failed:         1
Errors:         2
Time:           4.17s
Status:         ✗ FAILURE
════════════════════════════════════════════════════════════════════════════════

Echecs (1) — a corriger :
  ✗ Salle
```

## Question à la classe
Dans ce mur de texte, quelle information désigne précisément le module manquant ?

**Réponse attendue :** la ligne `ld.lld: error: undefined symbol: crc32` — c'est le nom du
symbole non résolu qui pointe vers le module manquant (ici, `crc32` fait partie de zlib,
donc la bibliothèque `libz` n'est pas liée). La ligne suivante,
`>>> referenced by main.cpp ... (android_main)`, ne désigne pas le module manquant mais son
**utilisateur** : c'est cette confusion — chercher la cause dans "qui appelle" plutôt que
dans "quel symbole" — qui piège le plus souvent en première lecture.
