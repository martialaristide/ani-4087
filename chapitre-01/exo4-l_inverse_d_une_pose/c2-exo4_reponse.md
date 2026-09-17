# Exercice 4 : L'inverse d'une pose

**Nom :** Martial Aristide Barra
**Filière :** Génie Informatique, Spécialisation IA, ÉNSP

## Énoncé

Écrire `Inverser(pose)` à la main : conjugué du quaternion, et position opposée tournée par ce conjugué. Vérifier : appliquer la pose à un point, puis l'inverse au résultat, et afficher l'écart au point de départ. Il doit être nul aux arrondis près.

## La fonction

```cpp
Pose Inverser(const Pose& pose) {
    NkQuatf qInv = Conjugate(pose.orientation);
    NkVec3f pInv = RotateVectorByQuat(qInv, -1.0f * pose.position);
    return Pose{ pInv, qInv };
}
```

Pour une pose telle que `ApplyPose(pose, p) = R(p) + t`, on veut que `ApplyPose(Inverser(pose), ApplyPose(pose, p))` redonne `p` pour tout point `p`. Cela impose deux choses : la rotation inverse est le conjugué du quaternion (car pour un quaternion unitaire, conjugué = inverse exact), et la position inverse est `-R⁻¹(t)` — la position opposée, tournée par ce même conjugué.

## Pourquoi à la main, pas via une inversion générale de matrice

Une inversion générique de matrice, face à un cas dégénéré issu d'un bug en amont, peut renvoyer silencieusement l'identité plutôt que d'échouer clairement
