# Exercice 3 : L'ordre inverse

**Nom :** Martial Aristide Barra
**Filière :** Génie Informatique, Spécialisation IA, ÉNSP

## Énoncé

Reprendre l'exercice précédent et écrire une seconde fonction qui applique la translation d'abord et la rotation ensuite. Afficher les deux résultats pour le même point. Puis trouver une pose et un point pour lesquels les deux coïncident, et dire pourquoi.

## La nouvelle fonction

```cpp
NkVec3f ApplyPoseTranslationFirst(const Pose& pose, const NkVec3f& point) {
    return RotateVectorByQuat(pose.orientation, point + pose.position);
}
```

Contrairement à `ApplyPose` (exercice 2), ici on additionne d'abord la position au point, puis on fait tourner le résultat obtenu — l'ordre inverse.

## Cas général : les deux résultats diffèrent

Entrée : `1 2 3   0 0 0.7071068 0.7071068   1 0 0` (position (1,2,3), rotation 90° autour de Z, point (1,0,0))

- `ApplyPose` (rotation puis translation) : `1.0000 / 3.0000 / 3.0000`
- `ApplyPoseTranslationFirst` (translation puis rotation) : `-2.0000 / 2.0000 / 3.0000`

Les deux résultats sont différents, ce qui confirme que l'ordre des opérations compte en général.

## Cas de coïncidence

Entrée : `0 0 5   0 0 0.7071068 0.7071068   1 2 3` (position (0,0,5), rotation 90° autour de Z, point (1,2,3))

- `ApplyPose` : `-2.0000 / 1.0000 / 8.0000`
- `ApplyPoseTranslationFirst` : `-2.0000 / 1.0000 / 8.0000`

Les deux résultats sont **identiques**.

## Pourquoi ça coïncide

Une rotation autour de l'axe Z ne change jamais la composante Z d'un vecteur : elle ne fait tourner que le plan (X, Y). Ici, la translation choisie, `(0, 0, 5)`, est purement le long de Z — donc cette translation est laissée inchangée par la rotation (elle est invariante par elle, car alignée avec l'axe de rotation).

Formellement : soit R la rotation et t la translation. Les deux ordres donnent :
- rotation puis translation : `R(p) + t`
- translation puis rotation : `R(p + t) = R(p) + R(t)`

Ces deux expressions sont égales exactement quand `R(t) = t`, c'est-à-dire quand la translation est un point fixe de la rotation — ce qui arrive précisément quand elle est alignée avec l'axe de rotation. C'est le cas ici, d'où la coïncidence, quel que soit le point choisi.

## Visualisation

Le programme affiche deux chemins depuis le point d'origine : un chemin cyan (rotation puis translation) et un chemin orange (translation puis rotation), avec leurs points d'arrivée marqués. Dans le cas général, les deux chemins et leurs points d'arrivée sont visiblement distincts. Dans le cas de coïncidence (translation le long de l'axe de rotation), les deux chemins et marqueurs se superposent exactement — confirmation visuelle du raisonnement mathématique ci-dessus.
