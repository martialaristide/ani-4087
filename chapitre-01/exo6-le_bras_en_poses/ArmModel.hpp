#pragma once

#include "Composer.hpp"
#include "NkQuatf.hpp"
#include "NkVec3f.hpp"
#include "Pose.hpp"

/** Longueur epaule -> coude, en metres (constante du modele, pas une entree). */
constexpr nk_float32 kArmLength = 0.35f;
/** Longueur coude -> main, en metres (constante du modele, pas une entree). */
constexpr nk_float32 kForearmLength = 0.30f;

struct ArmPoses {
    Pose shoulderWorld;
    Pose elbowWorld;
    Pose handWorld;
};

/**
 * Calcule la chaine cinematique epaule -> coude -> main par composition de
 * poses (chemin "officiel", partage par le programme interactif et la
 * suite de tests) : chaque articulation compose la pose monde de son
 * parent avec sa propre pose locale.
 */
inline ArmPoses ComputeArm(const NkQuatf& shoulderOrientation, const NkQuatf& elbowOrientation,
                           const NkQuatf& handOrientation) {
    Pose shoulderWorld{NkVec3f{0.0f, 0.0f, 0.0f}, shoulderOrientation};
    Pose elbowLocal{NkVec3f{kArmLength, 0.0f, 0.0f}, elbowOrientation};
    Pose handLocal{NkVec3f{kForearmLength, 0.0f, 0.0f}, handOrientation};

    Pose elbowWorld = Composer(shoulderWorld, elbowLocal);
    Pose handWorld = Composer(elbowWorld, handLocal);

    return ArmPoses{shoulderWorld, elbowWorld, handWorld};
}
