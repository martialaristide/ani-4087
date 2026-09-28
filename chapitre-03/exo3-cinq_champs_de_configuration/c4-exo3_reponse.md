# Chapitre 3 — Exercice 3 : cinq champs de configuration

Cinq champs de `NkWindowConfig` (hors `title`, `width`, `height` déjà utilisés
par le chapitre), testés un par un sur la même fenêtre. Méthode : une seule
ligne ajoutée dans `config` à chaque fois, reconstruction (`jenga build
--target CinqChampsConfig --config Release`), lancement, observation.

Fichier source (dernière variante testée, les 4 précédentes en commentaire
pour traçabilité) : [`00-CinqChampsConfig/main.cpp`](00-CinqChampsConfig/main.cpp).
Projet enregistré dans [`Tutoriels3D.jenga`](Tutoriels3D.jenga) via le même
helper `tutoproject()` que les exercices précédents.

## 1. `config.resizable = false;`

**Attendu :** impossible d'agrandir/réduire la fenêtre en tirant ses bords ou
ses coins.

**Observé :** aucun effet — la fenêtre s'est redimensionnée normalement sur
les bords et les coins, comme si le champ n'était pas lu à ce niveau.

**Pourquoi (hypothèse) :** le backend Win32 de `NkWindow` (`NkWin32Window.cpp`)
ne traduit probablement pas encore ce champ vers le style de fenêtre natif
correspondant (`WS_THICKFRAME` retiré côté `CreateWindowEx`/`SetWindowLong`) ;
le champ existe dans la structure de configuration mais n'est pas branché
jusqu'au bout côté création de fenêtre native.

## 2. `config.frame = false;`

**Attendu :** plus de barre de titre ni de bordure (fenêtre "sans cadre").

**Observé :** conforme à l'attente — la fenêtre apparaît comme un rectangle nu,
sans barre de titre ni bordure. En revanche son contenu ne s'affiche pas
proprement (résidus visuels, pas de remplissage net) — capture :
[`capture-2-frame.png`](capture-2-frame.png).

**Pourquoi (le contenu glitché) :** ce programme n'appelle jamais de fonction
de rendu/présentation (`Present`, `SwapBuffers`, `Clear`...), juste
`PollEvents()` en boucle. Sans le frame système pour gérer le repaint
automatique du client, et sans qu'aucune frame ne soit dessinée par
l'application, le contenu visible reste indéfini (ce qu'il y avait à l'écran
au moment de la création, jamais rafraîchi).

## 3. `config.bgColor = 0xFF0000FF;` (rouge opaque)

**Attendu :** le fond de la fenêtre passe du gris/noir par défaut au rouge.

**Observé :** aucun effet visible — le fond reste blanc (fond de fenêtre par
défaut de Windows en l'absence de tout rendu).

**Pourquoi (hypothèse) :** `bgColor` est vraisemblablement une couleur de
*clear* utilisée par le moteur de rendu (`NKRHI`/`NKRenderer`) au moment de
dessiner une frame, pas une couleur appliquée directement par la fenêtre
native au niveau système. Ce programme ne fait tourner aucune boucle de rendu
(pas de `Clear`/`Present`), donc ce champ n'a nulle part où s'appliquer, et
Windows affiche son remplissage par défaut.

## 4. `config.opacity = 0.5f;`

**Attendu :** la fenêtre devient semi-transparente, on doit voir le bureau à
travers.

**Observé :** conforme à l'attente — la fenêtre "Ma salle" est nettement
semi-transparente, le fond d'écran est visible à travers tout en gardant la
barre de titre normale. Capture : [`capture-4-opacity.png`](capture-4-opacity.png).

## 5. `config.alwaysOnTop = true;`

**Attendu :** la fenêtre reste visible au-dessus des autres fenêtres même en
cliquant ailleurs.

**Observé :** conforme à l'attente — la fenêtre "Ma salle" reste au premier
plan par-dessus le terminal, même après avoir cliqué sur celui-ci. Capture :
[`capture-5-alwaysontop.png`](capture-5-alwaysontop.png).

## Bilan

Sur les cinq champs testés, deux n'ont produit aucun effet visible
(`resizable`, `bgColor`) et trois se sont comportés exactement comme prévu
(`frame`, `opacity`, `alwaysOnTop`). Les deux échecs partagent un point commun
plausible : ce sont des champs qui, soit dépendent d'un moteur de rendu actif
(`bgColor`), soit semblent ne pas être encore complètement câblés jusqu'au
backend natif Win32 (`resizable`) — à la différence de `frame`, `opacity` et
`alwaysOnTop`, qui sont appliqués directement à la création/au style de la
fenêtre système et fonctionnent donc indépendamment de toute boucle de rendu.
