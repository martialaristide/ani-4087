#pragma once

typedef float nk_float32;

/** Vecteur a trois composantes, en metres pour une position, sans echelle. */
struct NkVec3f {
    nk_float32 x;
    nk_float32 y;
    nk_float32 z;
};

/** Produit scalaire entre deux vecteurs. */
inline nk_float32 Dot(const NkVec3f& a, const NkVec3f& b) {
    return a.x * b.x + a.y * b.y + a.z * b.z;
}
