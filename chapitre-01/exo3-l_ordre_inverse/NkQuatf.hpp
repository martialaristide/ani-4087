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
    NkVec3f t = Cross(qxyz, v) * 2.0f;
    return v + (t * q.w) + Cross(qxyz, t);
}
