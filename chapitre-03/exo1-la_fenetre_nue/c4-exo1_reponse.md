# Chapitre 3 — Exercice 1 : la fenêtre nue

## Le programme de quinze lignes

```cpp
#include "NKWindow/NKMain.h"
#include "NKWindow/Core/NkWindow.h"
#include "NKWindow/Core/NkWindowConfig.h"

using namespace nkentseu;

static void ConfigureAppData(NkAppData &d) {
        d.appName = "LaFenetreNue";
}
NK_REGISTER_ENTRY_APPDATA_UPDATER(ConfigureAppData)

int nkmain(const NkEntryState &state) {
        (void)state;

        NkWindowConfig config;
        config.title  = "Ma salle";
        config.width  = 1280;
        config.height = 720;

        NkWindow fenetre(config);
        if (!fenetre.IsValid()) {
                return 1;
        }

        while (fenetre.IsOpen()) {
                NkEvents().PollEvents();
        }

        return 0;
}
```

Fichier complet : [`00-FenetreNue/main.cpp`](00-FenetreNue/main.cpp).

## Le choix d'implantation

Je dispose déjà d'un moteur (NKA Engine / Nkentseu) avec sa propre couche fenêtrage
(`NKWindow`) et son propre point d'entrée (`nkmain`, macro `NK_REGISTER_ENTRY_APPDATA_UPDATER`).
Plutôt que de repartir de zéro avec un projet Jenga isolé, j'ai enregistré cet
exercice comme un sixième projet dans le fichier `Tutoriels3D.jenga` existant du
dépôt Nkentseu, en réutilisant le helper `tutoproject()` déjà en place pour les
cinq étapes de la progression du cours interne :

```python
# ===== Exercice chapitre 3 — la fenetre nue (ajout personnel) ====================
tutoproject("LaFenetreNue", ["00-FenetreNue/main.cpp"])
```

Ce helper attache automatiquement
$ cd /c/Users/Lenovo/Nkentseu/Nkentseu
$ jenga build --target LaFenetreNue --config Release


Compilation des 25 projets de la chaîne de dépendances (NKPlatform → ... →
LaFenetreNue), en configuration Release, toolchain mingw, cible Windows x86_64.
Les 25 projets ont compilé et lié sans erreur, produisant `LaFenetreNue.exe`.

Lancé directement, l'exécutable ouvre une fenêtre native intitulée **"Ma salle"**
(1280×720), avec la barre de titre et les boutons standard (réduire/agrandir/
fermer) — capture ci-jointe : [`capture-ma_salle.png`](capture-ma_salle.png).

## Temps passé

**Environ 2h30**, non chronométré précisément sur le moment — cette durée est
reconstituée après coup à partir de l'horodatage de mes échanges avec l'assistant
(du premier message sur cet exercice à l'envoi de la capture de la fenêtre
ouverte). Elle inclut donc aussi les pauses et les allers-retours de discussion,
pas seulement le temps de travail effectif. Je m'en sers comme première mesure
de référence pour comparer mes prochains exercices.
