# Exercice 10 — Mesurer en Debug et en Release

## Objectif

Écrire une boucle de calcul lourd, la chronométrer, la construire en Debug
puis en Release, et comparer les deux mesures face à un budget de 11 ms par
image de casque.

## src/main.cpp

```cpp
#include <chrono>
#include <cmath>
#include <cstdio>
#include <vector>

// Simule une charge de calcul "lourde" representative d'une passe de rendu
// (transformations, eclairage) : un grand nombre d'operations flottantes
// non triviales sur un tableau de donnees.
static double ChargeLourde(int nElements, int nIterations)
{
    std::vector<double> data(nElements);
    for (int i = 0; i < nElements; ++i)
        data[i] = static_cast<double>(i) * 0.0001;

    double acc = 0.0;
    for (int iter = 0; iter < nIterations; ++iter)
    {
        for (int i = 0; i < nElements; ++i)
        {
            double x = data[i];
            double v = std::sin(x) * std::cos(x) + std::sqrt(std::fabs(x) + 1.0);
            data[i] = v;
            acc += v;
        }
    }
    return acc;
}

int main()
{
    const int nElements = 20000;
    const int nIterations = 50;

    auto t0 = std::chrono::high_resolution_clock::now();
    double result = ChargeLourde(nElements, nIterations);
    auto t1 = std::chrono::high_resolution_clock::now();

    double ms = std::chrono::duration<double, std::milli>(t1 - t0).count();

    std::printf("Resultat (anti-optimisation): %f\n", result);
    std::printf("Temps de calcul: %f ms\n", ms);

    return 0;
}
```

## Salle.jenga

```python
from Jenga import *

with workspace("Salle"):
    configurations(["Debug", "Release"])
    targetoses([TargetOS.WINDOWS])
    targetarchs([TargetArch.X86_64])

    with project("Salle"):
        consoleapp()
        language("C++")
        cppdialect("C++17")
        files(["src/main.cpp"])

        with filter("config:Debug"):
            defines(["_DEBUG", "DEBUG"])
            optimize("Off")
            symbols(True)

        with filter("config:Release"):
            defines(["NDEBUG"])
            optimize("Speed")
            symbols(False)
```

Point important : `optimize()` doit être fixé explicitement pour chaque
configuration via `filter("config:...")`. Sans ces deux blocs, un projet
Jenga reste par défaut en `Optimization.OFF`, même construit avec
`--config Release` — le nom de la configuration seul ne déclenche aucune
optimisation automatique.

## Mesure réelle en Debug

Configuration: Debug
Target: Windows x86_64
Toolchain: mingw
...
✓ Built: Build\Bin\Debug-Windows\Salle\Salle.exe
Build Successful Time: 2.53s
Status: ✓ SUCCESS


## Mesure réelle en Release

Configuration: Release
Target: Windows x86_64
Toolchain: mingw
...
✓ Built: Build\Bin\Release-Windows\Salle\Salle.exe
Build Successful Time: 0.62s
Status: ✓ SUCCESS


Exécution :
Resultat (anti-optimisation): 1590237.823016
Temps de calcul: 17.440700 ms


## Rapport

| Configuration | Temps mesuré | Rapport au budget (11 ms) |
|---|---|---|
| Debug   | 38,36 ms | ×3,5 le budget |
| Release | 17,44 ms | ×1,6 le budget |

Le Debug est environ 2,2 fois plus lent que le Release pour exactement le
même code source, uniquement à cause de l'absence d'optimisation
(`-O0` contre l'optimisation "Speed" activée par le compilateur mingw).

## Laquelle des deux mesures aurait fait prendre une mauvaise décision ?

Sur cette machine, les **deux** configurations dépassent le budget de
11 ms — donc, contrairement à ce qu'on pourrait attendre naïvement, ce
n'est pas un cas où le Debug "ment" en disant qu'on est hors budget alors
que le Release serait dans les clous. Ici, le vrai risque est ailleurs :

La mesure **Debug (38,36 ms)** est celle qui aurait fait prendre une
mauvaise décision, parce qu'elle exagère l'ampleur du problème d'un facteur
proche de 2,2 par rapport à la réalité du binaire qui sera effectivement
livré (le Release). En jugeant la faisabilité du calcul sur ce chiffre, on
serait tenté d'une réaction disproportionnée — abandonner l'approche,
réécrire l'algorithme en urgence, sacrifier des fonctionnalités — alors que
la mesure Release (17,44 ms), certes encore au-dessus du budget, situe le
problème dans une catégorie très différente : un dépassement modéré (+58 %),
qui appelle une optimisation ciblée plutôt qu'une refonte complète.

La leçon reste la même que si le Release avait été sous les 11 ms : ne
jamais juger un budget de performance sur une mesure Debug. Seule la
mesure Release reflète le comportement du binaire réellement livré à
l'utilisateur du casque.
