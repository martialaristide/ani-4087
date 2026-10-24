// La latence injectee et la borne : une tete qui fait un quart de tour en 400 ms (commence a t = 1 s), des poses
// a 72 hertz gardees dans un tableau circulaire de 64, et un echantillon RENDU EN RETARD (comme le simulateur du
// moteur). On predit la pose de t + periode depuis cet echantillon, bornee a 100 ms, et on la compare a la VRAIE
// pose de t + periode. SANS fenetre. Tous les temps sont en secondes.
//
// Sortie standard (7 lignes) :
//   latence L ms : avec A deg (max X), sans S deg (max Y)      pour L = 0, 10, 20, 40, 80 (t de 1 s a 1,9 s)
//   sans borne : echantillon a T s, horizon H ms, predit P deg, vrai V deg
//   avec borne : echantillon a T s, horizon H ms, predit P deg, vrai V deg
//   (latence d'une seconde ; l'image est celle dont l'echantillon est le plus proche de t = 1,2 s)
// Sortie d'erreur : retards reels, horizons, degres extrapoles, formule de l'enonce, latences au-dela de 80 ms.
// Les angles sont calcules par atan2 en double ; le lacet lu sur un quaternion est ramene entre -180 et 180 deg.

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
        d.appName = "LaLatenceInjectee";
}
NK_REGISTER_ENTRY_APPDATA_UPDATER(ConfigureAppData)

namespace {
        const double kPi = 3.14159265358979323846;
        const double kHz = 72.0; // frequence du casque simule
        const double kPeriode = 1.0 / kHz;
        const double kDebutGeste = 1.0; // le quart de tour commence a t = 1 s
        const float kBorne = 0.1f;      // fenetre d'extrapolation maximale : 100 ms
        const int kPoses = 216;         // une pose par periode, pendant trois secondes
        const int kPremier = 72;        // t = 1,0 s
        const int kDernier = 136;       // t = 1,9 s (136 / 72 = 1,889 s)

        // Un quart de tour en 400 ms, depart et arrivee doux. t en secondes depuis le debut du geste, lacet en degres.
        double Lacet(double t) {
                const double amplitude = 90.0, duree = 0.400;
                if (t <= 0.0) return 0.0;
                if (t >= duree) return amplitude;
                return amplitude * 0.5 * (1.0 - std::cos(kPi * t / duree));
        }
        // Lacet de la tete a l'instant absolu t (le geste commence a t = 1 s).
        double LacetTete(double t) { return Lacet(t - kDebutGeste); }

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
        // Le meme angle par atan2 en double.
        double AngleDeg(const NkQuatf &a, const NkQuatf &b) {
                const NkQuatf q = (b * a.Conjugate()).Normalized();
                const double s = std::sqrt((double)q.x * q.x + (double)q.y * q.y + (double)q.z * q.z);
                return 2.0 * std::atan2(s, std::fabs((double)q.w)) * 180.0 / kPi;
        }
        // Lacet d'une rotation autour de l'axe vertical : 2 * atan2(q.y, q.w), ramene entre -180 et 180 degres
        // (on prend d'abord le quaternion de w >= 0, car q et -q codent la meme rotation).
        double LacetLu(const NkQuatf &q) {
                double y = q.y, w = q.w;
                if (w < 0.0) {
                        y = -y;
                        w = -w;
                }
                return 2.0 * std::atan2(y, w) * 180.0 / kPi;
        }

        // Une pose horodatee, comme NkXrTimedPose : la pose, son instant (secondes) et sa vitesse.
        struct PoseHorodatee {
                double time = 0.0;
                NkQuatf pose = NkQuatf::Identity();
                NkVec3f velocity = NkVec3f(0.f, 0.f, 0.f);
        };

        // Le simulateur garde les 64 dernieres poses dans un tableau circulaire.
        class Simulateur {
        public:
                static const unsigned kMaxSamples = 64u;

                void Push(const PoseHorodatee &s) {
                        mSampleHead = (mSampleHead + 1u) % kMaxSamples;
                        mSamples[mSampleHead] = s;
                        if (mSampleCount < kMaxSamples) ++mSampleCount;
                }

