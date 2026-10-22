// Le cout mesure : l'orientation d'une tete construite de deux facons, un million de fois.
//   matrices    : RotationY * RotationX puis conversion en quaternion normalise (chemin du simulateur)
//   quaternions : produit direct qLacet * qTangage, normalise (meme regard que les matrices)
// Usage : MaSalle <Debug|Release>   (le mot sert d'etiquette aux lignes de mesure)
// Sortie standard : les six lignes de mesure. Sortie d'erreur : la verification et les sommes lues.

#include <cmath>
#include <cstdio>

#include "NKMath/NkAngle.h"
#include "NKMath/NkQuat.h"
#include "NKMath/NkMat.h"
#include "NKTime/NkChrono.h"

using namespace nkentseu;
using namespace nkentseu::math;

namespace {
        const int kIterations = 1000000;

        NkQuatf ParMatrices(float lacetDeg, float tangageDeg) {
                const NkMat4f rotation = NkMat4f::RotationY(NkAngle(lacetDeg)) *
                                         NkMat4f::RotationX(NkAngle(tangageDeg));
                return NkQuatf(rotation).Normalized();
        }

        NkQuatf Directe(float lacetDeg, float tangageDeg) {
                const NkQuatf qLacet = NkQuatf::RotateY(NkAngle(lacetDeg));
                const NkQuatf qTangage = NkQuatf::RotateX(NkAngle(tangageDeg));
                return (qLacet * qTangage).Normalized();
        }

        // Un million de tours ; les quatre composantes de chaque resultat s'ajoutent a une somme lue.
        template <typename F> double Mesurer(F construire, double &somme) {
                double s = 0.0;
                NkChrono c;
                for (int i = 0; i < kIterations; ++i) {
                        const float lacetDeg = (float)(i % 3600) * 0.1f;
                        const float tangageDeg = (float)(i % 900) * 0.1f;
                        const NkQuatf q = construire(lacetDeg, tangageDeg);
                        s += (double)q.x + (double)q.y + (double)q.z + (double)q.w;
                }
                const double ms = (double)c.Elapsed().ToMilliseconds();
                somme = s;
                return ms;
        }
}

int main(int argc, char **argv) {
        const char *config = argc > 1 ? argv[1] : "Release";

        // Verification : les deux versions donnent le meme quaternion (45 degres de lacet, 30 de tangage).
        const NkQuatf a = ParMatrices(45.0f, 30.0f);
        const NkQuatf b = Directe(45.0f, 30.0f);
        const float signe = (a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w) < 0.0f ? -1.0f : 1.0f; // q et -q : meme rotation
        float ecart = 0.0f;
        const float d[4] = {a.x - signe * b.x, a.y - signe * b.y, a.z - signe * b.z, a.w - signe * b.w};
        for (int k = 0; k < 4; ++k)
                if (std::fabs(d[k]) > ecart) ecart = std::fabs(d[k]);
        std::fprintf(stderr, "matrices    : (%.6f, %.6f, %.6f, %.6f)\n", a.x, a.y, a.z, a.w);
        std::fprintf(stderr, "quaternions : (%.6f, %.6f, %.6f, %.6f)\n", b.x, b.y, b.z, b.w);
        std::fprintf(stderr, "ecart maximal entre les deux : %.8f\n", ecart);

        // Trois mesures de chaque version.
        for (int tour = 0; tour < 3; ++tour) {
                double sM = 0.0, sQ = 0.0;
                const double msM = Mesurer(ParMatrices, sM);
                const double msQ = Mesurer(Directe, sQ);
                std::printf("%s, matrices : %.3f ms\n", config, msM);
                std::printf("%s, quaternions : %.3f ms\n", config, msQ);
                std::fprintf(stderr, "somme lue (tour %d) : matrices %.4f, quaternions %.4f\n", tour + 1, sM, sQ);
        }
        return 0;
}
