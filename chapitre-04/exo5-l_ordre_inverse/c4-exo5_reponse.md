# Exercice 4.5 — L'ordre inverse

## Objectif

Les matrices ne commutent pas : `Translation × Echelle` et `Echelle × Translation` ne donnent pas le même résultat. L'erreur d'ordre ne produit aucun message — seulement des objets qui se retrouvent déplacés, parfois légèrement, parfois très loin. Ce programme calcule où un objet atterrit avec le mauvais ordre, pour reconnaître la signature de cette erreur.

## Bon ordre vs mauvais ordre

Le bon ordre (translation puis échelle appliquée au repère local de l'objet) place le centre de l'objet exactement à `(tx, ty, tz)` : c'est la translation seule qui décide de la position.

Le mauvais ordre applique l'échelle à la translation elle-même, axe par axe :
mauvaisX = sx * tx / 1000
mauvaisY = sy * ty / 1000
mauvaisZ = sz * tz / 1000
Chaque composante de l'échelle ne touche que la composante correspondante de la translation — jamais une autre. C'est le piège explicite de l'énoncé : `sx` ne multiplie jamais `ty` ou `tz`.

## Pourquoi multiplier avant de diviser

`sx * tx / 1000` et `sx * (tx / 1000)` ne donnent pas le même résultat en division entière. Par exemple pour `porte` en z : `sz = 40`, `tz = -1980`. Multiplier d'abord donne `40 * (-1980) = -79200`, puis `/1000 = -79`. Diviser d'abord aurait donné `tz / 1000 = -1` (puisque -1980/1000 tronqué vers zéro vaut -1), puis `40 * (-1) = -40` — un résultat complètement différent et faux. L'ordre des opérations n'est pas qu'une question de style, il change la réponse.

## La troncature vers zéro

En C++, la division entière de deux entiers de signes différents tronque vers zéro, pas vers le bas : `-79200 / 1000` vaut `-79`, et non `-80` comme le donnerait une division "mathématique" suivie d'un `floor`. L'énoncé insiste là-dessus car c'est le comportement par défaut du langage (depuis C++11, la troncature vers zéro est garantie par la norme) — il n'y a donc rien à coder spécialement pour l'obtenir, juste à ne pas la contrarier avec un arrondi manuel.

## L'écart et le bilan

L'écart sur un axe est `|bonne_position - mauvaise_position|`, et l'écart retenu pour un objet est le plus grand des trois écarts par axe — parce qu'un seul axe mal placé suffit à rendre l'erreur visible, peu importe que les deux autres soient restés corrects.

`DEPLACES` compte les objets dont l'écart n'est pas nul ; `PIRE` est le plus grand écart parmi tous les objets, initialisé à 0 pour le cas sans objet.

## Pourquoi un cube d'échelle 1000 ne révèle rien

Avec une échelle de 1000 millièmes (soit un facteur 1, donc "ne change rien", comme le dit l'énoncé), le mauvais ordre donne `1000 * tx / 1000 = tx`, identique à la bonne position. C'est exactement le cas de `repere` dans l'exemple : écart nul sur les trois axes. C'est pour cette raison précise que l'erreur d'ordre des matrices peut passer inaperçue pendant longtemps dans un moteur — elle ne se voit que sur les objets dont l'échelle s'écarte de 1 ET dont la translation n'est pas nulle sur cet axe.

## Déroulé sur l'exemple fourni

- **fond** : `sx=4000, sy=2500, sz=100`, `t=(0, 1250, -2000)`. `mauvaisX = 4000*0/1000 = 0`, `mauvaisY = 2500*1250/1000 = 3125`, `mauvaisZ = 100*(-2000)/1000 = -200`. Écarts : 0, 1875, 1800 → retenu 1875.
- **gauche** : le même mur tourné, `sx=100, sy=2500, sz=4000`, `t=(-2000,1250,0)`. `mauvaisX=100*(-2000)/1000=-200`, `mauvaisY=3125`, `mauvaisZ=4000*0/1000=0`. Écarts : 1800, 1875, 0 → retenu 1875.
- **repere** : échelle 1000 partout → écart nul sur les trois axes → 0.
- **porte** : `sz=40, tz=-1980` → `mauvaisZ = -79200/1000 = -79` (troncature vers zéro, pas -80). Écarts : 70, 1000, 1901 → retenu 1901, le pire de tous.

Trois objets déplacés (`fond`, `gauche`, `porte`), le pire écart étant celui de `porte`.

Sortie obtenue :
fond 0 3125 -200 1875
gauche -200 3125 0 1875
repere -1100 500 1200 0
porte -630 2000 -79 1901
DEPLACES 3
PIRE 1901

Identique, caractère pour caractère, à celle exigée par l'énoncé.
