#pragma once

#include "NkQuatf.hpp"
#include "NkVec3f.hpp"
#include "Pose.hpp"

#include <cmath>

/** Avance une position de dt secondes a vitesse lineaire constante. */
inline NkVec3f ExtrapolatePosition(const NkVec3f& pos, const NkVec3f& linVel, nk_float32 dt) {
    return pos + dt * linVel;
}

/**
 * Avance une orientation de dt secondes a vitesse angulaire constante.
 *
 * Une vitesse angulaire est un vecteur dont la DIRECTION est l'axe de
 * rotation et la NORME la vitesse de rotation en rad/s. Pour extrapoler :
 * normaliser le vecteur pour obtenir l'axe, multiplier sa norme par dt
 * pour obtenir l'angle balaye sur cette duree, construire le quaternion
 * delta correspondant (formule axe-angle standard), et le composer avec
 * l'orientation actuelle.
 *
 * Cas particulier obligatoire : si la norme de la vitesse angulaire est
 * proche de zero, il ne faut PAS essayer de la normaliser -- diviser un
 * vecteur nul par sa propre norme (nulle) produit une division par zero et
 * un axe NaN, qui contaminerait silencieusement tout calcul en aval. Une
 * vitesse angulaire nulle signifie simplement "pas de rotation" : on
 * retourne l'orientation inchangee, sans passer par la formule axe-angle
 * du tout.
 *
 * Convention de repere (l'enonce ne la precise pas, choix documente ici) :
 * la vitesse angulaire est exprimee dans le repere du MONDE, pas dans le
 * repere local de la pose. Le quaternion delta est donc multiplie a
 * GAUCHE de l'orientation actuelle (Multiply(delta, q), et non
 * Multiply(q, delta), qui aurait applique la rotation dans le repere
 * local de la pose plutot que dans le monde).
 */
inline NkQuatf ExtrapolateOrientation(const NkQuatf& q, const NkVec3f& angVel, nk_float32 dt) {
    nk_float32 speed = Length(angVel);
    if (speed < 1e-6f) {
        return q;  // vitesse angulaire nulle : pas de rotation, evite la division par zero.
    }
    NkVec3f axis = (1.0f / speed) * angVel;
    nk_float32 angle = speed * dt;
    nk_float32 half = angle * 0.5f;
    nk_float32 s = std::sin(half);
    NkQuatf delta{axis.x * s, axis.y * s, axis.z * s, std::cos(half)};
    return Multiply(delta, q);  // delta appliquee en repere monde.
}

/** Avance une pose complete (position + orientation) de dt secondes. */
inline Pose ExtrapolatePose(const Pose& pose, const NkVec3f& linVel, const NkVec3f& angVel,
                            nk_float32 dt) {
    return Pose{ExtrapolatePosition(pose.position, linVel, dt),
                ExtrapolateOrientation(pose.orientation, angVel, dt)};
}
