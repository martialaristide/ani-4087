#pragma once

#include "NkVec3f.hpp"

/** Quaternion (x, y, z, w), suppose unitaire par l'appelant. */
struct NkQuatf {
    nk_float32 x;
    nk_float32 y;
    nk_float32 z;
    nk_float32 w;
};

/**
 * Applique la rotation du quaternion unitaire q au vecteur v.
 *
 * Formule vectorielle (evite un produit de quaternions complet) :
 *   t  = 2 * (q.xyz x v)
 *   v' = v + q.w * t + (q.xyz x t)
 */
inline NkVec3f RotateVectorByQuat(const NkQuatf& q, const NkVec3f& v) {
    NkVec3f qxyz{q.x, q.y, q.z};
    NkVec3f t = 2.0f * Cross(qxyz, v);
    return v + (q.w * t) + Cross(qxyz, t);
}

/**
 * Conjugue d'un quaternion unitaire : (-x, -y, -z, w).
 *
 * Pour un quaternion unitaire, le conjugue est aussi son inverse : il
 * represente la rotation opposee (meme axe, angle oppose). C'est la piece
 * qui permet de "defaire" une rotation sans repasser par une inversion
 * generale de matrice.
 */
inline NkQuatf Conjugate(const NkQuatf& q) {
    return NkQuatf{-q.x, -q.y, -q.z, q.w};
}
