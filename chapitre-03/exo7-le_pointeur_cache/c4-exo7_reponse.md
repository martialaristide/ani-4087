# Chapitre 3 — Exercice 7 : le pointeur caché

## Le code

```cpp
fenetre.ShowMouse(false);
fenetre.ClipMouseToClient(true);

while (enCours) {
        NkEvents().PollEvents();

        std::printf("pos = (%d, %d)   rawDelta = (%d, %d)\n",
                                NkInput.MouseX(), NkInput.MouseY(),
                                NkInput.MouseRawDeltaX(), NkInput.MouseRawDeltaY());
}
```

Fichier complet : [`00-LePointeurCache/main.cpp`](00-LePointeurCache/main.cpp).
Fermeture propre (croix + Échap) reprise des exercices 4 et 6.

## Test : déplacement jusqu'au bord

Extrait représentatif de la console (position atteignant le coin haut-droit,
puis mouvement de souris prolongé une fois "bloquée") :

pos = (1229, 21) rawDelta = (0, -28)
pos = (1229, 0) rawDelta = (0, -29) <- y touche le bord haut (0)
pos = (1235, 0) rawDelta = (3, -27)
pos = (1259, 0) rawDelta = (0, -1)
pos = (1261, 0) rawDelta = (2, -5)
pos = (1277, 0) rawDelta = (10, -16) <- x touche le bord droit (1277)
pos = (1277, 0) rawDelta = (10, -16)
pos = (1277, 0) rawDelta = (0, -1)
pos = (1277, 0) rawDelta = (1, 0)
pos = (1277, 0) rawDelta = (1, 0) <- pos totalement fige, rawDelta continue
pos = (1277, 0) rawDelta = (2, -1)
pos = (1277, 0) rawDelta = (5, -3)
pos = (1277, 0) rawDelta = (12, -20)


Une fois `pos` arrivé à `(1277, 0)` (le coin haut-droit de la fenêtre 1280×720,
compte tenu de la taille du curseur/bordure), il ne change plus du tout —
`x` et `y` restent strictement figés à ces valeurs, quel que soit le
mouvement réel de la souris. `rawDelta`, en revanche, continue de varier
sans interruption pendant toute la durée du test, y compris longtemps après
que `pos` a cessé de bouger.

## Laquelle continue de bouger, et pourquoi c'est celle-là qu'il faut

**`rawDelta` continue de bouger** ; `pos` se fige dès que le curseur atteint
le bord de la zone de confinement (`ClipMouseToClient`).

C'est logique avec ce que représente chacune des deux valeurs :

- **`pos` (`MouseX`/`MouseY`)** est une position **absolue**, exprimée dans
  les coordonnées de la fenêtre. Une position absolue est nécessairement
  bornée par la taille de l'écran ou de la zone de confinement : le
  curseur ne peut matériellement pas avoir de coordonnée "hors fenêtre"
  puisqu'il est physiquement retenu à l'intérieur (`ClipMouseToClient(true)`).
  Une fois au bord, il n'y a plus nulle part où aller : la position reste
  donc collée à la valeur limite.

- **`rawDelta`** ne décrit pas une position mais un **déplacement physique
  relatif** de la souris entre deux lectures, indépendant de toute notion
  de bord ou d'écran — c'est la donnée brute remontée par le périphérique
  (ou son pilote), avant tout clampage. La souris elle-même continue de
  glisser sur le tapis, donc cette valeur continue de refléter fidèlement
  ce mouvement réel, même si le curseur affiché (ou confiné) n'a plus où
  se déplacer.

**C'est `rawDelta` qu'il faut utiliser** pour tout ce qui relève d'un
contrôle par mouvement continu et non borné — le cas typique étant la
caméra à la première personne en VR/FPS (rotation de la tête ou du regard
en fonction du mouvement de la souris). Si on pilotait la caméra avec `pos`,
la rotation s'arrêterait net dès que le curseur touche un bord de l'écran,
alors que l'utilisateur continue physiquement de bouger sa souris dans
cette direction : la caméra resterait bloquée tant qu'il ne ramène pas la
souris vers le centre. En pilotant avec `rawDelta`, ce problème disparaît
complètement puisque cette valeur n'est jamais bornée par la taille de
l'écran — c'est précisément pour ce cas d'usage que `ShowMouse(false)` +
`ClipMouseToClient(true)` sont utilisés ensemble : masquer/confiner
`pos` (qui n'a plus d'utilité visuelle une fois le curseur caché) tout en
lisant `rawDelta` pour un contrôle fluide et illimité.
