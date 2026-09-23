# Exercice 21 — La même mesure, deux machines

## Boucle de calcul chronométrée
```cpp
const long long N = 411300000LL;
volatile double resultat = 0.0;
auto debut = std::chrono::high_resolution_clock::now();
for (long long i = 0; i < N; ++i) {
    resultat += (i * 1.0000001) - (i * 0.9999999);
}
auto fin = std::chrono::high_resolution_clock::now();
double ms = std::chrono::duration<double, std::milli>(fin - debut).count();
​```

`volatile` empêche le compilateur d'éliminer la boucle par optimisation (le résultat n'est
jamais utilisé autrement). Le nombre d'itérations (411 300 000) a été calibré pour prendre
~1 seconde sur mon PC en Release (`-O2`).

## Résultat sur PC
```
$ g++ -O2 test_benchmark_pc.cpp -o test_benchmark_pc.exe
$ ./test_benchmark_pc.exe
Iterations: 411300000
Temps: 998.324 ms (0.998 s)
​```

## Résultat sur l'appareil
Même boucle, même nombre d'itérations, intégrée dans l'application Android (Release,
optimisations actives) :

09-23 06:40:18.950 19594 19609 I ExoLePaquetVide: Benchmark: 411300000 iterations en 3484.841 ms
​```

Rapport
Machine	Temps pour 411 300 000 iterations	Debit approx.
PC	998.324 ms	~412 000 iter/ms
Appareil	3484.841 ms	~118 000 iter/ms

Ratio : l'appareil est environ 3,49x plus lent que le PC sur ce calcul (3484.841 / 998.324).

Ce que ça implique pour un budget de 11 ms

Un budget de 11 ms correspond a une frame a ~90 Hz (1000 ms / 90 ≈ 11,1 ms), le seuil
habituel en VR pour eviter le "judder" et le desagrement visuel.

Sur PC, en 11 ms je peux faire environ 412 000 x 11 ≈ 4 530 000 iterations de ce calcul.
Sur l'appareil, en 11 ms je ne peux faire qu'environ 118 000 x 11 ≈ 1 300 000 iterations
du meme calcul — soit 3,5 fois moins de travail dans le meme budget de temps.

Concretement : si je developpe et regle mes performances en observant le comportement sur
mon PC, je vais systematiquement sous-estimer le cout reel sur l'appareil cible. Un budget
de calcul qui tient confortablement dans 11 ms sur mon PC peut prendre pres de 40 ms sur
l'appareil — largement au-dela d'une frame, ce qui produit des saccades. Il faut donc :

toujours mesurer sur l'appareil cible, jamais extrapoler depuis le PC ;
prevoir une marge (viser un budget bien en-dessous de 11 ms sur PC pour absorber
l'ecart materiel) ;
envisager d'optimiser specifiquement pour l'architecture mobile (repartition sur les
coeurs, reduction du travail par frame, simplification des calculs) plutot que de se fier
aux temps mesures en developpement sur poste de travail.


