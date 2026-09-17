#pragma once

#include "NkQuatf.hpp"
#include "NkVec3f.hpp"

/** Position (metres) et orientation (quaternion unitaire), jamais d'echelle. */
struct Pose {
    NkVec3f position;
    NkQuatf orientation;
};

/** Applique une pose a un point : rotation puis translation, dans cet ordre. */
inline NkVec3f ApplyPose(const Pose& pose, const NkVec3f& point) {
    return RotateVectorByQuat(pose.orientation, point) + pose.position;
}
