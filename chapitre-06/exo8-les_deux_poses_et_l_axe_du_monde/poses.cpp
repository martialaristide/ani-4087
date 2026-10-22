// Les deux poses et l'axe du monde : la position des deux yeux depuis la pose de la tete,
// de deux facons.
//   tete  : l'oeil est decale le long de la DROITE DE LA TETE, c'est-a-dire l'image de +X
//           (la droite de la salle) par l'orientation de la tete : q * (1, 0, 0).
//   monde : l'oeil est decale le long de l'axe x du monde, (1, 0, 0), quelle que soit la tete.
// Salle : avant = -Z, haut = +Y, droite = +X. Positions en metres, ecarts en millimetres.
//
// Sortie standard : les six positions puis les trois ecarts (ce qui va dans poses.txt).
// Sortie d'erreur : une verification de NkQuatf::Right().

#include <cmath>
#include <cstdio>

#include "NKMath/NkAngle.h"
#include "NKMath/NkQuat.h"
#include "NKMath/NkVec.h"

using namespace nkentseu;
using namespace nkentseu::math;

namespace {
        const float kIpd = 0.063f; // ecart entre les pupilles, en metres (valeur du simulateur)

        struct Pose {
                NkVec3f position;
                NkQuatf orientation;
        };

        // position + direction * distance
        NkVec3f Decaler(const NkVec3f &p, const NkVec3f &direction, float distance) {
                return NkVec3f(p.x + direction.x * distance, p.y + direction.y * distance,
                               p.z + direction.z * distance);
        }

        // Evite d'imprimer "-0.0000".
        float Propre(float v) { return std::fabs(v) < 5e-5f ? 0.0f : v; }

        void ImprimerYeux(const char *nom, const char *methode, const NkVec3f &g, const NkVec3f &d) {
                std::printf("%s, %s : G (%.4f, %.4f, %.4f) D (%.4f, %.4f, %.4f)\n", nom, methode,
                            Propre(g.x), Propre(g.y), Propre(g.z), Propre(d.x), Propre(d.y), Propre(d.z));
        }
}

int main() {
        const NkVec3f positionTete(0.30f, 1.70f, 1.20f);
        const NkVec3f droiteMonde(1.0f, 0.0f, 0.0f);
        const float demiEcart = kIpd * 0.5f;

        struct Entree {
                const char *nom;
                Pose pose;
        };
        const Entree poses[3] = {
                {"neutre", {positionTete, NkQuatf::Identity()}},
                {"roulis 30", {positionTete, NkQuatf::RotateZ(NkAngle(30.0f))}},
                {"lacet 90", {positionTete, NkQuatf::RotateY(NkAngle(90.0f))}},
        };

        float ecartsMm[3];
        for (int i = 0; i < 3; ++i) {
                const Pose &tete = poses[i].pose;

                // La droite de la tete : l'image de +X par l'orientation de la tete.
                const NkVec3f droiteTete = tete.orientation * droiteMonde;

                // Methode 1 : le long de la droite de la tete.
                const NkVec3f gaucheTete = Decaler(tete.position, droiteTete, -demiEcart);
                const NkVec3f droitTete = Decaler(tete.position, droiteTete, +demiEcart);
                // Methode 2 : le long de l'axe x du monde.
                const NkVec3f gaucheMonde = Decaler(tete.position, droiteMonde, -demiEcart);
                const NkVec3f droitMonde = Decaler(tete.position, droiteMonde, +demiEcart);

                ImprimerYeux(poses[i].nom, "tete", gaucheTete, droitTete);
                ImprimerYeux(poses[i].nom, "monde", gaucheMonde, droitMonde);

                // Ecart entre l'oeil gauche des deux methodes, en millimetres.
                const float dx = gaucheTete.x - gaucheMonde.x;
                const float dy = gaucheTete.y - gaucheMonde.y;
                const float dz = gaucheTete.z - gaucheMonde.z;
                ecartsMm[i] = std::sqrt(dx * dx + dy * dy + dz * dz) * 1000.0f;

                // Verification (stderr) : NkQuatf::Right() rend-il bien l'image de +X ?
                const NkVec3f r = tete.orientation.Right();
                std::fprintf(stderr, "%s : Right() = (%.4f, %.4f, %.4f), q * (1,0,0) = (%.4f, %.4f, %.4f)\n",
                             poses[i].nom, Propre(r.x), Propre(r.y), Propre(r.z), Propre(droiteTete.x),
                             Propre(droiteTete.y), Propre(droiteTete.z));
        }

        for (int i = 0; i < 3; ++i)
                std::printf("%s : ecart %.1f mm\n", poses[i].nom, ecartsMm[i]);
        return 0;
}
