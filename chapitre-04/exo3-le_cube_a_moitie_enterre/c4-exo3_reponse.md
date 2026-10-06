# Exercice 4.3 — Le cube à moitié enterré

## Objectif

Le cube du moteur est centré sur son origine : à l'échelle 1 (1000 millièmes), il va de −500 à +500 sur chaque axe. Poser ce cube au sol en mettant son centre à une hauteur nulle ne fonctionne donc pas — sa moitié basse reste sous le sol. Le programme calcule, pour chaque cube, où se trouve son bas et son haut, en déduit un verdict, et indique la hauteur de centre qui le poserait correctement.

## La règle

Pour un cube d'échelle verticale `e` (en millièmes, donc aussi en millimètres de hauteur puisque le cube de base mesure 1000 mm) et de centre à la hauteur `y` :

- demi-hauteur : `demi = e / 2`
- bas du cube : `bas = y - demi`
- haut du cube : `haut = y + demi`
- hauteur de centre qui pose le cube : `e / 2`, c'est-à-dire `demi` — elle ne dépend jamais de `y`, seulement de la taille du cube.

Le verdict se choisit dans cet ordre précis, et on s'arrête au premier qui s'applique :

1. `SOUS LE SOL` si `haut <= 0` — le cube entier, y compris son point le plus haut, est sous ou au niveau du sol.
2. `ENTERRE` si `bas < 0` — le cube dépasse le sol par en haut mais sa base est enfoncée.
3. `POSE` si `bas == 0` — exactement posé.
4. `FLOTTE` sinon — la base est strictement au-dessus du sol.

## Pourquoi tester SOUS LE SOL avant ENTERRE

Un cube peut remplir deux conditions à la fois : être "enterré" (`bas < 0`) et avoir aussi son sommet sous le sol (`haut <= 0`). C'est le cas de `cave` dans l'exemple. L'énoncé précise que `SOUS LE SOL` prime : un cube qu'on ne verra jamais (il est intégralement sous le sol) n'est pas simplement "un peu enfoncé", il est invisible. Si le test `ENTERRE` passait en premier, `cave` afficherait à tort `ENTERRE` alors qu'il faut `SOUS LE SOL`.

## Le piège de la demi-hauteur

L'énoncé insiste sur ce point parce que c'est l'erreur la plus naturelle : utiliser `e` au lieu de `e / 2` pour poser le cube. Le cas `lampe` dans l'exemple illustre exactement cette confusion — un cube de 700 mm de haut centré à `y = 700` au lieu de `y = 350` : son bas se retrouve à 350 mm au-dessus du sol, donc `FLOTTE`, alors qu'on croyait l'avoir posé.

## PIRE et la valeur absolue

`PIRE` est la plus grande distance entre le bas d'un cube et le sol, en valeur absolue — donc `abs(bas)`, pas seulement les cas enterrés. Un cube qui flotte haut au-dessus du sol compte aussi dans ce calcul, avec la même formule. `PIRE` vaut 0 s'il n'y a aucun cube, d'où l'initialisation à 0 avant la boucle plutôt qu'au premier écart rencontré.

## Déroulé sur l'exemple fourni

Entrée :
4
unite 1000 0
tabouret 700 350
lampe 700 700
cave 400 -300

- **unite** : `e=1000`, `demi=500`, `y=0`. `bas=-500`, `haut=500`. `haut>0`, `bas<0` → `ENTERRE`. Hauteur de pose : 500.
- **tabouret** : `e=700`, `demi=350`, `y=350`. `bas=0`, `haut=700`. `haut>0`, `bas` pas `<0`, `bas==0` → `POSE`. Hauteur de pose : 350.
- **lampe** : `e=700`, `demi=350`, `y=700`. `bas=350`, `haut=1050`. Aucun des trois premiers tests ne passe → `FLOTTE`. Hauteur de pose : 350 (toujours la demi-hauteur, indépendante de `y`).
- **cave** : `e=400`, `demi=200`, `y=-300`. `bas=-500`, `haut=-100`. `haut<=0` → `SOUS LE SOL`, testé et retenu avant même de regarder `bas`.

Verdicts non-`POSE` : `unite`, `lampe`, `cave`, soit `A CORRIGER 3`.
Écarts absolus des bas : `|-500|=500`, `|0|=0`, `|350|=350`, `|-500|=500` → `PIRE 500`.

Sortie obtenue :
unite -500 500 ENTERRE 500
tabouret 0 700 POSE 350
lampe 350 1050 FLOTTE 350
cave -500 -100 SOUS LE SOL 200
A CORRIGER 3
PIRE 500

Identique, caractère pour caractère, à la sortie exigée par l'énoncé.
