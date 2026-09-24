#include <cstdio>
#include <chrono>

int main() {
    const long long N = 411300000LL; // a ajuster pour viser ~1s
    volatile double resultat = 0.0;
    auto debut = std::chrono::high_resolution_clock::now();

    for (long long i = 0; i < N; ++i) {
        resultat += (i * 1.0000001) - (i * 0.9999999);
    }

    auto fin = std::chrono::high_resolution_clock::now();
    double ms = std::chrono::duration<double, std::milli>(fin - debut).count();
    printf("Iterations: %lld\n", N);
    printf("Temps: %.3f ms (%.3f s)\n", ms, ms / 1000.0);
    printf("Resultat (ignorer): %f\n", resultat);
    return 0;
}
