// Ce que la prediction vous apporte : trois gestes de tete ecrits en code (lent, rapide, demi-tour),
// echantillonnes comme un casque a 72 hertz, avec et sans prediction, SANS fenetre.
//   Chaque pose k (t = k * periode) est un quaternion NkQuatf::RotateY(NkAngle(lacet en degres)).
//   La vitesse de la pose k vient des poses k-1 et k (VitesseAngulaire) ; avec prediction on montre la
//   pose extrapolee d'une periode (Extrapoler), sans prediction la pose telle quelle. Les deux sont
//   comparees a la VRAIE pose de l'instant d'affichage, t + periode : Lacet(t + periode).
//   On mesure pendant le geste : poses k = 1 .. K avec K = partie entiere de duree * 72.
//   rapport = erreur sans prediction / erreur avec prediction (le facteur de reduction de l'erreur).
// Sortie standard : une ligne par geste,
//   <nom> (<amplitude> deg en <duree> ms) : avec A deg, sans S deg, rapport R
//   L'angle de ces lignes est calcule par atan2 en double (precis pour les tres petits angles).
// Sortie d'erreur : formule de l'enonce (acos en 32 bits), erreurs par tiers du geste, controle a vitesse constante.

#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <vector>

#include "NKPlatform/NkPlatformDetect.h"
#include "NKWindow/NKMain.h"
#include "NKMath/NkAngle.h"
#include "NKMath/NkQuat.h"

using namespace nkentseu;
using namespace nkentseu::math;

static void ConfigureAppData(NkAppData &d) {
        d.appName = "CeQueLaPredictionApporte";
}
NK_REGISTER_ENTRY_APPDATA_UPDATER(ConfigureAppData)

namespace {
        const double kPi = 3.14159265358979323846;
        const double kHz = 72.0; // frequence du casque simule

        // Un geste en cosinus : depart et arrivee doux. t en secondes depuis le debut, lacet en degres.
        double Lacet(double t, double amplitude, double duree) {
                if (t <= 0.0) return 0.0;
                if (t >= duree) return amplitude;
                return amplitude * 0.5 * (1.0 - std::cos(kPi * t / duree));
        }
        // Meme geste a vitesse rigoureusement constante (controle seulement).
        double LacetLineaire(double t, double amplitude, double duree) {
                if (t <= 0.0) return 0.0;
                if (t >= duree) return amplitude;
                return amplitude * t / duree;
        }

        // Vitesse angulaire entre deux poses horodatees (module de realite virtuelle du moteur).
        NkVec3f VitesseAngulaire(const NkQuatf &from, const NkQuatf &to, float32 dtSeconds) {
                if (dtSeconds <= 0.f) {
                        return NkVec3f(0.f, 0.f, 0.f);
                }
                NkQuatf delta = (to * from.Conjugate()).Normalized();
                // Les deux hemispheres du quaternion codent la meme rotation ; on
                // force le chemin court, sinon un delta minuscule peut se lire
                // comme un tour presque complet dans l'autre sens.
                if (delta.w < 0.f) {
                        delta = NkQuatf(-delta.x, -delta.y, -delta.z, -delta.w);
                }
                const float32 angleRad = delta.Angle().Rad();
                if (angleRad <= 1e-6f) {
                        return NkVec3f(0.f, 0.f, 0.f);
                }
                return delta.Axis() * (angleRad / dtSeconds);
        }

        // Predire : faire tourner l'orientation a cette vitesse pendant le temps qui reste avant l'affichage.
        NkQuatf Extrapoler(const NkQuatf &orientation, const NkVec3f &vitesse, float32 dtSeconds) {
                const float32 speedRad = vitesse.Len();
                if (speedRad > 1e-6f) {
                        const NkVec3f axis = vitesse * (1.f / speedRad);
                        // Delta en espace de BASE : il se compose a GAUCHE de
                        // l'orientation courante (meme convention que la mesure).
                        const NkQuatf delta(NkAngle::FromRad(speedRad * dtSeconds), axis);
                        return (delta * orientation).Normalized();
                }
                return orientation;
        }

        NkQuatf Pose(double lacetDeg) { return NkQuatf::RotateY(NkAngle((float)lacetDeg)); }

        // Angle (degres) qui separe a de b : formule de l'enonce (acos, 32 bits).
        double AngleDegEnonce(const NkQuatf &a, const NkQuatf &b) {
                return (double)(b * a.Conjugate()).Normalized().Angle().Deg();
        }
        // Le meme angle par atan2 en double : precis aussi pour les tres petits angles.
        double AngleDeg(const NkQuatf &a, const NkQuatf &b) {
                const NkQuatf q = (b * a.Conjugate()).Normalized();
                const double s = std::sqrt((double)q.x * q.x + (double)q.y * q.y + (double)q.z * q.z);
                return 2.0 * std::atan2(s, std::fabs((double)q.w)) * 180.0 / kPi;
        }

