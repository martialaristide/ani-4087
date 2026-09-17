#pragma once

#include "NkQuatf.hpp"
#include "NkVec3f.hpp"
#include "Pose.hpp"

/**
 * Applique une pose a un point dans l'ordre inverse de la convention du
 * module : translation d'abord, puis rotation.
 *
 * A comparer avec ApplyPose() (rotation puis translation, Pose.hpp) : ici on
 * additionne d'abord la position au point, puis on fait tourner le resultat
 * par le quaternion.
 */
inline NkVec3f ApplyPoseTranslationFirst(const Pose& pose, const NkVec3f& point) {
    return RotateVectorByQuat(pose.orientation, point + pose.position);
}
