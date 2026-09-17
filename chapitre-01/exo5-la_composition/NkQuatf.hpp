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
 * represente la rotation opposee (meme axe, angle oppose).
 */
inline NkQuatf Conjugate(const NkQuatf& q) {
    return NkQuatf{-q.x, -q.y, -q.z, q.w};
}

/**
 * Produit de deux quaternions (produit de Hamilton) : combine deux
 * rotations en une seule.
 *
 * A ne pas confondre avec RotateVectorByQuat(), qui fait tourner un
 * VECTEUR. Multiply() combine deux ROTATIONS entre elles : le quaternion
 * resultant applique a un vecteur donne le meme resultat que d'appliquer
 * d'abord la rotation b, puis la rotation a
 * (RotateVectorByQuat(Multiply(a, b), v) == RotateVectorByQuat(a,
 * RotateVectorByQuat(b, v))). C'est l'ingredient qui permet de composer des
 * poses (voir Composer() dans Composer.hpp) sans jamais avoir a repasser
 * par une matrice.
 */
inline NkQuatf Multiply(const NkQuatf& a, const NkQuatf& b) {
    return NkQuatf{
        a.w * b.x + a.x * b.w + a.y * b.z - a.z * b.y,
        a.w * b.y - a.x * b.z + a.y * b.w + a.z * b.x,
        a.w * b.z + a.x * b.y - a.y * b.x + a.z * b.w,
        a.w * b.w - a.x * b.x - a.y * b.y - a.z * b.z,
    };
}
