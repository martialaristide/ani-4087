# Chapitre 3 — Exercice 5 : la taille qui change

## Le rappel ajouté

```cpp
NkEvents().AddEventCallback<NkWindowResizeEvent>([](NkWindowResizeEvent *e) {
        std::printf("Nouvelle taille : %u x %u\n", e->GetWidth(), e->GetHeight());
});
```

Fichier complet : [`00-LaTailleQuiChange/main.cpp`](00-LaTailleQuiChange/main.cpp).
Projet enregistré dans [`Tutoriels3D.jenga`](Tutoriels3D.jenga) via le même
helper `tutoproject()` que les exercices précédents.

## Test 1 — redimensionnement lent (bord tiré très doucement, ~3 s)

Nouvelle taille : 1295 x 759
Nouvelle taille : 1277 x 712
Nouvelle taille : 1294 x 759
Nouvelle taille : 1276 x 712
Nouvelle taille : 1293 x 759
Nouvelle taille : 1275 x 712
Nouvelle taille : 1292 x 759
Nouvelle taille : 1274 x 712
Nouvelle taille : 1291 x 759
Nouvelle taille : 1273 x 712
Nouvelle taille : 1290 x 759
Nouvelle taille : 1272 x 712
Nouvelle taille : 1289 x 759
Nouvelle taille : 1271 x 712


**14 événements** pour un déplacement d'environ 6 pixels en largeur : chaque
événement ne fait avancer la taille que d'1 pixel.

## Test 2 — redimensionnement d'un coup (un seul geste rapide et bref)

Nouvelle taille : 1291 x 759
Nouvelle taille : 1273 x 712
Nouvelle taille : 1267 x 759
Nouvelle taille : 1249 x 712
Nouvelle taille : 1109 x 759
Nouvelle taille : 1091 x 712
Nouvelle taille : 1055 x 759
Nouvelle taille : 1037 x 712
Nouvelle taille : 1035 x 759
Nouvelle taille : 1017 x 712
Nouvelle taille : 1028 x 759
Nouvelle taille : 1010 x 712
Nouvelle taille : 1026 x 759
Nouvelle taille : 1008 x 712
Nouvelle taille : 1024 x 759
Nouvelle taille : 1006 x 712
Nouvelle taille : 1023 x 759
Nouvelle taille : 1005 x 712
Nouvelle taille : 1022 x 759
Nouvelle taille : 1004 x 712
Nouvelle taille : 1021 x 759
Nouvelle taille : 1003 x 712


**22 événements**, mais avec un profil très différent du test lent : les
premiers événements couvrent de grands écarts d'un coup (1267 → 1109, soit
158 pixels de largeur en un seul événement), puis la fin du geste ralentit
naturellement (la main freine avant de relâcher) et retombe sur des écarts
d'1 à 2 pixels par événement, comme dans le test lent.

## Ce qu'on en conclut sur le nombre d'événements

Le nombre d'événements reçus n'est pas proportionnel à la distance totale
parcourue par le bord de la fenêtre, mais à la façon dont le système
d'exploitation découpe le geste en messages de redimensionnement (`WM_SIZE`
sous Windows, un par NkWindowResizeEvent) :

- **Un geste lent** est échantillonné très finement par le système : chaque
  micro-déplacement de la souris déclenche son propre événement, avec un
  écart d'à peine 1 pixel entre deux tailles consécutives — d'où beaucoup
  d'événements pour peu de changement réel.
- **Un geste rapide** n'est pas ignoré, mais il est condensé : le système ne
  peut pas suivre la souris pixel par pixel à cette vitesse, donc chaque
  événement représente un bond de plusieurs dizaines, voire plus d'une
  centaine de pixels. Le nombre total d'événements peut malgré tout rester
  élevé (22 ici, contre 14 pour le test lent), mais leur *répartition* est
  très inégale : quelques gros sauts au début, puis de petits pas à la fin
  quand le mouvement ralentit avant l'arrêt.

En clair : ce n'est jamais un seul événement final qui est émis pour un
redimensionnement, mais une série — dont la densité dépend de la vitesse du
geste, pas de la distance parcourue. Un programme qui voudrait réagir "à la
fin" du redimensionnement (recalcul de layout coûteux, par exemple) doit
donc écouter des événements dédiés de début/fin d'opération plutôt que
d'agir à chaque `NkWindowResizeEvent` reçu.
