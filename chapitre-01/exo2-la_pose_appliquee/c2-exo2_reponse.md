# Exercice 2 : La pose appliquée

**Nom :** Martial Aristide Barra
**Filière :** Génie Informatique, Spécialisation IA, ÉNSP

## Énoncé

Écrire la structure `Pose`, avec une position et un quaternion, et la fonction qui applique une pose à un point : rotation puis translation. Le quaternion est donné par ses quatre composantes, déjà normalisé. Le programme lit une pose et un point, et affiche le point transformé.

## Convention retenue

L'énoncé ne précisait ni l'ordre de lecture des valeurs ni l'ordre des composantes du quaternion. Convention choisie, cohérente avec OpenXR (déjà utilisé partout ailleurs dans ce cours) :

- Lecture : position (x y z), puis quaternion (x y z w), puis point (x y z) — 10 réels au total.
- Sortie : le point transformé, 3 lignes, 4 décimales.

## La structure et la fonction

```cpp
struct Pose {
    NkVec3f position;
    NkQuatf orientation;
};

NkVec3f ApplyPose(const Pose& pose, const NkVec3f& point) {
    NkVec3f rotated = RotateVectorByQuat(pose.orientation, point);
    return rotated + pose.position;
}
```

La rotation utilise une formule vectorielle équivalente à `q * v * conjugué(q)`, sans passer par une matrice intermédiaire. L'ordre rotation-puis-translation est fixé et non négociable : l'inverser ferait tourner le point autour de l'origine du monde au lieu de le faire tourner sur lui-même avant de le déplacer.

## Exemples vérifiés

**Rotation 90° autour de Z, avec translation :**
Entrée : `1 2 3   0 0 0.7071068 0.7071068   1 0 0`
Sortie : `1.0000 / 3.0000 / 3.0000`
Le point (1,0,0) tourne en (0,1,0), puis se translate de (1,2,3), donnant (1,3,3).

**Rotation identité, avec translation seule :**
Entrée : `1 2 3   0 0 0 1   1 0 0`
Sortie : `2.0000 / 2.0000 / 3.0000`
Sans rotation, le point (1,0,0) reste inchangé puis se translate de (1,2,3), donnant (2,2,3).

**Entrée invalide** (moins de 10 valeurs) : message d'usage clair sur `stderr`, code de sortie 1, plutôt qu'un échec silencieux.

## Visualisation

En plus du calcul, le programme ouvre une fenêtre montrant : les axes du monde à l'origine (X rouge, Y vert, Z bleu) en projection isométrique, le repère local de la pose (axes tournés par le quaternion) à sa position, et deux points reliés à la pose par des segments fins — un atténué montrant la position sans rotation (translation seule), et le point final réel (magenta) après rotation puis translation. Une animation de 1,5 seconde (rejouable avec Espace) montre le mouvement en deux temps : rotation autour de l'origine locale, puis translation vers la position finale, avec un marqueur jaune distinct pendant l'animation. Cette visualisation rend concret pourquoi l'ordre des deux opérations n'est pas interchangeable.
