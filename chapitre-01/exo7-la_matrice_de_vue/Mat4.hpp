#pragma once

#include "NkQuatf.hpp"
#include "NkVec3f.hpp"
#include "Pose.hpp"

#include <cmath>

/** Matrice 4x4, m[ligne][colonne]. */
struct Mat4 {
    nk_float32 m[4][4];
};

inline Mat4 Mat4Identity() {
    return Mat4{{
        {1.0f, 0.0f, 0.0f, 0.0f},
        {0.0f, 1.0f, 0.0f, 0.0f},
        {0.0f, 0.0f, 1.0f, 0.0f},
        {0.0f, 0.0f, 0.0f, 1.0f},
    }};
}

/**
 * Convertit une pose en matrice 4x4 : bloc rotation 3x3 derive du
 * quaternion (formule standard), position en derniere colonne, derniere
 * ligne (0,0,0,1).
 */
inline Mat4 PoseToMatrix(const Pose& pose) {
    nk_float32 x = pose.orientation.x, y = pose.orientation.y, z = pose.orientation.z,
               w = pose.orientation.w;
    Mat4 result{};
    result.m[0][0] = 1.0f - 2.0f * (y * y + z * z);
    result.m[0][1] = 2.0f * (x * y - w * z);
    result.m[0][2] = 2.0f * (x * z + w * y);
    result.m[0][3] = pose.position.x;

    result.m[1][0] = 2.0f * (x * y + w * z);
    result.m[1][1] = 1.0f - 2.0f * (x * x + z * z);
    result.m[1][2] = 2.0f * (y * z - w * x);
    result.m[1][3] = pose.position.y;

    result.m[2][0] = 2.0f * (x * z - w * y);
    result.m[2][1] = 2.0f * (y * z + w * x);
    result.m[2][2] = 1.0f - 2.0f * (x * x + y * y);
    result.m[2][3] = pose.position.z;

    result.m[3][0] = 0.0f;
    result.m[3][1] = 0.0f;
    result.m[3][2] = 0.0f;
    result.m[3][3] = 1.0f;
    return result;
}

namespace detail {

inline nk_float32 Det3(nk_float32 a, nk_float32 b, nk_float32 c, nk_float32 d, nk_float32 e,
                       nk_float32 f, nk_float32 g, nk_float32 h, nk_float32 i) {
    return a * (e * i - f * h) - b * (d * i - f * g) + c * (d * h - e * g);
}

// Determinant du mineur 3x3 obtenu en retirant la ligne skipRow et la
// colonne skipCol de m.
inline nk_float32 Minor3x3(const Mat4& m, int skipRow, int skipCol) {
    nk_float32 v[9];
    int idx = 0;
    for (int r = 0; r < 4; ++r) {
        if (r == skipRow) continue;
        for (int c = 0; c < 4; ++c) {
            if (c == skipCol) continue;
            v[idx++] = m.m[r][c];
        }
    }
    return Det3(v[0], v[1], v[2], v[3], v[4], v[5], v[6], v[7], v[8]);
}

inline nk_float32 Determinant4(const Mat4& m) {
    nk_float32 det = 0.0f;
    for (int c = 0; c < 4; ++c) {
        nk_float32 sign = (c % 2 == 0) ? 1.0f : -1.0f;
        det += sign * m.m[0][c] * Minor3x3(m, 0, c);
    }
    return det;
}

}  // namespace detail

/** Determinant de la matrice 4x4 (expose pour diagnostic/affichage). */
inline nk_float32 ComputeDeterminant4(const Mat4& m) {
    return detail::Determinant4(m);
}

/** Colonne d'une matrice 4x4 (sert a extraire origine/axes pour l'affichage). */
inline NkVec3f MatColumn3(const Mat4& m, int col) {
    return NkVec3f{m.m[0][col], m.m[1][col], m.m[2][col]};
}

/**
 * Inversion 4x4 GENERALE, par cofacteurs/adjugate -- la methode qu'on
 * utiliserait pour n'importe quelle matrice, sans savoir qu'il s'agit
 * d'une pose.
 *
 * ATTENTION -- DEFAUT REPRODUIT A DESSEIN, PAS UNE BONNE PRATIQUE :
 * si le determinant est proche de zero (matrice singuliere, ce qui ne
 * devrait jamais se produire pour une VRAIE pose valide mais peut arriver
 * si un bug en amont a corrompu la matrice), cette fonction renvoie
 * l'identite SANS AUCUNE ERREUR, SANS AVERTISSEMENT. C'est exactement le
 * "mensonge silencieux" que le chapitre 2 denonce a propos de l'inversion
 * generique : une matrice manifestement fausse (determinant nul) produit
 * un resultat qui a l'air parfaitement normal (l'identite), sans jamais
 * signaler que l'entree etait invalide. Voir InvertAnalytical() pour la
 * methode que le module utilise reellement, qui ne peut pas tomber dans ce
 * piege puisqu'elle ne passe jamais par une inversion generique.
 */
inline Mat4 InvertGeneral(const Mat4& m) {
    constexpr nk_float32 kSingularEpsilon = 1e-6f;
    nk_float32 det = detail::Determinant4(m);

    if (std::fabs(det) < kSingularEpsilon) {
        return Mat4Identity();  // <-- le mensonge silencieux, reproduit a dessein.
    }

    Mat4 result{};
    for (int r = 0; r < 4; ++r) {
        for (int c = 0; c < 4; ++c) {
            nk_float32 sign = ((r + c) % 2 == 0) ? 1.0f : -1.0f;
            nk_float32 cofactor = sign * detail::Minor3x3(m, r, c);
            // adjugate[c][r] = cofactor[r][c] ; inverse[c][r] = adjugate[c][r] / det.
            result.m[c][r] = cofactor / det;
        }
    }
    return result;
}

/**
 * Inversion ANALYTIQUE d'une pose : construit directement le conjugue du
 * quaternion et la position opposee tournee par ce conjugue (formule de
 * Inverser(), exercice 4), assemblee ici en Mat4.
 *
 * Une pose (rotation unitaire + translation, jamais d'echelle) est
 * TOUJOURS inversible par construction -- il n'existe pas de pose valide
 * degeneree. Cette fonction ne peut donc jamais avoir a gerer un cas
 * singulier : contrairement a InvertGeneral(), elle ne peut pas masquer
 * silencieusement un bug, puisqu'elle ne verifie jamais un determinant --
 * elle n'en a simplement pas besoin.
 */
inline Mat4 InvertAnalytical(const Pose& pose) {
    NkQuatf qInv = Conjugate(pose.orientation);
    NkVec3f pInv = RotateVectorByQuat(qInv, -1.0f * pose.position);
    return PoseToMatrix(Pose{pInv, qInv});
}
