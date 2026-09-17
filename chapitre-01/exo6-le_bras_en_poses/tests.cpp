// Suite de tests du bras articule -- separee du programme interactif,
// aucune dependance a GLFW. Compilee et lancee via `make test`.
//
// Un assert() qui echoue arrete immediatement le binaire avec un message
// clair (l'expression testee + le commentaire attache) et un code de
// sortie non nul -- suffisant pour designer sans ambiguite quel test a
// casse, sans avoir a tout revalider a la main.

#include "ArmModel.hpp"
#include "Composer.hpp"
#include "NkQuatf.hpp"
#include "NkVec3f.hpp"
#include "Pose.hpp"
#include "ReferenceKinematics.hpp"

#include <cassert>
#include <cmath>
#include <cstdio>
#include <random>

namespace {

constexpr nk_float32 kTolerance = 1e-3f;
constexpr nk_float32 kTightTolerance = 1e-4f;

NkQuatf QuatFromAxisAngle(NkVec3f axis, float angleRad) {
    float len = Length(axis);
    if (len > 1e-8f) {
        axis = axis * (1.0f / len);
    }
    float half = angleRad * 0.5f;
    float s = std::sin(half);
    return NkQuatf{axis.x * s, axis.y * s, axis.z * s, std::cos(half)};
}

void TestSansRotation() {
    const NkQuatf identity{0.0f, 0.0f, 0.0f, 1.0f};
    const ArmPoses arm = ComputeArm(identity, identity, identity);

    assert(Length(arm.elbowWorld.position - NkVec3f{0.35f, 0.0f, 0.0f}) < kTightTolerance &&
           "Test 'sans rotation' : position du coude incorrecte");
    assert(Length(arm.handWorld.position - NkVec3f{0.65f, 0.0f, 0.0f}) < kTightTolerance &&
           "Test 'sans rotation' : position de la main incorrecte");

    NkVec3f refElbow, refHand;
    ComputeArmReference(identity, identity, kArmLength, kForearmLength, refElbow, refHand);
    assert(Length(arm.elbowWorld.position - refElbow) < kTolerance &&
           "Test 'sans rotation' : desaccord avec la methode matricielle (coude)");
    assert(Length(arm.handWorld.position - refHand) < kTolerance &&
           "Test 'sans rotation' : desaccord avec la methode matricielle (main)");
}

void TestEpauleA90Degres() {
    const NkQuatf shoulder{0.0f, 0.0f, 0.7071068f, 0.7071068f};
    const NkQuatf identity{0.0f, 0.0f, 0.0f, 1.0f};
    const ArmPoses arm = ComputeArm(shoulder, identity, identity);

    assert(Length(arm.elbowWorld.position - NkVec3f{0.0f, 0.35f, 0.0f}) < kTightTolerance &&
           "Test 'epaule a 90 deg' : position du coude incorrecte");
    assert(Length(arm.handWorld.position - NkVec3f{0.0f, 0.65f, 0.0f}) < kTightTolerance &&
           "Test 'epaule a 90 deg' : position de la main incorrecte");

    NkVec3f refElbow, refHand;
    ComputeArmReference(shoulder, identity, kArmLength, kForearmLength, refElbow, refHand);
    assert(Length(arm.elbowWorld.position - refElbow) < kTolerance &&
           "Test 'epaule a 90 deg' : desaccord avec la methode matricielle (coude)");
    assert(Length(arm.handWorld.position - refHand) < kTolerance &&
           "Test 'epaule a 90 deg' : desaccord avec la methode matricielle (main)");
}

void TestRotationSimultanee() {
    const NkQuatf shoulder = QuatFromAxisAngle(NkVec3f{0.0f, 1.0f, 0.0f}, 0.7854f);   // 45 deg / Y
    const NkQuatf elbow = QuatFromAxisAngle(NkVec3f{1.0f, 0.0f, 0.0f}, 0.5236f);      // 30 deg / X
    const NkQuatf hand = QuatFromAxisAngle(NkVec3f{0.0f, 0.0f, 1.0f}, 1.0472f);       // 60 deg / Z (n'affecte pas la position)

    const ArmPoses arm = ComputeArm(shoulder, elbow, hand);

    NkVec3f refElbow, refHand;
    ComputeArmReference(shoulder, elbow, kArmLength, kForearmLength, refElbow, refHand);

    assert(Length(arm.elbowWorld.position - refElbow) < kTolerance &&
           "Test 'rotation simultanee' : desaccord avec la methode matricielle (coude)");
    assert(Length(arm.handWorld.position - refHand) < kTolerance &&
           "Test 'rotation simultanee' : desaccord avec la methode matricielle (main)");
}

// Composer(Composer(A,B),C) doit appliquer un point exactement comme
// Composer(A,Composer(B,C)) -- propriete d'associativite de la
// composition de poses. On compare l'effet sur un point plutot que les
// quaternions bruts pour eviter l'ambiguite de signe (q et -q representent
// la meme rotation).
void TestAssociativite() {
    const Pose a{NkVec3f{1.0f, 0.0f, 0.0f}, QuatFromAxisAngle(NkVec3f{0, 0, 1}, 0.6f)};
    const Pose b{NkVec3f{0.0f, 0.5f, 0.2f}, QuatFromAxisAngle(NkVec3f{1, 0, 0}, 1.1f)};
    const Pose c{NkVec3f{-0.3f, 0.1f, 0.4f}, QuatFromAxisAngle(NkVec3f{0, 1, 0}, 0.9f)};
    const NkVec3f testPoint{0.7f, -0.4f, 0.2f};

    const Pose left = Composer(Composer(a, b), c);
    const Pose right = Composer(a, Composer(b, c));

    const NkVec3f pointLeft = ApplyPose(left, testPoint);
    const NkVec3f pointRight = ApplyPose(right, testPoint);

    assert(Length(pointLeft - pointRight) < kTolerance &&
           "Test 'associativite' : Composer((A,B),C) != Composer(A,(B,C))");
}

// Inegalite triangulaire : la main ne doit jamais s'eloigner de l'epaule de
// plus de ARM_LENGTH+FOREARM_LENGTH, quelle que soit l'orientation. Egalite
// exacte quand le bras est aligne (deja verifie par TestSansRotation).
void TestInegaliteTriangulaire() {
    constexpr float kMaxReach = kArmLength + kForearmLength;

    assert(std::fabs(0.65f - kMaxReach) < kTightTolerance &&
           "Test 'inegalite triangulaire' : ARM_LENGTH+FOREARM_LENGTH inattendu");

    std::mt19937 rng(42);  // graine fixe : reproductible
    std::uniform_real_distribution<float> axisDist(-1.0f, 1.0f);
    std::uniform_real_distribution<float> angleDist(0.0f, 6.2831853f);

    constexpr int kTrials = 30;
    for (int i = 0; i < kTrials; ++i) {
        NkVec3f shoulderAxis{axisDist(rng), axisDist(rng), axisDist(rng)};
        NkVec3f elbowAxis{axisDist(rng), axisDist(rng), axisDist(rng)};
        NkQuatf shoulder = QuatFromAxisAngle(shoulderAxis, angleDist(rng));
        NkQuatf elbow = QuatFromAxisAngle(elbowAxis, angleDist(rng));

        const ArmPoses arm = ComputeArm(shoulder, elbow, NkQuatf{0, 0, 0, 1});
        float distance = Length(arm.handWorld.position - arm.shoulderWorld.position);

        assert(distance <= kMaxReach + kTightTolerance &&
               "Test 'inegalite triangulaire' : la main a depasse la portee maximale");
    }
}

}  // namespace

int main() {
    TestSansRotation();
    TestEpauleA90Degres();
    TestRotationSimultanee();
    TestAssociativite();
    TestInegaliteTriangulaire();

    std::printf("Tous les tests sont passes.\n");
    return 0;
}
