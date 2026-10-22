// Les deux ordres et la matrice : le regard d'une tete construit de trois facons.
//
// Lacet = 45 degres autour de l'axe vertical (Y), tangage = 30 degres autour de X.
// Avant de la salle : (0, 0, -1).
//
//   1. qLacet * qTangage            (produit de quaternions du moteur)
//   2. qTangage * qLacet            (l'autre ordre)
//   3. RotationY * RotationX -> NkQuatf(matrice).Normalized()   (chemin du simulateur)
//
// Verification a la main : une tete qui leve les yeux de 30 degres, sans rien
// d'autre, regarde vers (0, sin 30, -cos 30). Sa composante y vaut donc
// sin(30 degres) = 0.5, et le lacet (rotation autour de la verticale) ne change
// pas y. On compare ce 0.5 calcule a la main aux y des trois regards.

#include <cmath>
#include <cstdio>

#include "NKWindow/NKMain.h"
#include "NKMath/NkAngle.h"
#include "NKMath/NkQuat.h"
#include "NKMath/NkMat.h"

using namespace nkentseu;
using namespace nkentseu::math;

namespace {
    const float kPi = 3.14159265358979f;

    // Evite d'imprimer "-0.0000".
    float Propre(float v) { return std::fabs(v) < 5e-5f ? 0.0f : v; }

    void Imprimer(const char *nom, const NkVec3f &v) {
        std::printf("%s : (%.4f, %.4f, %.4f)\n", nom, Propre(v.x), Propre(v.y), Propre(v.z));
    }
}

int nkmain(const NkEntryState &state) {
    (void)state;

    const float lacetDeg = 45.0f;
    const float tangageDeg = 30.0f;
    const NkVec3f avant(0.0f, 0.0f, -1.0f);

    // Les deux ordres, en quaternions du moteur.
    const NkQuatf qLacet = NkQuatf::RotateY(NkAngle(lacetDeg));
    const NkQuatf qTangage = NkQuatf::RotateX(NkAngle(tangageDeg));
    const NkVec3f regardLacetTangage = (qLacet * qTangage) * avant;
    const NkVec3f regardTangageLacet = (qTangage * qLacet) * avant;

    // Le chemin du simulateur : deux matrices et une conversion.
    const NkMat4f rotation = NkMat4f::RotationY(NkAngle(lacetDeg)) *
                             NkMat4f::RotationX(NkAngle(tangageDeg));
    const NkQuatf q = NkQuatf(rotation).Normalized();
    const NkVec3f regardMatrices = q * avant;

    Imprimer("lacet*tangage", regardLacetTangage);
    Imprimer("tangage*lacet", regardTangageLacet);
    Imprimer("matrices", regardMatrices);

    // Calcul a la main : y d'une tete qui a leve les yeux de 30 degres.
    const float yMain = std::sin(tangageDeg * kPi / 180.0f);
    std::printf("a la main (y = sin 30 degres) : %.4f\n", Propre(yMain));
    std::printf("ecart y a la main - lacet*tangage : %.4f\n", Propre(regardLacetTangage.y - yMain));
    std::printf("ecart y a la main - tangage*lacet : %.4f\n", Propre(regardTangageLacet.y - yMain));
    std::printf("ecart y a la main - matrices      : %.4f\n", Propre(regardMatrices.y - yMain));
    return 0;
}
