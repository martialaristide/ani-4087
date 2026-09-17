# Exercice : Les trois directions

**Nom :** Martial Aristide Barra
**Filière :** Génie Informatique, Spécialisation IA, ÉNSP

## Énoncé

Écrire les trois fonctions qui fixent la convention une fois pour toutes : `Avant()`, `Haut()` et `Droite()`, qui rendent chacune un vecteur unitaire dans la convention du module. Le programme lit trois réels, les traite comme un point, et affiche son produit scalaire avec chacune des trois directions. Trois lignes, quatre décimales.

## Les trois fonctions

- `Avant()` retourne `(0, 0, -1)` — convention −Z, comme OpenXR.
- `Haut()` retourne `(0, 1, 0)` — axe +Y.
- `Droite()` retourne `(1, 0, 0)` — axe +X.

Ce sont des vecteurs unitaires, définis une fois pour toutes dans `Directions.cpp` et documentés (Doxygen) dans `Directions.hpp`, plutôt que codés en dur à chaque endroit du programme qui en a besoin.

## Structure du code

- `NkVec3f.hpp` : le type vecteur (`NkVec3f`, champs en `nk_float32`) et l'opération `Dot`.
- `Directions.hpp` / `Directions.cpp` : les trois fonctions de convention.
- `main.cpp` : lecture de l'entrée, vérification interne, calcul, affichage.
- `Makefile` : compilation via `make`.

## Vérification interne

En tout début de `main()`, un `assert` vérifie que les trois vecteurs de base sont bien unitaires et mutuellement orthogonaux. C'est une preuve dans le code lui-même que la convention posée forme une base orthonormée valide, pas seulement que le programme compile et tourne.

## Validation d'entrée

Si la lecture des trois réels échoue (par exemple si seulement deux valeurs sont fournies), le programme affiche un message d'usage clair sur `stderr` et retourne un code de sortie non nul, plutôt que d'échouer silencieusement.

## Exemple d'exécution

Entrée :

1.0 2.0 3.0


Sortie :

-3.0000
2.0000
1.0000


Ce qui correspond bien à `-z`, `y`, `x` du point donné (Avant = −Z, Haut = +Y, Droite = +X), puisque le produit scalaire avec un vecteur de base redonne directement une seule coordonnée du point.

Test de robustesse : une entrée incomplète (`1.0 2.0`) déclenche bien le message d'usage sur `stderr` et un code de sortie 1, plutôt qu'un plantage ou un résultat silencieusement faux.

## Compilation

make
./exo1
