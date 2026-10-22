// La projection a la main : la matrice de projection decentree (quatre angles), ecrite a partir
// de la matrice nulle, comparee a celles du moteur.
//   epreuve symetrique : 70 degres verticalement, rapport 1280/720, plans 0,05 et 100 m
//                        -> comparee a NkMat4f::Perspective
//   epreuve quest      : oeil gauche d'un Quest 2, gauche -52, droite 45, haut 48, bas -50 degres
//                        -> comparee a NkCamera3D (SetNearFar, SetFovAsym, GetProj)
// NkMat4f range ses coefficients par colonne : mat[colonne][ligne].
//
// Sortie standard : les huit lignes d'ecarts puis le plus grand ecart de chaque epreuve.
// Sortie d'erreur : les deux matrices de chaque epreuve (mienne, moteur), ligne par ligne.

#include <cmath>
#include <cstdio>

#include "NKPlatform/NkPlatformDetect.h"
#include "NKWindow/NKMain.h"
#include "NKMath/NkAngle.h"
#include "NKMath/NkMat.h"
#include "NKRenderer/Core/NkCamera.h"

using namespace nkentseu;
using namespace nkentseu::math;
using namespace nkentseu::renderer;

static void ConfigureAppData(NkAppData &d) {
        d.appName = "LaProjectionALaMain";
}
NK_REGISTER_ENTRY_APPDATA_UPDATER(ConfigureAppData)

namespace {
        const float kPi = 3.14159265358979f;

        float EnRadians(float degres) { return degres * kPi / 180.0f; }

        // La matrice de projection decentree. Angles en radians, signes : gauche et bas negatifs.
        NkMat4f Projection(float gauche, float droite, float haut, float bas, float nearPlane, float farPlane) {
                const float tanLeft = std::tan(gauche);
                const float tanRight = std::tan(droite);
                const float tanUp = std::tan(haut);
                const float tanDown = std::tan(bas);
                const float tanWidth = tanRight - tanLeft;
                const float tanHeight = tanUp - tanDown;

                NkMat4f result; // on part de la matrice NULLE : on remet tout a zero explicitement
                for (int c = 0; c < 4; ++c)
                        for (int l = 0; l < 4; ++l)
                                result.mat[c][l] = 0.0f;

                result.mat[0][0] = 2.0f / tanWidth;
                result.mat[1][1] = 2.0f / tanHeight;
                result.mat[2][0] = (tanRight + tanLeft) / tanWidth;   // decentrage horizontal
                result.mat[2][1] = (tanUp + tanDown) / tanHeight;     // decentrage vertical
                result.mat[2][3] = -1.0f;
                result.mat[2][2] = (farPlane + nearPlane) / (nearPlane - farPlane);
                result.mat[3][2] = 2.0f * farPlane * nearPlane / (nearPlane - farPlane);
                return result;
        }

        void ImprimerMatrice(const char *nom, const NkMat4f &m) {
                for (int l = 0; l < 4; ++l)
                        std::fprintf(stderr, "%s ligne %d : %12.6f %12.6f %12.6f %12.6f\n", nom, l, m.mat[0][l],
                                     m.mat[1][l], m.mat[2][l], m.mat[3][l]);
        }

        // Ecarts coefficient par coefficient : une ligne imprimee par ligne de matrice.
        float ImprimerEcarts(const char *epreuve, const NkMat4f &a, const NkMat4f &b) {
                float plusGrand = 0.0f;
                for (int l = 0; l < 4; ++l) {
                        float e[4];
                        for (int c = 0; c < 4; ++c) {
                                e[c] = std::fabs(a.mat[c][l] - b.mat[c][l]);
                                if (e[c] > plusGrand) plusGrand = e[c];
                        }
                        std::printf("%s, ligne %d : %.3e %.3e %.3e %.3e\n", epreuve, l, e[0], e[1], e[2], e[3]);
                }
                return plusGrand;
        }
}

int nkmain(const NkEntryState &state) {
        (void)state;
        const float nearPlane = 0.05f;
        const float farPlane = 100.0f;

        // ── Epreuve symetrique : les quatre angles se deduisent de 70 degres et du rapport ──
        const float aspect = 1280.0f / 720.0f;
        const float demiHaut = EnRadians(70.0f) * 0.5f;          // 35 degres
        const float tanHaut = std::tan(demiHaut);
        const float haut = demiHaut;
        const float bas = -demiHaut;
        const float droite = std::atan(tanHaut * aspect);        // tan(droite) = tan(haut) * rapport
        const float gauche = -droite;

        const NkMat4f maSym = Projection(gauche, droite, haut, bas, nearPlane, farPlane);
        const NkMat4f moteurSym = NkMat4f::Perspective(NkAngle(70.0f), aspect, nearPlane, farPlane);

        // ── Epreuve quest : l'oeil gauche releve sur un Quest 2 ──
        const float qGauche = EnRadians(-52.0f);
        const float qDroite = EnRadians(45.0f);
        const float qHaut = EnRadians(48.0f);
        const float qBas = EnRadians(-50.0f);

        const NkMat4f maQuest = Projection(qGauche, qDroite, qHaut, qBas, nearPlane, farPlane);
        NkCamera3D cam;
        cam.SetNearFar(nearPlane, farPlane);
        cam.SetFovAsym(qGauche, qDroite, qHaut, qBas);
        const NkMat4f moteurQuest = cam.GetProj();

        // ── Sorties ──
        std::fprintf(stderr, "angles symetriques (rad) : gauche %.6f droite %.6f haut %.6f bas %.6f\n", gauche,
                     droite, haut, bas);
        ImprimerMatrice("symetrique mienne ", maSym);
        ImprimerMatrice("symetrique moteur ", moteurSym);
        ImprimerMatrice("quest mienne      ", maQuest);
        ImprimerMatrice("quest moteur      ", moteurQuest);

        const float eSym = ImprimerEcarts("symetrique", maSym, moteurSym);
        const float eQuest = ImprimerEcarts("quest", maQuest, moteurQuest);
        std::printf("symetrique : plus grand ecart %.3e\n", eSym);
        std::printf("quest : plus grand ecart %.3e\n", eQuest);
        std::fflush(stdout);
        return 0;
}