                const PoseHorodatee &SampleAt(double now, double latencySeconds) const {
                        // Latence nulle : l'echantillon le plus recent, point.
                        if (latencySeconds <= 0.0 || mSampleCount <= 1u) {
                                return mSamples[mSampleHead];
                        }
                        // Latence simulee : on rend l'echantillon d'il y a « latence » secondes.
                        const double target = now - latencySeconds;
                        unsigned index = mSampleHead;
                        for (unsigned i = 0u; i < mSampleCount; ++i) {
                                if (mSamples[index].time <= target) {
                                        return mSamples[index];
                                }
                                index = (index + kMaxSamples - 1u) % kMaxSamples;
                        }
                        // Plus vieux disponible : l'historique ne remonte pas si loin.
                        return mSamples[(mSampleHead + kMaxSamples - (mSampleCount - 1u)) % kMaxSamples];
                }

        private:
                PoseHorodatee mSamples[kMaxSamples];
                unsigned mSampleHead = kMaxSamples - 1u;
                unsigned mSampleCount = 0u;
        };

        // Pose predite a l'instant when depuis un echantillon. borne : fenetre limitee a +- 100 ms.
        NkQuatf Predire(const PoseHorodatee &s, double when, bool borne, float &dtUtilise) {
                float dt = (float)(when - s.time);
                if (borne) {
                        if (dt < -kBorne) dt = -kBorne;
                        if (dt > kBorne) dt = kBorne;
                }
                dtUtilise = dt;
                return Extrapoler(s.pose, s.velocity, dt);
        }

        // La pose numero k de la tete, avec sa vitesse tiree de la pose precedente.
        PoseHorodatee Echantillon(int k, const NkQuatf &precedente) {
                PoseHorodatee e;
                e.time = k * kPeriode;
                e.pose = Pose(LacetTete(e.time));
                e.velocity = (k > 0) ? VitesseAngulaire(precedente, e.pose, (float)kPeriode) : NkVec3f(0.f, 0.f, 0.f);
                return e;
        }

        struct Stat {
                int n = 0, bornees = 0;
                double avec = 0.0, sans = 0.0, maxAvec = 0.0, maxSans = 0.0;
                double avecEnonce = 0.0, sansEnonce = 0.0;
                double retardMoy = 0.0, horizonMoy = 0.0;
        };

        // Mesure pour une latence : de t = 1 s a t = 1,9 s, avec prediction (bornee) puis sans.
        Stat MesurerLatence(double latenceS) {
                Stat st;
                Simulateur sim;
                NkQuatf prec = Pose(LacetTete(0.0));
                for (int k = 0; k < kPoses; ++k) {
                        const PoseHorodatee e = Echantillon(k, prec);
                        prec = e.pose;
                        sim.Push(e);
                        if (k < kPremier || k > kDernier) continue;
                        const double now = e.time;
                        const PoseHorodatee s = sim.SampleAt(now, latenceS);
                        float dt = 0.f;
                        const NkQuatf predite = Predire(s, now + kPeriode, true, dt);
                        const NkQuatf vraie = Pose(LacetTete(now + kPeriode)); // la vraie pose de l'instant d'affichage
                        const double eAvec = AngleDeg(predite, vraie);
                        const double eSans = AngleDeg(s.pose, vraie);
                        st.avec += eAvec;
                        st.sans += eSans;
                        if (eAvec > st.maxAvec) st.maxAvec = eAvec;
                        if (eSans > st.maxSans) st.maxSans = eSans;
                        st.avecEnonce += AngleDegEnonce(predite, vraie);
                        st.sansEnonce += AngleDegEnonce(s.pose, vraie);
                        st.retardMoy += now - s.time;
                        st.horizonMoy += (now + kPeriode) - s.time;
                        if (std::fabs((now + kPeriode) - s.time) > (double)kBorne) ++st.bornees;
                        ++st.n;
                }
                if (st.n > 0) {
                        st.avec /= st.n;
                        st.sans /= st.n;
                        st.avecEnonce /= st.n;
                        st.sansEnonce /= st.n;
                        st.retardMoy /= st.n;
                        st.horizonMoy /= st.n;
                }
                return st;
        }
}