        struct Resultat {
                int n = 0;
                double avec = 0.0, sans = 0.0;             // moyennes (atan2, double)
                double avecEnonce = 0.0, sansEnonce = 0.0; // moyennes (formule de l'enonce)
                double tiersAvec[3] = {0, 0, 0}, tiersSans[3] = {0, 0, 0};
        };

        // Mesure sur un geste. lineaire : geste a vitesse constante (controle). Pour le controle, on ne garde
        // que les poses dont l'instant d'affichage reste dans le geste (t + periode <= duree).
        Resultat Mesurer(double amplitude, double duree, bool lineaire) {
                Resultat r;
                const double P = 1.0 / kHz;
                const int K = (int)std::floor((lineaire ? duree - P : duree) * kHz + 1e-9);
                auto f = [&](double t) { return lineaire ? LacetLineaire(t, amplitude, duree) : Lacet(t, amplitude, duree); };
                int cptTiers[3] = {0, 0, 0};
                NkQuatf qPrev = Pose(f(0.0));
                for (int k = 1; k <= K; ++k) {
                        const double t = k * P;
                        const NkQuatf q = Pose(f(t));
                        const NkVec3f v = VitesseAngulaire(qPrev, q, (float)P);
                        const NkQuatf qPredite = Extrapoler(q, v, (float)P);
                        const NkQuatf qVraie = Pose(f(t + P)); // la vraie pose de l'instant d'affichage
                        const double eAvec = AngleDeg(qPredite, qVraie);
                        const double eSans = AngleDeg(q, qVraie);
                        r.avec += eAvec;
                        r.sans += eSans;
                        r.avecEnonce += AngleDegEnonce(qPredite, qVraie);
                        r.sansEnonce += AngleDegEnonce(q, qVraie);
                        const int tiers = ((k - 1) * 3) / K;
                        r.tiersAvec[tiers] += eAvec;
                        r.tiersSans[tiers] += eSans;
                        ++cptTiers[tiers];
                        qPrev = q;
                }
                r.n = K;
                if (K > 0) {
                        r.avec /= K;
                        r.sans /= K;
                        r.avecEnonce /= K;
                        r.sansEnonce /= K;
                }
                for (int i = 0; i < 3; ++i) {
                        if (cptTiers[i] > 0) {
                                r.tiersAvec[i] /= cptTiers[i];
                                r.tiersSans[i] /= cptTiers[i];
                        }
                }
                return r;
        }

        void Geste(const char *nom, double amplitude, double duree) {
                const Resultat r = Mesurer(amplitude, duree, false);
                std::printf("%s (%.0f deg en %.0f ms) : avec %.5f deg, sans %.5f deg, rapport %.2f\n", nom, amplitude,
                            duree * 1000.0, r.avec, r.sans, r.avec > 0.0 ? r.sans / r.avec : 0.0);
                std::fflush(stdout);
                std::fprintf(stderr, "%s : %d poses mesurees ; vitesse de pointe (formule, calcul) %.1f deg/s\n", nom, r.n,
                             amplitude * kPi / (2.0 * duree));
                std::fprintf(stderr, "  formule de l'enonce (acos, 32 bits) : avec %.5f, sans %.5f\n", r.avecEnonce, r.sansEnonce);
                std::fprintf(stderr, "  par tiers du geste : avec %.5f / %.5f / %.5f ; sans %.5f / %.5f / %.5f\n", r.tiersAvec[0],
                             r.tiersAvec[1], r.tiersAvec[2], r.tiersSans[0], r.tiersSans[1], r.tiersSans[2]);
        }
}

int nkmain(const NkEntryState &state) {
        (void)state;

        // Les trois gestes : rapide imposé (90 deg en 400 ms), lent et demi-tour de mon choix.
        Geste("rapide", 90.0, 0.400);
        Geste("lent", 30.0, 1.200);
        Geste("demi-tour", 180.0, 0.300);

        // Controle : a vitesse rigoureusement constante, l'erreur avec prediction doit etre quasi nulle.
        const Resultat c = Mesurer(90.0, 0.400, true);
        std::fprintf(stderr, "controle vitesse constante (90 deg en 400 ms, %d poses) : avec %.6f deg, sans %.6f deg\n", c.n,
                     c.avec, c.sans);
        return 0;
}
