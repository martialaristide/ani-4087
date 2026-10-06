# Exercice 4.7 — Les drapeaux qu'on nomme

## Objectif

Un moteur de rendu allume ses sous-systèmes par un masque de bits : chaque drapeau simple est une puissance de 2, et on les combine par OU binaire pour obtenir une configuration. Nommer explicitement ses drapeaux plutôt que de tout allumer par défaut (`ALL`) permet de savoir précisément ce qu'on utilise — et de repérer tout de suite une faute de frappe, qui n'empêche jamais le programme de continuer, mais désactive silencieusement un sous-système.

## OU binaire, pas addition

La valeur part de 0 et chaque nom reconnu s'y combine par `|=`, jamais par `+=`. C'est essentiel pour deux raisons que l'énoncé pointe explicitement :
- Un nom répété ne change rien avec `|=` (`x | x == x`), alors qu'une addition le compterait deux fois.
- Un drapeau déjà contenu dans un nom composé ne s'additionne pas non plus : si `3D_BASE` (qui contient déjà `SHADOW`) et `SHADOW` apparaissent tous les deux, l'addition ferait doublement compter le bit `16`, alors que le OU le laisse simplement allumé une fois.

## Le type de la valeur

`ALL` vaut `4294967295`, soit `2^32 - 1` : tous les bits d'un entier non signé de 32 bits. Cette valeur dépasse ce qu'un `int` signé de 32 bits peut représenter (plafond à `2147483647`) — la stocker dans un `int` serait un débordement, un comportement indéfini en C++. Le programme utilise donc `std::uint32_t` partout où cette valeur circule : c'est aussi sémantiquement correct, puisqu'un masque de bits n'a pas de signe.

## La table des noms

Treize drapeaux simples (puissances de 2 de 1 à 4096), plus cinq noms composés obtenus eux aussi par OU binaire : `NONE = 0`, `2D_ESSENTIALS = RENDER2D | TEXT`, `3D_BASE = RENDER3D | SHADOW | POST_PROCESS`, `DEBUG = OVERLAY | SIMULATION`, `ALL = 4294967295`. Tous les dix-huit noms vivent dans une seule table de correspondance (nom → valeur), ce qui évite une cascade de dix-huit `if` à chaque lecture.

`NONE` est un piège explicite de l'énoncé : c'est un nom *connu* qui vaut 0. Il ne doit donc jamais déclencher une ligne `INCONNU`, même s'il n'ajoute littéralement rien à la valeur (`OR` avec 0 est neutre).

## Le cas N = 0

Si aucun drapeau n'est nommé, la configuration garde sa valeur par défaut, `ALL` — tout est allumé, par prudence, faute d'information. C'est l'inverse d'un oubli silencieux : ne rien dire, c'est tout activer, jamais rien désactiver. Le programme initialise `valeur` à `ALL` uniquement quand `N == 0`, et à `0` sinon — ce choix est fait une seule fois, avant la boucle de lecture, puis jamais modifié par un cas particulier dans la boucle.

## Les dépendances, et leur ordre strict

Quatre drapeaux simples réclament d'autres drapeaux pour avoir un sens : `TEXT` a besoin de `RENDER2D`, `UI` a besoin de `RENDER2D` et `TEXT`, `SHADOW` a besoin de `RENDER3D`, `OVERLAY` a besoin de `RENDER2D` et `TEXT`. Un drapeau **éteint** ne réclame rien — c'est pour cela que chaque vérification commence par tester si le drapeau lui-même est allumé avant de regarder ses dépendances. L'ordre d'affichage est fixe (`TEXT`, `UI`, `SHADOW`, `OVERLAY`, chacun avec ses dépendances dans l'ordre donné), donc le programme parcourt une liste ordonnée de ces quatre règles plutôt qu'une structure non ordonnée comme une table de hachage.

## Déroulé sur l'exemple fourni

Entrée : `RENDER3D`, `SHADOW`, `TEXT`, `RENDU3D`.

- `RENDU3D` n'est dans aucune table : `INCONNU RENDU3D`. Le programme continue — c'est exactement le risque que l'exercice met en scène : une faute de frappe n'arrête rien, elle éteint juste un sous-système sans prévenir.
- Valeur : `RENDER3D(2) | SHADOW(16) | TEXT(4) = 22`, soit `0x00000016` en hexadécimal (8 chiffres, majuscules).
- Dépendances : `TEXT` est allumé mais `RENDER2D` ne l'est pas → `MANQUE TEXT RENDER2D`. `UI` est éteint → rien à dire. `SHADOW` est allumé et sa dépendance `RENDER3D` est bien présente → rien à dire. `OVERLAY` est éteint → rien à dire.
- Sur les treize drapeaux simples, trois sont allumés (`RENDER3D`, `TEXT`, `SHADOW`) → `ALLUMES 3`, `ETEINTS 10`.

Sortie obtenue :
INCONNU RENDU3D
VALEUR 22
HEXA 0x00000016
MANQUE TEXT RENDER2D
ALLUMES 3
ETEINTS 10

Et pour vérifier le cas par défaut, avec `N = 0` :
VALEUR 4294967295
HEXA 0xFFFFFFFF
ALLUMES 13
ETEINTS 0
Aucune ligne `MANQUE` ici, puisque tous les bits — donc toutes les dépendances — sont déjà satisfaits.

Les deux sorties correspondent, caractère pour caractère, à celles attendues.
