
# Demo 8 — Deux machines, un rapport

## La boucle chronométrée

Même code exact sur les deux machines : une boucle scalaire (variable
`volatile` pour empêcher le compilateur de l'optimiser ou de la vectoriser),
411 300 000 itérations, chronométrée avec `std::chrono::high_resolution_clock`.

```cpp
const long long N = 411300000LL;
volatile double resultat = 0.0;
auto debut = std::chrono::high_resolution_clock::now();

for (long long i = 0; i < N; ++i) {
    resultat += (i * 1.0000001) - (i * 0.9999999);
}

auto fin = std::chrono::high_resolution_clock::now();
double ms = std::chrono::duration<double, std::milli>(fin - debut).count();
```

Sur PC : `test_benchmark_pc.cpp`, compilé en optimisé (`g++ -O2`).
Sur l'appareil : la même boucle, intégrée dans `android_main()` de l'exo16,
compilée en configuration Release (optimisations activées), mesurée via
`__android_log_print` et lue dans le logcat.

## Rapport

| | PC | Téléphone |
|---|---|---|
| Machine | Intel Core i7-7700HQ @ 2.80 GHz | Huawei MLA-L11 (chipset msm8953, Snapdragon 625) |
| Build | g++ -O2, natif Windows | Release Android, arm64-v8a, NDK |
| Itérations | 411 300 000 | 411 300 000 |
| Temps mesuré | 2414,090 ms (2,414 s) | 3481,446 ms (3,481 s) |
| Débit | ~170,4 M itérations/s | ~118,2 M itérations/s |

Sortie brute PC (./test_benchmark_pc) :

Iterations: 411300000
Temps: 2414.090 ms (2.414 s)
Resultat (ignorer): 16916768958.498348


Sortie brute téléphone (logcat, tag ExoLePaquetVide) :

09-24 09:43:08.165 I ExoLePaquetVide: Benchmark: 411300000 iterations en 3481.446 ms


Constat : sur ce test précis (calcul flottant scalaire, un seul coeur
sollicité), le téléphone met environ 1,44x plus de temps que le PC pour
faire exactement le même travail. Le PC est un processeur de portable
gaming à 2,8 GHz avec un pipeline bien plus large ; le téléphone est un
Snapdragon 625 de milieu de gamme, avec des coeurs bien plus modestes et un
budget thermique/énergétique totalement différent — il tourne sur batterie,
dans un boîtier sans ventilateur, et doit partager ce budget avec l'écran,
le radio, etc.

## Question posée à la classe

Si cette boucle représentait le coût d'une scène (calculs physiques,
animation, IA, post-traitement...), et qu'on visait un budget de frame
fixe (disons 16 ms pour 60 FPS) sur ce téléphone — sachant qu'il est
environ 1,44x plus lent que la machine de développement sur ce genre de
calcul — qu'est-ce qu'il faudrait retirer ou simplifier dans la scène
pour tenir ce budget sur l'appareil ?

Propositions recueillies (à rouvrir au chapitre 16) :

- Réduire le nombre d'objets/particules simulés, ou le nombre d'itérations
  d'un solveur physique.
- Baisser la résolution ou la fréquence des calculs qui n'ont pas besoin
  d'être faits à chaque frame (ex : IA ou physique à 30 Hz au lieu de 60 Hz).
- Simplifier les shaders / le post-traitement (moins de passes, résolution
  réduite pour certains effets).
- Réduire la portée de calcul (LOD — niveau de détail — selon la distance
  à la caméra, culling plus agressif des objets hors-champ).
- Déplacer certains calculs sur un autre coeur/thread plutôt que de tout
  faire en série sur le thread principal.
- Précalculer ce qui peut l'être à l'avance (hors ligne ou au chargement)
  plutôt qu'à chaque frame.


