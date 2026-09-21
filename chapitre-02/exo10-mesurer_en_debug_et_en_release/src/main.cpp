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
