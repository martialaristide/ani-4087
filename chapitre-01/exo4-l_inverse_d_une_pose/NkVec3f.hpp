#pragma once

#include <cmath>

typedef float nk_float32;

/** Vecteur a trois composantes, en metres pour une position, sans echelle. */
struct NkVec3f {
    nk_float32 x;
    nk_float32 y;
    nk_float32 z;
};

inline NkVec3f operator+(const NkVec3f& a, const NkVec3f& b) {
    return NkVec3f{a.x + b.x, a.y + b.y, a.z + b.z};
}

inline NkVec3f operator-(const NkVec3f& a, const NkVec3f& b) {
    return NkVec3f{a.x - b.x, a.y - b.y, a.z - b.z};
}

inline NkVec3f operator*(nk_float32 s, const NkVec3f& v) {
    return NkVec3f{s * v.x, s * v.y, s * v.z};
}

inline NkVec3f operator*(const NkVec3f& v, nk_float32 s) {
    return s * v;
}

/** Produit vectoriel entre deux vecteurs. */
inline NkVec3f Cross(const NkVec3f& a, const NkVec3f& b) {
    return NkVec3f{
        a.y * b.z - a.z * b.y,
        a.z * b.x - a.x * b.z,
        a.x * b.y - a.y * b.x,
    };
}

/** Norme euclidienne du vecteur. */
inline nk_float32 Length(const NkVec3f& v) {
    return std::sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
}