int nkmain(const NkEntryState &state) {
        (void)state;

        // ── Les cinq latences ─────────────────────────────────────────────────────
        const int latencesMs[5] = {0, 10, 20, 40, 80};
        for (int i = 0; i < 5; ++i) {
                const Stat st = MesurerLatence(latencesMs[i] * 1e-3);
                std::printf("latence %d ms : avec %.4f deg (max %.4f), sans %.4f deg (max %.4f)\n", latencesMs[i], st.avec, st.maxAvec,
                            st.sans, st.maxSans);
                std::fflush(stdout);
                std::fprintf(stderr, "  latence %d ms : %d instants ; retard reel moyen de l'echantillon %.3f ms, horizon moyen %.3f ms, images ou la borne agit : %d\n",
                             latencesMs[i], st.n, st.retardMoy * 1e3, st.horizonMoy * 1e3, st.bornees);
                std::fprintf(stderr, "    formule de l'enonce (acos, 32 bits) : avec %.4f, sans %.4f\n", st.avecEnonce, st.sansEnonce);
        }

        // ── Sans la borne, puis avec : une latence d'une seconde ───────────────────
        // L'image choisie est celle dont l'echantillon rendu est le plus proche du milieu du geste, t = 1,2 s.
        {
                const double latence = 1.0;
                Simulateur sim;
                NkQuatf prec = Pose(LacetTete(0.0));
                PoseHorodatee meilleur;
                double meilleurNow = 0.0, meilleurEcart = 1e300;
                int meilleurK = -1;
                for (int k = 0; k < kPoses; ++k) {
                        const PoseHorodatee e = Echantillon(k, prec);
                        prec = e.pose;
                        sim.Push(e);
                        const PoseHorodatee s = sim.SampleAt(e.time, latence);
                        const double ecart = std::fabs(s.time - 1.2);
                        if (ecart < meilleurEcart) {
                                meilleurEcart = ecart;
                                meilleur = s;
                                meilleurNow = e.time;
                                meilleurK = k;
                        }
                }
                const double when = meilleurNow + kPeriode;
                const double vraiDeg = LacetLu(Pose(LacetTete(when)));
                float dtSans = 0.f, dtAvec = 0.f;
                const NkQuatf predSans = Predire(meilleur, when, false, dtSans);
                const NkQuatf predAvec = Predire(meilleur, when, true, dtAvec);
                std::printf("sans borne : echantillon a %.4f s, horizon %.1f ms, predit %.2f deg, vrai %.2f deg\n", meilleur.time,
                            (double)dtSans * 1e3, LacetLu(predSans), vraiDeg);
                std::printf("avec borne : echantillon a %.4f s, horizon %.1f ms, predit %.2f deg, vrai %.2f deg\n", meilleur.time,
                            (double)dtAvec * 1e3, LacetLu(predAvec), vraiDeg);
                std::fflush(stdout);

                const double vitesseDegS = (double)meilleur.velocity.Len() * 180.0 / kPi;
                std::fprintf(stderr, "borne : image k = %d, instant demande %.4f s, echantillon rendu a %.4f s (retard reel %.1f ms = %.1f periodes)\n",
                             meilleurK, meilleurNow, meilleur.time, (meilleurNow - meilleur.time) * 1e3,
                             (meilleurNow - meilleur.time) / kPeriode);
                std::fprintf(stderr, "  instant vise (t + periode) %.4f s ; horizon sans borne %.1f ms ; compte a la main 64 / 72 = %.1f ms\n", when,
                             (double)dtSans * 1e3, 64.0 / 72.0 * 1e3);
                std::fprintf(stderr, "  lacet de l'echantillon %.2f deg, vitesse %.1f deg/s ; degres extrapoles sans borne %.1f, avec borne %.1f\n",
                             LacetLu(meilleur.pose), vitesseDegS, vitesseDegS * (double)dtSans, vitesseDegS * (double)dtAvec);
        }

        // ── Diagnostic : latences au-dela de 80 ms (avec prediction bornee) ─────────
        const int extra[5] = {100, 120, 160, 200, 300};
        for (int i = 0; i < 5; ++i) {
                const Stat st = MesurerLatence(extra[i] * 1e-3);
                std::fprintf(stderr, "diag latence %d ms : avec %.4f (max %.4f), sans %.4f (max %.4f) ; horizon moyen %.1f ms, images bornees %d sur %d\n",
                             extra[i], st.avec, st.maxAvec, st.sans, st.maxSans, st.horizonMoy * 1e3, st.bornees, st.n);
        }
        return 0;
}
