#pragma once

#include "NkQuatf.hpp"
#include "NkVec3f.hpp"

/** Position (metres) et orientation (quaternion unitaire), jamais d'echelle. */
struct Pose {
    NkVec3f position;
    NkQuatf orientation;
};

/**
 * Applique une pose a un point : rotation puis translation, dans cet ordre.
 *
 * L'ordre est fixe par la convention du module (voir exercices 2 et 3) : la
 * rotation s'applique au point exprime dans le repere local de la pose,
 * puis on translate le resultat jusqu'a la position de la pose dans le
 * monde.
 */
inline NkVec3f ApplyPose(const Pose& pose, const NkVec3f& point) {
    return RotateVectorByQuat(pose.orientation, point) + pose.position;
}

/**
 * Calcule la pose inverse : celle qui "defait" exactement ApplyPose().
 *
 * ApplyPose(pose, p) = RotateVectorByQuat(pose.orientation, p) + pose.position.
 * Pour retrouver p a partir de q = ApplyPose(pose, p), il faut annuler les
 * deux operations dans l'ordre inverse :
 *   1. Soustraire la translation      : q - pose.position
 *   2. Annuler la rotation            : RotateVectorByQuat(Conjugate(orientation), ...)
 *
 * D'ou : Inverser(pose) applique a q donne
 *   RotateVectorByQuat(Conjugate(orientation), q - pose.position)
 *       = RotateVectorByQuat(Conjugate(orientation), q) + RotateVectorByQuat(Conjugate(orientation), -pose.position)
 *
 * ce qui est exactement ApplyPose() avec l'orientation Conjugate(orientation)
 * et la position RotateVectorByQuat(Conjugate(orientation), -pose.position).
 * C'est la pose retournee ici.
 *
 * Pourquoi calculer cette formule a la main plutot que passer par une
 * inversion generale de matrice 4x4 : une inversion generique doit gerer le
 * cas degenere (matrice non inversible, ou pres de l'etre) et peut, sur un
 * bug ou une matrice mal conditionnee, renvoyer silencieusement une
 * identite ou un resultat incorrect sans avertir personne. Une pose (rotation
 * unitaire + translation, jamais d'echelle) est toujours inversible par
 * construction : autant utiliser une formule fermee qui ne peut pas echouer
 * silencieusement, plutot qu'un outil generique qui le pourrait.
 */
inline Pose Inverser(const Pose& pose) {
    NkQuatf qInv = Conjugate(pose.orientation);
    NkVec3f pInv = RotateVectorByQuat(qInv, -1.0f * pose.position);
    return Pose{pInv, qInv};
}
