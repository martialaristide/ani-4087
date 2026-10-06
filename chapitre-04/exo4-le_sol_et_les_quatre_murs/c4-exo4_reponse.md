# Exercice 4.4 — Le sol et les quatre murs

## Objectif

Vérifier que les quatre angles d'une pièce rectangulaire sont réellement fermés par les murs, et non simplement "à peu près" couverts. Une pièce dont les murs font exactement la longueur du sol laisse, à chaque coin, un trou carré de la taille de l'épaisseur des murs — invisible de l'intérieur, flagrant dès qu'on sort de la pièce.

## Les données géométriques

Vu de dessus, `x` va vers la droite, `z` vers l'entrée (donc le fond est en `z` négatif, la gauche en `x` négatif). Le sol fait `L` de côté, centré sur l'origine : il va de `-L/2` à `L/2`. On note `h = L/2`.

Chaque mur est décrit par son centre `(cx, cz)` et sa taille `(sx, sz)`. Son emprise au sol (un rectangle) est :
xmin = cx - sx/2 xmax = cx + sx/2
zmin = cz - sz/2 zmax = cz + sz/2

## Les quatre carrés d'angle

Chaque angle est un carré de côté `e` (l'épaisseur des murs), **juste à l'extérieur du sol** — c'est-à-dire sur le prolongement du sol, dans l'espace que les murs sont censés occuper :

| Angle | x | z |
|---|---|---|
| FOND_GAUCHE | `-h-e` à `-h` | `-h-e` à `-h` |
| FOND_DROIT | `h` à `h+e` | `-h-e` à `-h` |
| ENTREE_GAUCHE | `-h-e` à `-h` | `h` à `h+e` |
| ENTREE_DROIT | `h` à `h+e` | `h` à `h+e` |

## La règle de "bouché" — et son piège

Un angle est `BOUCHE` si **un seul mur**, pris isolément, contient le carré en entier sur les deux axes :
mur.xmin <= carre.xmin ET mur.xmax >= carre.xmax
ET mur.zmin <= carre.zmin ET mur.zmax >= carre.zmax

Le piège explicite de l'énoncé : deux murs qui couvrent chacun la moitié du carré ne le bouchent pas. Dans l'exemple de base, le mur `fond` s'arrête à `x = -2000` alors que l'angle `FOND_GAUCHE` va jusqu'à `x = -2100` ; le mur `gauche` s'arrête à `z = -2000` pour le même angle qui va jusqu'à `z = -2100`. Chacun des deux murs couvre une moitié du carré, mais ni l'un ni l'autre ne le couvre en entier — donc `TROU`. C'est pour cette raison que la fonction de vérification teste chaque mur **seul**, jamais une union de plusieurs murs : une union donnerait à tort `BOUCHE` dans ce cas précis.

Un mur qui touche l'angle par un bord sans le dépasser ne suffit pas non plus : il faut une inégalité large dans le bon sens sur les quatre bornes, mais sur les QUATRE à la fois pour UN SEUL mur.

## Pourquoi allonger les murs du fond et de l'entrée règle le problème

Allonger `fond` et `entree` de deux épaisseurs (`sx` de 4000 à 4200) étire leur emprise de `-2000..2000` à `-2100..2100` en `x`. Ce nouveau `xmin = -2100` atteint exactement le bord externe des angles FOND_GAUCHE et FOND_DROIT (`-h-e = -2100` et `h+e = 2100`). Le mur `fond` contient alors, à lui seul, tout l'angle FOND_GAUCHE en `x` (`-2100` à `2100` recouvre `-2100` à `-2000`) ET tout son carré en `z` (`-2100` à `-2000` recouvre exactement `-2100` à `-2000`) : `BOUCHE`. Même raisonnement pour `entree` sur les deux angles côté entrée. Les murs `gauche` et `droit`, inchangés, n'ont pas besoin d'être allongés puisque ce sont désormais `fond` et `entree` qui couvrent chaque angle en entier.

## Cas limite : aucun mur

Si `N = 0`, aucun mur n'est testé ; la fonction qui cherche un mur contenant le carré ne trouve jamais de candidat, donc les quatre angles sont `TROU` et `TROUS` vaut 4 — exactement ce que demande la règle 5 de l'énoncé, sans cas particulier à coder : la boucle de recherche sur une liste vide renvoie naturellement "aucun mur trouvé".

## Vérification sur les deux exemples

**Cas de base** (murs à la longueur du sol) :
fond -2000 2000 -2100 -2000
entree -2000 2000 2000 2100
gauche -2100 -2000 -2000 2000
droit 2000 2100 -2000 2000
FOND_GAUCHE TROU
FOND_DROIT TROU
ENTREE_GAUCHE TROU
ENTREE_DROIT TROU
TROUS 4

**Cas corrigé** (`fond` et `entree` allongés à `sx = 4200`) :
fond -2100 2100 -2100 -2000
entree -2100 2100 2000 2100
gauche -2100 -2000 -2000 2000
droit 2000 2100 -2000 2000
FOND_GAUCHE BOUCHE
FOND_DROIT BOUCHE
ENTREE_GAUCHE BOUCHE
ENTREE_DROIT BOUCHE
TROUS 0

Les deux sorties correspondent, caractère pour caractère, à celles attendues par l'énoncé.
