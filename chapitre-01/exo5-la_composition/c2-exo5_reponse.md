# Exercice 5 : La composition

**Nom :** Martial Aristide Barra
**Filière :** Génie Informatique, Spécialisation IA, ÉNSP

## Énoncé

Écrire la composition de deux poses, puis vérifier sur un cas que composer puis appliquer donne le même résultat qu'appliquer l'une après l'autre. Afficher les deux points obtenus et leur écart.

## Rappel théorique

Composer une pose parent `P` avec une pose enfant `C` doit produire une pose équivalente à « appliquer `C`, puis appliquer `P` » — l'exemple du chapitre 2 : la main est dans le repère du coude, lui-même dans celui de l'épaule.

```cpp
Pose Composer(const Pose& parent, const Pose& child) {
    NkVec3f pos = ApplyPose(parent, child.position);
    NkQuatf rot = Multiply(parent.orientation, child.orientation);
    return Pose{ pos, rot };
}
```

`Multiply` combine deux rotations entre elles (différent de `RotateVectorByQuat`, qui fait tourner un vecteur par une seule rotation).

## Exemple vérifié

Parent : position (1,0,0), rotation 90° autour de Z.
Enfant : position (0,2,0), rotation 90° autour de Y.
Point : (1,1,1).

Entrée : `1 0 0   0 0 0.7071068 0.7071068   0 2 0   0 0.7071068 0 0.7071068   1 1 1`

- Point via composition (`ApplyPose(Composer(parent, child), point)`) : `-2.0000 / 1.0000 / -1.0000`
- Point via applications successives (`ApplyPose(parent, ApplyPose(child, point))`) : `-2.0000 / 1.0000 / -1.0000`
- Écart : `0.000000`

Les deux résultats sont identiques, confirmant que composer les poses avant d'appliquer donne bien le même résultat que les appliquer l'une après l'autre.

## Vérification interne

Ce même cas de test tourne automatiquement au démarrage du programme via `assert`, avant même de lire l'entrée utilisateur : si `Composer` casse un jour, le programme s'arrête clairement.

## Visualisation

Le programme affiche les axes du monde, le repère du parent, et le repère de l'enfant tel que placé dans le monde par la composition. Un seul repère composé est affiché plutôt que deux repères redondants : `Composer` ne fait rien d'autre que combiner directement position et orientation par les mêmes formules que la vérification, donc afficher un second repère "de la pose composée" séparément aurait été une duplication artificielle de la même donnée, pas une vraie comparaison. Une trajectoire animée en deux temps (d'abord via l'enfant, puis via le parent) est rejouable avec la touche Espace, pour visualiser le chemin composé.
