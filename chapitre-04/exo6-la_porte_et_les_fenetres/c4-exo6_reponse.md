# Exercice 4.6 — La porte et les fenêtres

## Objectif

Une porte ou une fenêtre n'est pas un trou découpé dans le mur : c'est un panneau fin plaqué devant, avec sa propre épaisseur et sa propre profondeur. Trop près du mur, les deux surfaces se disputent le même espace à l'écran (z-fighting, ici nommé « clignote ») ; trop loin, un vide visible apparaît derrière ; trop grand, le panneau dépasse du mur qui est censé le porter. Le programme vérifie ces quatre défauts possibles, dans un ordre fixe.

## Les grandeurs calculées

Pour un panneau de centre `(u, y)`, de taille `(l, h)`, d'épaisseur `e` et de profondeur de centre `d` :

- **Saillie** (face avant) : `d + e/2` — à quelle distance devant le mur le panneau avance.
- **Face arrière** : `d - e/2` — à quelle distance du mur (profondeur 0) la face arrière du panneau se trouve.

Le mur va de `-W/2` à `W/2` en largeur et de `0` à `H` en hauteur.

## L'ordre des verdicts, et pourquoi il est fixe

1. **`DEBORDE`** si le panneau sort du rectangle du mur sur n'importe quel bord : `u - l/2 < -W/2`, ou `u + l/2 > W/2`, ou `y - h/2 < 0`, ou `y + h/2 > H`. Ce test porte uniquement sur la géométrie dans le plan du mur — il ne regarde même pas la profondeur. C'est pourquoi `haute`, dans l'exemple, affiche `DEBORDE` malgré une saillie par ailleurs tout à fait correcte (40) : l'énoncé demande explicitement d'afficher la saillie même pour un panneau qui déborde, elle reste calculée et affichée, seul le verdict change.
2. **`INVISIBLE`** si la saillie est négative ou nulle — le panneau est entièrement noyé dans ou derrière le mur.
3. **`CLIGNOTE`** si la saillie est strictement inférieure au seuil — les deux surfaces sont trop proches pour que la carte graphique tranche de façon stable laquelle est devant.
4. **`DECOLLE`** si la face arrière dépasse strictement le seuil — un vide visible existe entre le panneau et le mur.
5. **`OK`** sinon.

Tester `DEBORDE` en premier a du sens : un panneau mal placé dans le plan du mur est un défaut plus grave et plus visible qu'un problème de profondeur, et les deux défauts peuvent coexister (rien n'empêche un panneau de déborder ET d'avoir une mauvaise saillie) — on ne garde que le premier qui s'applique.

## Les inégalités strictes, et pourquoi elles comptent

L'énoncé précise deux pièges symétriques :
- Un panneau exactement à la taille du mur ne déborde pas : les comparaisons de bord utilisent `<` et `>` (strictes), jamais `<=` ou `>=` — un bord qui coïncide exactement avec celui du mur (`u - l/2 == -W/2`) est acceptable.
- Une saillie égale au seuil ne clignote pas : `saillie < seuil` est strict. C'est le même principe que l'exercice du cube et la caméra : le seuil est la limite acceptable, pas déjà une faute.

## Déroulé sur l'exemple fourni

Mur de 4 m × 2,5 m, seuil de 5 mm.

- **porte** : tient dans le mur (vérifié sur les quatre bords). `d=20, e=40` → saillie `40`, face arrière `0`. Saillie bien au-dessus du seuil, face arrière à 0 (pas de vide) → `OK`.
- **fenetre** : même profondeur, tient dans le mur → `OK` également.
- **colle** : `e=2, d=0` → saillie `0+1=1`. Positive mais `1 < 5` → `CLIGNOTE`. La moitié du panneau est dans le mur et sa face avant n'avance que d'un millimètre : c'est justement le cas limite que le seuil est censé attraper.
- **haute** : `y=2200, h=1000` → `y+h/2 = 2700 > H(2500)` → `DEBORDE`, constaté avant même d'examiner la profondeur. La saillie (`40`) est quand même calculée et affichée.
- **flottante** : `d=300, e=20` → saillie `310` (loin au-dessus du seuil, donc pas `CLIGNOTE`), face arrière `300-10=290`, largement `> 5` → `DECOLLE` : un vide de near 29 cm se verrait derrière le panneau.

Deux panneaux `OK`, trois `A REPRENDRE`.

Sortie obtenue :
porte 40 OK
fenetre 40 OK
colle 1 CLIGNOTE
haute 40 DEBORDE
flottante 310 DECOLLE
OK 2
A REPRENDRE 3

Identique, caractère pour caractère, à celle exigée par l'énoncé.
