# Exercice 8 : L'extrapolation

**Nom :** Martial Aristide Barra
**Filière :** Génie Informatique, Spécialisation IA, ÉNSP

## Énoncé

Écrire la fonction qui avance une pose de `dt` secondes à vitesses constantes, linéaire et angulaire. Le programme lit une pose, ses deux vitesses et une durée, et affiche la pose extrapolée. Traiter le cas d'une vitesse angulaire nulle sans diviser par zéro.

## Les deux fonctions

**Position**, triviale à vitesse constante :
```cpp
NkVec3f ExtrapolatePosition(const NkVec3f& pos, const NkVec3f& linVel, nk_float32 dt) {
    return pos + dt * linVel;
}
```

**Orientation**, le cœur de l'exercice : la vitesse angulaire est un vecteur dont la direction est l'axe de rotation et la norme la vitesse en rad/s.

```cpp
NkQuatf ExtrapolateOrientation(const NkQuatf& q, const NkVec3f& angVel, nk_float32 dt) {
    nk_float32 speed = Length(angVel);
    if (speed < 1e-6f) {
        return q; // vitesse angulaire nulle : evite la division par zero
    }
    NkVec3f axis = (1.0f / speed) * angVel;
    nk_float32 angle = speed * dt;
    nk_float32 half = angle * 0.5f;
    nk_float32 s = std::sin(half);
    NkQuatf delta{ axis.x * s, axis.y * s, axis.z * s, std::cos(half) };
    return Multiply(delta, q);
}
```

## Le cas de la vitesse angulaire nulle

Si la norme de la vitesse angulaire est nulle, tenter de la normaliser diviserait par zéro et produirait un axe fait de `NaN`. La fonction vérifie donc la norme en premier, et retourne l'orientation inchangée si elle est proche de zéro, sans jamais atteindre l'étape de normalisation.

## Convention retenue (non précisée par l'énoncé)

La vitesse angulaire est traitée comme exprimée dans le repère du monde : le quaternion delta est multiplié à gauche de l'orientation actuelle (`Multiply(delta, q)`, pas `Multiply(q, delta)`).

## Exemples vérifiés

**Rotation autour de Z à π rad/s pendant 1 seconde** :
Entrée : `1 2 3   0 0 0 1   0.5 -1 2   0 0 3.14159265 1`
Sortie : position `1.5000 / 1.0000 / 5.0000`, quaternion `0.0000 / 0.0000 / 1.0000 / -0.0000` (rotation de 180°, cohérent avec π rad/s pendant 1 s).

**Vitesse angulaire nulle (le cas protégé)** :
Entrée : `0 0 0   0.1 0.2 0.3 0.9273   1 1 1   0 0 0   2`
Sortie : position `2.0000 / 2.0000 / 2.0000`, quaternion **inchangé** `0.1000 / 0.2000 / 0.3000 / 0.9273` — aucun crash, aucun `NaN` (vérifié explicitement avec `std::isnan` sur chaque composante).

## Visualisation

Le programme affiche un repère à la pose de départ et un repère à la pose extrapolée, reliés par une trajectoire animée. Plutôt qu'une simple interpolation linéaire entre les deux poses, l'animation évalue la vraie fonction d'extrapolation à `u*dt` pour chaque instant `u` entre 0 et 1, ce qui donne une trajectoire courbe fidèle à la rotation réelle plutôt qu'un raccourci visuel trompeur.
