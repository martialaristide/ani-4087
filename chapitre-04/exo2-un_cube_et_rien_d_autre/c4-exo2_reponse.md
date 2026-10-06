# Exercice 4.2 — Un cube et rien d'autre

## Objectif

Diagnostiquer, dans un ordre fixe de causes, pourquoi un cube posé devant la caméra ne s'affiche pas. Un écran vide ne donne aucun message d'erreur ; la seule manière de déboguer vite est de tester les causes possibles une par une, dans un ordre précis, et de garder la première qui s'applique.

## Les cinq causes testées, dans l'ordre

1. **`RENDER3D ETEINT`** — le bit 2 de `drapeaux` (`NK_SS_RENDER3D`) est à zéro. Sans ce sous-système, rien n'est dessiné, quelle que soit la géométrie ou l'éclairage.
2. **`ECHELLE NULLE`** — une des trois dimensions du cube (`sx`, `sy`, `sz`) est nulle. Un cube sans volume dans une direction n'a pas de surface à peindre.
3. **`CAMERA DANS LE CUBE`** — la face avant du cube, à `distance - sz/2` de la caméra, est négative ou nulle. On ne voit jamais les faces d'un cube depuis l'intérieur ; une distance exactement nulle compte aussi comme "dans le cube", d'après l'énoncé.
4. **`COUPE PAR LE PLAN PROCHE`** — la face avant est positive (donc hors du cube), mais plus proche que le plan rapproché de la caméra. La carte graphique découpe tout ce qui est en avant de ce plan, même si l'objet existe géométriquement.
5. **`PAS DE LUMIERE`** — ni lumière directe ni lumière ambiante. Le cube est géométriquement visible, mais rien ne l'éclaire. Une ambiante seule, même faible, suffit à éviter ce verdict.

Si aucune de ces cinq causes ne s'applique : `VISIBLE`.

## Pourquoi l'ordre compte

Un cube peut avoir plusieurs défauts à la fois (par exemple, le rendu 3D éteint ET aucune lumière). L'énoncé impose de ne garder que le premier défaut rencontré dans l'ordre fixe, pas une liste de tous les défauts. Le code teste donc les causes avec une cascade de `if / else if`, et s'arrête au premier test vrai.

## Le piège du type de `drapeaux`

`NK_SS_ALL` vaut `4294967295`, soit 2³²−1. C'est plus grand que ce qu'un `int` signé de 32 bits peut représenter (qui plafonne à 2 147 483 647) : lire cette valeur dans un `int` provoquerait un débordement — un comportement indéfini en C++, pas une erreur visible à l'exécution. Le type utilisé est donc `unsigned long long` (64 bits), à la fois assez grand et sémantiquement correct puisque `drapeaux` est un masque de bits, pas une quantité signée.

Autre piège, purement syntaxique cette fois : en C++, l'opérateur `&` (bit à bit) a une priorité **plus faible** que `==`. Écrire `drapeaux & 2 == 0` serait interprété comme
5
blanc 18 1000 1000 1000 2000 1 150 50
ombre 16 1000 1000 1000 2000 1 150 50
plat 18 1000 0 1000 2000 1 150 50
mur 18 4000 2500 4000 1500 1 150 50
noir 18 1000 1000 1000 2000 0 0 50

Sortie obtenue :
blanc VISIBLE
ombre RENDER3D ETEINT
plat ECHELLE NULLE
mur CAMERA DANS LE CUBE
noir PAS DE LUMIERE
VISIBLES 1
EN PANNE 4

Vérification du détail :
- **blanc** : drapeaux 18 = 16+2, bit RENDER3D allumé. Aucune taille nulle. Face avant = 2000 − 1000/2 = 1500, positive et au-delà du plan rapproché (50). Une lumière. → VISIBLE.
- **ombre** : drapeaux 16, bit RENDER3D éteint (16 & 2 = 0). → RENDER3D ETEINT, peu importe le reste.
- **plat** : sy = 0. → ECHELLE NULLE.
- **mur** : sz = 4000, distance = 1500. Face avant = 1500 − 4000/2 = 1500 − 2000 = −500, négative. → CAMERA DANS LE CUBE.
- **noir** : lumieres = 0 et ambiante = 0, et toutes les étapes précédentes sont passées sans déclencher de cause. → PAS DE LUMIERE.

Sortie identique, caractère pour caractère, à celle exigée par l'énoncé.
