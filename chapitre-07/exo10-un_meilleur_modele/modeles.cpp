// Un meilleur modele ? Prediction du lacet une periode plus loin (72 Hz), de deux facons, sur quatre gestes.
//   Travail sur le lacet seul, en degres, sans quaternion. Calcul fait au demarrage, sans ouvrir de fenetre.
//   Poses : p0, p1, p2 = les trois dernieres poses (la plus ancienne en premier), a intervalle T = 1/72 s.
//   Vitesse (identique pour les deux modeles, tiree des DEUX dernieres poses) : v = (p2 - p1) / T.
//   Acceleration (tiree des TROIS dernieres poses)                          : a = (p2 - 2 p1 + p0) / (T * T).
//   Modele a vitesse constante : prediction = p2 + v * T.
//   Modele a acceleration      : prediction = p2 + v * T + a * T * T / 2   (meme horizon : une periode, memes poses).
//   Erreur = |prediction - lacet reel a l'instant k T|, pour tous les instants k T ayant lieu pendant le geste et les
//   200 ms qui suivent (0 < k T <= duree + 0.2 s). Avant l'instant 0, la tete est au repos (a 0) pour les trois gestes en
//   cosinus, et elle tourne deja a vitesse constante pour l'arret (sa pose est v * t, y compris pour t < 0).
//   Quatre gestes :
//     rapide    : le quart de tour de l'enonce, 90 deg en 400 ms, depart et arrivee doux (cosinus).
//     lent      : meme forme, 30 deg en 1500 ms.
//     demi-tour : meme forme, 180 deg en 250 ms (brusque).
//     arret     : vitesse constante 200 deg/s pendant 450 ms (90 deg), puis arret net (la pose reste a 90 deg).
// Sortie standard : une ligne par geste. Sortie d'erreur : diagnostics (images ou l'acceleration est pire, essai avec bruit).

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstdint>
#include <vector>

#include "NKPlatform/NkPlatformDetect.h"
#include "NKWindow/NKMain.h"
#include "NKWindow/Core/NkWindow.h"
#include "NKWindow/Core/NkWindowConfig.h"
#include "NKWindow/Core/NkWESystem.h"

using namespace nkentseu;

static void ConfigureAppData(NkAppData &d) {
        d.appName = "UnMeilleurModele";
}
NK_REGISTER_ENTRY_APPDATA_UPDATER(ConfigureAppData)

namespace {
        const double kPi = 3.14159265358979323846;
        const double kPeriode = 1.0 / 72.0; // echantillonnage a 72 hertz

        // Geste en cosinus : depart et arrivee doux. t en secondes depuis le debut, lacet en degres.
        double LacetDoux(double t, double amplitude, double duree) {
                if (t <= 0.0) return 0.0;
                if (t >= duree) return amplitude;
                return amplitude * 0.5 * (1.0 - std::cos(kPi * t / duree));
        }

        // Un quart de tour en 400 ms, depart et arrivee doux (celui de l'enonce).
        double Lacet(double t) { return LacetDoux(t, 90.0, 0.400); }

        // Vitesse constante puis arret net : la pose est vitesse * min(t, tArret).
        double LacetArret(double t, double vitesse, double tArret) { return vitesse * std::min(t, tArret); }

        struct Geste {
                const char *nom;
                int forme;      // 0 = cosinus, 1 = arret net
                double a, b;    // cosinus : amplitude (deg), duree (s) ; arret : vitesse (deg/s), instant de l'arret (s)
                double Duree() const { return b; }
                double Pose(int i) const {
                        const double t = (double)i * kPeriode;
                        if (forme == 1) return LacetArret(t, a, b);
                        if (a == 90.0 && b == 0.400) return Lacet(t);
                        return LacetDoux(t, a, b);
                }
        };

        struct Resultat {
                int n = 0, accelPire = 0, accelMeilleure = 0;
                double moyV = 0, maxV = 0, moyA = 0, maxA = 0;
                double plusGrandePerte = 0, tPerte = 0;   // acceleration pire que la vitesse : ea - ev
                double plusGrandGain = 0, tGain = 0;      // acceleration meilleure : ev - ea
        };

