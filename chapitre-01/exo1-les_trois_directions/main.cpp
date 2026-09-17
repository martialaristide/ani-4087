#include "Directions.hpp"
#include "NkVec3f.hpp"

#include <cassert>
#include <cmath>
#include <cstdio>
#include <cstdlib>

namespace {

constexpr nk_float32 kTolerance = 1e-5f;

bool IsUnit(const NkVec3f& v) {
    return std::fabs(Dot(v, v) - 1.0f) < kTolerance;
}

bool IsOrthogonal(const NkVec3f& a, const NkVec3f& b) {
    return std::fabs(Dot(a, b)) < kTolerance;
}

}  // namespace

int main() {
    const NkVec3f avant = Avant();
    const NkVec3f haut = Haut();
    const NkVec3f droite = Droite();

    // Documente, dans le code lui-meme, que la convention posee forme bien
    // une base orthonormee (actif seulement en Debug, via <cassert>).
    assert(IsUnit(avant) && "Avant() doit etre unitaire");
    assert(IsUnit(haut) && "Haut() doit etre unitaire");
    assert(IsUnit(droite) && "Droite() doit etre unitaire");
    assert(IsOrthogonal(avant, haut) && "Avant() et Haut() doivent etre orthogonaux");
    assert(IsOrthogonal(avant, droite) && "Avant() et Droite() doivent etre orthogonaux");
    assert(IsOrthogonal(haut, droite) && "Haut() et Droite() doivent etre orthogonaux");

    nk_float32 x = 0.0f;
    nk_float32 y = 0.0f;
    nk_float32 z = 0.0f;
    if (std::scanf("%f %f %f", &x, &y, &z) != 3) {
        std::fprintf(stderr,
                      "Usage : trois nombres reels separes par des espaces (ex: 1.0 2.0 3.0)\n");
        return EXIT_FAILURE;
    }

    const NkVec3f point{x, y, z};

    std::printf("%.4f\n", Dot(point, avant));
    std::printf("%.4f\n", Dot(point, haut));
    std::printf("%.4f\n", Dot(point, droite));

    return EXIT_SUCCESS;
}
