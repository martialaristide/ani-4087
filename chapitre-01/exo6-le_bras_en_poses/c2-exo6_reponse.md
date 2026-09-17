# Exercice 6 : Le bras en poses

**Nom :** Martial Aristide Barra
**Filière :** Génie Informatique, Spécialisation IA, ÉNSP

## Énoncé

Construire un bras articulé à trois segments : une épaule à l'origine, un coude à un bras de distance, une main à un avant-bras du coude. Chaque articulation porte sa pose dans le repère de la précédente, et la pose d'une articulation dans le monde s'obtient en composant celle de son parent avec la sienne. Afficher la position du coude et celle de la main dans l'espace du monde. Faire tourner l'épaule et vérifier que la main suit.

## Le modèle

Épaule à l'origine du monde (orientation variable). Coude en pose locale dans le repère de l'épaule, à `(0.35, 0, 0)` (35 cm, longueur du bras). Main en pose locale dans le repère du coude, à `(0.30, 0, 0)` (30 cm, longueur de l'avant-bras). Le modèle (`ComputeArm`, dans `ArmModel.hpp`) est partagé entre le programme interactif et la suite de tests, pour garantir qu'ils testent bien le même code.

```cpp
Pose shoulderWorld = Pose{ {0,0,0}, shoulderOrientation };
Pose elbowLocal    = Pose{ {ARM_LENGTH, 0, 0}, elbowOrientation };
Pose handLocal     = Pose{ {FOREARM_LENGTH, 0, 0}, handOrientation };

Pose elbowWorld = Composer(shoulderWorld, elbowLocal);
Pose handWorld  = Composer(elbowWorld, handLocal);
```

## Exemples vérifiés

**Sans rotation** : Coude `(0.3500, 0.0000, 0.0000)`, Main `(0.6500, 0.0000, 0.0000)`.

**Épaule tournée de 90° autour de Z** : Coude `(-0.0000, 0.3500, 0.0000)`, Main `(-0.0000, 0.6500, 0.0000)`. Le bras entier a pivoté autour de l'épaule — la main a bien suivi, confirmant la composition en chaîne. (Le `-0.0000` est un artefact normal de l'arrondi flottant sur une valeur théoriquement nulle.)

## Vérification indépendante

`ReferenceKinematics.hpp` recalcule les mêmes positions par une méthode entièrement indépendante — matrices de rotation 3×3 construites explicitement et multipliées à la main, sans passer ni par les quaternions, ni par `Multiply`, ni par `Composer`. Les deux méthodes s'accordent, une preuve plus solide qu'un auto-test, puisqu'un bug systémique dans les quaternions n'aurait pas pu se cacher des deux côtés à la fois.

## Suite de tests séparée

`tests.cpp`, compilé indépendamment du programme interactif via `make test` (sans dépendance à GLFW), fait passer 5 tests : les deux exemples ci-dessus, une rotation simultanée sur les trois articulations comparée à la méthode matricielle, l'**associativité de la composition** (`Composer(Composer(A,B),C) ≈ Composer(A,Composer(B,C))`), et l'**inégalité triangulaire** (30 tirages à graine fixe : la distance épaule-main ne dépasse jamais 0,65 m). Résultat : tous les tests passent.

## Visualisation

Le bras est dessiné comme une chaîne de segments (épaule-coude, coude-main), avec un point coloré à chaque articulation. L'épaule (flèches gauche/droite) et le coude (touches A/E) sont tous les deux interactifs en continu, permettant d'articuler le bras entièrement en temps réel. Un cercle de portée maximale (rayon 0,65 m) est dessiné autour de l'épaule : la main reste toujours à l'intérieur ou sur ce cercle, quelle que soit la manipulation — la version visuelle et vérifiable à l'œil du test d'inégalité triangulaire.