        Resultat Evalue(const Geste &g) {
                Resultat r;
                const int kMax = (int)std::floor((g.Duree() + 0.2) / kPeriode + 1e-9);
                double sV = 0, sA = 0;
                for (int k = 1; k <= kMax; ++k) {
                        const double p0 = g.Pose(k - 3), p1 = g.Pose(k - 2), p2 = g.Pose(k - 1); // les trois dernieres poses
                        const double cible = g.Pose(k);                                          // la pose une periode plus loin
                        const double v = (p2 - p1) / kPeriode;
                        const double acc = (p2 - 2.0 * p1 + p0) / (kPeriode * kPeriode);
                        const double predV = p2 + v * kPeriode;
                        const double predA = predV + acc * kPeriode * kPeriode / 2.0;
                        const double eV = std::fabs(predV - cible), eA = std::fabs(predA - cible);
                        sV += eV;
                        sA += eA;
                        r.maxV = std::max(r.maxV, eV);
                        r.maxA = std::max(r.maxA, eA);
                        ++r.n;
                        if (eA > eV + 1e-9) {
                                ++r.accelPire;
                                if (eA - eV > r.plusGrandePerte) { r.plusGrandePerte = eA - eV; r.tPerte = k * kPeriode; }
                        } else if (eA < eV - 1e-9) {
                                ++r.accelMeilleure;
                                if (eV - eA > r.plusGrandGain) { r.plusGrandGain = eV - eA; r.tGain = k * kPeriode; }
                        }
                }
                r.moyV = sV / r.n;
                r.moyA = sA / r.n;
                return r;
        }

        // Generateur pseudo-aleatoire deterministe (essai avec bruit), uniforme dans [-1, 1].
        double Bruit(uint32_t &etat) {
                etat = etat * 1664525u + 1013904223u;
                return (double)(etat >> 8) / 8388608.0 * 1.0 - 1.0; // etat >> 8 : 24 bits -> [0, 2) - 1
        }
}

int nkmain(const NkEntryState &state) {
        (void)state;

        const Geste gestes[4] = {
                {"rapide (90 deg en 400 ms)", 0, 90.0, 0.400},
                {"lent (30 deg en 1500 ms)", 0, 30.0, 1.500},
                {"demi-tour (180 deg en 250 ms)", 0, 180.0, 0.250},
                {"arret (200 deg/s pendant 450 ms, puis arret net)", 1, 200.0, 0.450},
        };

        for (const Geste &g : gestes) {
                const Resultat r = Evalue(g);
                std::printf("%s : vitesse %.4f deg (max %.4f), acceleration %.4f deg (max %.4f)\n", g.nom, r.moyV, r.maxV, r.moyA, r.maxA);
                std::fprintf(stderr,
                             "  %d predictions ; acceleration meilleure %d, pire %d ; plus grande perte %.4f deg (a %.1f ms), plus grand gain %.4f deg (a %.1f ms)\n",
                             r.n, r.accelMeilleure, r.accelPire, r.plusGrandePerte, r.tPerte * 1000.0, r.plusGrandGain, r.tGain * 1000.0);
        }
        std::fflush(stdout);

        // Essai supplementaire, hors enonce : tete immobile (lacet 0), chaque pose recoit un bruit uniforme dans [-0.1, 0.1] deg.
        {
                const double amplitudeBruit = 0.1;
                uint32_t etat = 12345u;
                const int n = 100000;
                double sV = 0, sA = 0;
                for (int i = 0; i < n; ++i) {
                        const double p0 = amplitudeBruit * Bruit(etat), p1 = amplitudeBruit * Bruit(etat), p2 = amplitudeBruit * Bruit(etat);
                        const double v = (p2 - p1) / kPeriode;
                        const double acc = (p2 - 2.0 * p1 + p0) / (kPeriode * kPeriode);
                        const double predV = p2 + v * kPeriode;
                        const double predA = predV + acc * kPeriode * kPeriode / 2.0;
                        sV += std::fabs(predV - 0.0);
                        sA += std::fabs(predA - 0.0);
                }
                std::fprintf(stderr, "essai avec bruit (tete immobile, +-%.2f deg sur chaque pose, %d predictions) : vitesse %.4f deg, acceleration %.4f deg, rapport %.3f\n",
                             amplitudeBruit, n, sV / n, sA / n, (sA / n) / (sV / n));
        }
        return 0;
}
