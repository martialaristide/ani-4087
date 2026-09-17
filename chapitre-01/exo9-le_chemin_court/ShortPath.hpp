#pragma once

#include "NkQuatf.hpp"
#include "NkVec3f.hpp"

#include <algorithm>
#include <cmath>

/**
 * Vitesse angulaire moyenne pour passer de q1 a q2 en dt secondes.
 *
 * Sur les quaternions, on suit toujours le chemin court : de 350 deg a
 * 10 deg, on peut passer par 0 deg en 20 deg (le chemin court), ou par
 * 180 deg en 340 deg (le chemin long). Une soustraction naive de rotation
 * donne le chemin long. La raison profonde : un quaternion q et son
 * oppose -q representent EXACTEMENT la meme orientation (double
 * revetement de SO(3)) -- rien n'empeche deux orientations tres proches
 * en realite d'etre representees par des quaternions pris sur des
 * hemispheres opposes de ce double revetement, avec un produit scalaire
 * negatif. Dans ce cas, calculer la rotation "de q1 vers q2" sans y
 * preter attention emprunte le grand tour autour de la sphere plutot que
 * le petit, alors que les deux orientations sont presque identiques.
 *
 * @param forceShortPath si vrai, force le chemin court en repliant q2 sur
 *        le meme hemisphere que q1 (via Negate()) quand leur produit
 *        scalaire est negatif -- cela ne change PAS l'orientation
 *        representee par q2 (q et -q sont la meme orientation), seulement
 *        sa representation, pour que le calcul de delta emprunte le petit
 *        arc plutot que le grand.
 */
inline NkVec3f AverageAngularVelocity(const NkQuatf& q1, const NkQuatf& q2In, nk_float32 dt,
                                       bool forceShortPath) {
    NkQuatf q2 = q2In;
    if (forceShortPath && Dot(q1, q2) < 0.0f) {
        q2 = Negate(q2);  // meme orientation, chemin court garanti.
    }

    NkQuatf delta = Multiply(q2, Conjugate(q1));  // rotation de q1 vers q2.
    nk_float32 w = std::fmax(-1.0f, std::fmin(1.0f, delta.w));  // clamp, securite pour acos.
    nk_float32 angle = 2.0f * std::acos(w);
    nk_float32 s = std::sqrt(1.0f - w * w);
    NkVec3f axis =
        (s < 1e-6f) ? NkVec3f{0.0f, 0.0f, 0.0f} : NkVec3f{delta.x / s, delta.y / s, delta.z / s};
    return (angle / dt) * axis;
}
