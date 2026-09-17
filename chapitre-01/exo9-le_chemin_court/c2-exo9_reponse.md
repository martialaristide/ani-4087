# Spécification — Exercice 9 : Le chemin court

## Contexte

Dépôt : `chapitre-01/exo9-le_chemin_court/`
Fichier de réponse attendu par la plateforme : `c2-exo9_reponse.md`

## Énoncé (rappel)

Écrire la vitesse angulaire moyenne entre deux orientations séparées de `dt`, avec le forçage du chemin court. Puis retirer le forçage et trouver deux quaternions pour lesquels le résultat devient absurde. Rendre les deux valeurs, avec et sans.

## Rappel théorique (chapitre 2)

Sur les quaternions, on suit toujours le chemin court : de 350° à 10°, on
peut passer par 0° en 20° (le chemin court), ou par 180° en 340° (le
chemin long). Une soustraction naïve donne le chemin long. La raison
profonde : **un quaternion `q` et son opposé `-q` représentent exactement
la même orientation** (double revêtement de SO(3)). Rien n'empêche deux
orientations très proches d'être représentées par des quaternions dont le
produit scalaire est négatif — dans ce cas, un calcul naïf de la rotation
entre elles emprunte le chemin long, autour de la sphère, au lieu du court.

## Code de base à réutiliser

`NkVec3f.hpp` (avec `Length`), `NkQuatf.hpp` (avec `Multiply`,
`Conjugate`) — mêmes fichiers que les exercices précédents. Ajouter `Dot`
et `Negate` pour les quaternions :

```cpp
inline nk_float32 Dot(const NkQuatf& a, const NkQuatf& b) {
    return a.x*b.x + a.y*b.y + a.z*b.z + a.w*b.w;
}
inline NkQuatf Negate(const NkQuatf& q) {
    return NkQuatf{ -q.x, -q.y, -q.z, -q.w };
}
```

## La fonction à écrire

```cpp
/**
 * Vitesse angulaire moyenne pour passer de q1 à q2 en dt secondes.
 * @param forceShortPath si vrai, force le chemin court quand le produit
 *        scalaire de q1 et q2 est negatif (elles pointent vers des
 *        "hemispheres" opposes du double revetement des quaternions,
 *        alors qu'elles peuvent representer des orientations tres proches).
 */
inline NkVec3f AverageAngularVelocity(const NkQuatf& q1, const NkQuatf& q2In,
                                        nk_float32 dt, bool forceShortPath) {
    NkQuatf q2 = q2In;
    if (forceShortPath && Dot(q1, q2) < 0.0f) {
        q2 = Negate(q2); // meme orientation, chemin court garanti
    }
    NkQuatf delta = Multiply(q2, Conjugate(q1)); // rotation de q1 vers q2
    nk_float32 w = std::fmax(-1.0f, std::fmin(1.0f, delta.w)); // clamp, securite acos
    nk_float32 angle = 2.0f * std::acos(w);
    nk_float32 s = std::sqrt(1.0f - w*w);
    NkVec3f axis = (s < 1e-6f) ? NkVec3f{0,0,0} : NkVec3f{ delta.x/s, delta.y/s, delta.z/s };
    return (angle / dt) * axis;
}
```

## L'exemple qui devient absurde sans le forçage (déjà vérifié)

Deux quaternions représentant des orientations **très proches en
réalité** (seulement 10° d'écart), mais dont l'un a été négativement
"replié" sur l'autre hémisphère du double revêtement :

```
q1 = identité = (0, 0, 0, 1)
q2 = -(rotation de 10° autour de Z) = (-0.0000, -0.0000, -0.0872, -0.9962)
```

Produit scalaire `q1 · q2 = -0.9962` (négatif — signal du problème).

**Sans forçage** : vitesse angulaire `(0.0000, 0.0000, -6.1087)` rad/s
(norme ≈ 6,11 rad/s, soit environ 350°/s).

**Avec forçage** : vitesse angulaire `(0.0000, 0.0000, 0.1745)` rad/s
(norme ≈ 0,17 rad/s, soit 10°/s).

**Pourquoi c'est absurde sans le forçage** : la vraie rotation physique
entre `q1` et `q2` (qui représentent quasiment la même orientation, à 10°
près) n'est que de 10°. Le calcul sans forçage prétend qu'on a tourné à
environ 350°/s — trente-cinq fois plus vite que la réalité — simplement
parce que les deux quaternions ont été pris sur des hémisphères opposés du
double revêtement, un artefact de représentation, pas un vrai mouvement.

## Format d'entrée/sortie

- **Entrée** : deux quaternions et un `dt` : 4 + 4 + 1 = 9 réels (q1 xyzw,
  q2 xyzw, dt).
- **Sortie console** : deux blocs de 3 lignes, 4 décimales, sans texte
  mélangé — d'abord la vitesse angulaire SANS forçage, puis AVEC forçage
  (6 lignes au total).

## Exigence de qualité

- Documentation Doxygen expliquant le double revêtement des quaternions et
  pourquoi le forçage du chemin court est nécessaire (déjà en partie
  rédigée ci-dessus, à reprendre et enrichir).
- Vérification interne par `assert` avec l'exemple absurde ci-dessus :
  vérifier que la norme SANS forçage dépasse largement (`> 3.0` rad/s, un
  seuil largement au-dessus de ce qu'une vraie rotation de 10° donnerait)
  et que la norme AVEC forçage est proche de la valeur correcte (`≈ 0.1745`,
  tolérance `1e-2`).
- Un Makefile minimal.
- Validation d'entrée stricte.

## Visualisation

- GLFW + OpenGL, isométrique, cohérent avec les exercices précédents.
- Afficher les deux orientations `q1` et `q2` comme deux repères proches
  dans l'espace (puisqu'elles représentent presque la même orientation).
- Afficher, sous forme de deux arcs distincts partant du même point, le
  chemin court (petit arc, correspondant au calcul avec forçage) et le
  chemin long (grand arc, quasiment un tour complet, correspondant au
  calcul sans forçage) — pour rendre visible à quel point l'écart entre
  les deux résultats est disproportionné par rapport à la vraie différence
  d'orientation.
- Toujours pas de bibliothèque de police.


