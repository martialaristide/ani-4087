#pragma once

#include "NkQuatf.hpp"
#include "NkVec3f.hpp"

/**
 * Calcule la position monde du coude et de la main par une methode
 * INDEPENDANTE de RotateVectorByQuat/Multiply/Composer : matrices de
 * rotation 3x3 construites explicitement a partir du quaternion (formule
 * standard de conversion), puis multipliees a la main.
 *
 * Sert UNIQUEMENT a verifier Composer() par une voie de calcul differente.
 * Verifier Composer avec des assert qui appellent Composer lui-meme ne
 * prouverait rien de solide (un bug dans Composer biaiserait aussi bien le
 * calcul que sa propre verification) -- deux implementations independantes
 * qui s'accordent sont une bien meilleure preuve. Jamais utilise dans le
 * programme interactif lui-meme.
 */

struct Mat3 {
    nk_float32 m[3][3];
};

/** Conversion quaternion unitaire -> matrice de rotation 3x3 (formule standard). */
inline Mat3 QuatToMat3(const NkQuatf& q) {
    nk_float32 x = q.x, y = q.y, z = q.z, w = q.w;
    Mat3 result;
    result.m[0][0] = 1.0f - 2.0f * (y * y + z * z);
    result.m[0][1] = 2.0f * (x * y - z * w);
    result.m[0][2] = 2.0f * (x * z + y * w);

    result.m[1][0] = 2.0f * (x * y + z * w);
    result.m[1][1] = 1.0f - 2.0f * (x * x + z * z);
    result.m[1][2] = 2.0f * (y * z - x * w);

    result.m[2][0] = 2.0f * (x * z - y * w);
    result.m[2][1] = 2.0f * (y * z + x * w);
    result.m[2][2] = 1.0f - 2.0f * (x * x + y * y);
    return result;
}

inline NkVec3f MulMat3Vec3(const Mat3& m, const NkVec3f& v) {
    return NkVec3f{
        m.m[0][0] * v.x + m.m[0][1] * v.y + m.m[0][2] * v.z,
        m.m[1][0] * v.x + m.m[1][1] * v.y + m.m[1][2] * v.z,
        m.m[2][0] * v.x + m.m[2][1] * v.y + m.m[2][2] * v.z,
    };
}

inline Mat3 MulMat3Mat3(const Mat3& a, const Mat3& b) {
    Mat3 result{};
    for (int i = 0; i < 3; ++i) {
        for (int j = 0; j < 3; ++j) {
            nk_float32 sum = 0.0f;
            for (int k = 0; k < 3; ++k) {
                sum += a.m[i][k] * b.m[k][j];
            }
            result.m[i][j] = sum;
        }
    }
    return result;
}

/**
 * Chaine cinematique de reference (epaule -> coude -> main), calculee
 * entierement en matrices 3x3, pour comparaison avec la chaine de Composer.
 */
inline void ComputeArmReference(const NkQuatf& shoulderOrientation, const NkQuatf& elbowOrientation,
                                 nk_float32 armLength, nk_float32 forearmLength,
                                 NkVec3f& outElbowWorld, NkVec3f& outHandWorld) {
    Mat3 shoulderMat = QuatToMat3(shoulderOrientation);
    Mat3 elbowLocalMat = QuatToMat3(elbowOrientation);
    Mat3 elbowWorldMat = MulMat3Mat3(shoulderMat, elbowLocalMat);

    outElbowWorld = MulMat3Vec3(shoulderMat, NkVec3f{armLength, 0.0f, 0.0f});
    outHandWorld = outElbowWorld + MulMat3Vec3(elbowWorldMat, NkVec3f{forearmLength, 0.0f, 0.0f});
}
