# Exercice 7 : La matrice de vue

**Nom :** Martial Aristide Barra
**Filière :** Génie Informatique, Spécialisation IA, ÉNSP

## Énoncé

Écrire les deux versions : celle qui inverse la matrice de la pose par une inversion générale, et celle qui construit directement le conjugué et la translation opposée. Comparer les seize coefficients. Puis passer une pose dégénérée à la première et regarder ce qu'elle rend.

## Les deux méthodes

**`InvertGeneral(Mat4)`** : inversion 4x4 générique par cofacteurs/adjugate. Reproduit volontairement le défaut dangereux décrit par le chapitre 2 : si le déterminant du bloc de rotation est proche de zéro, la fonction renvoie l'identité **silencieusement**, sans la moindre erreur.

**`InvertAnalytical(Pose)`** : construit directement l'inverse à partir du conjugué du quaternion et de la position opposée tournée par ce conjugué — la même formule que `Inverser` de l'exercice 4, assemblée ici sous forme de matrice.

## Comparaison des seize coefficients (cas normal)

Pose testée : position (3,-2,5), rotation 90° autour de Z — la même que l'exercice 4, pour vérifier la cohérence entre les deux exercices.

Matrice de la pose :

-0.0000 -1.0000 0.0000 3.0000
1.0000 -0.0000 0.0000 -2.0000
0.0000 0.0000 1.0000 5.0000
0.0000 0.0000 0.0000 1.0000


Matrice inverse (les deux méthodes, `InvertGeneral` et `InvertAnalytical`, s'accordent à 0,000001 près) :

-0.0000 1.0000 0.0000 2.0000
-1.0000 -0.0000 -0.0000 3.0000
-0.0000 0.0000 1.0000 -5.0000
0.0000 0.0000 0.0000 1.0000


La position retrouvée dans cette matrice, `(2, 3, -5)`, correspond exactement à la pose inverse déjà calculée à la main à l'exercice 4.

## Le cas dégénéré

Une pose valide, construite avec un quaternion réellement unitaire, ne peut jamais produire une matrice singulière — une rotation est toujours inversible. Le vrai test de dégénérescence ne porte donc pas sur un quaternion bizarre, mais sur une **matrice directement corrompue**, simulant un bug en amont (deux lignes identiques dans le bloc de rotation, déterminant nul) :

1 0 0 5
1 0 0 6
0 0 1 7
0 0 0 1


Passée à `InvertGeneral`, cette matrice donne un déterminant de `0.000000`, et la fonction renvoie l'identité **sans aucune erreur affichée** — exactement la démonstration recherchée : la méthode générale masque le problème au lieu de le signaler. C'est précisément ce que le chapitre 2 appelle un mensonge silencieux.

## Conclusion

`InvertAnalytical` est non seulement plus rapide (pas de calcul de déterminant ni d'adjugate), mais surtout plus honnête : elle ne peut pas produire de résultat faux sans le savoir, parce qu'elle ne dépend d'aucun garde-fou de singularité qui pourrait se déclencher silencieusement.
