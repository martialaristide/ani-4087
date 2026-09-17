// Chapitre 2, exercice 6 : "Le bras en poses"
//
// Bras articule a trois segments (epaule -> coude -> main), chaque
// articulation composant la pose monde de son parent avec sa propre pose
// locale (voir ArmModel.hpp). Sortie console = contrat fixe calcule une
// fois a partir de l'entree ; la fenetre ajoute une manipulation
// interactive de l'epaule et du coude par-dessus, a titre de demonstration
// visuelle -- elle ne modifie jamais ce qui a ete imprime.
//
// Note sur le cercle de portee affiche : c'est le grand cercle de rayon
// (ARM_LENGTH+FOREARM_LENGTH) dans le plan z=0 autour de l'epaule, projete
// en isometrique -- pas le contour exact d'une sphere 3D vue sous tous les
// angles. Comme les controles interactifs (rotation autour de Z) gardent le
// bras dans ce plan, c'est une limite fidele et lisible a l'ecran pour cette
// demonstration. La preuve rigoureuse, valable pour TOUTE orientation en
// 3D, est le test d'inegalite triangulaire de tests.cpp (make test).

#include "ArmModel.hpp"
#include "Composer.hpp"
#include "NkQuatf.hpp"
#include "NkVec3f.hpp"
#include "Pose.hpp"

#include <GLFW/glfw3.h>

#include <chrono>
#include <cmath>
#include <cstdio>
#include <cstdlib>

namespace {

constexpr int kWindowWidth = 900;
constexpr int kWindowHeight = 700;
constexpr float kOriginX = kWindowWidth / 2.0f;
constexpr float kOriginY = kWindowHeight / 2.0f;
constexpr float kScale = 350.0f;  // pixels par metre (bras ~0.65 m, agrandi pour rester lisible)

constexpr float kWorldAxisLength = 0.2f;
constexpr float kReachRadius = kArmLength + kForearmLength;
constexpr float kAngularSpeedRadPerSec = 1.3f;  // vitesse de rotation interactive

using Clock = std::chrono::high_resolution_clock;

void WorldToScreen(const NkVec3f& world, float& screenX, float& screenY) {
    screenX = kOriginX + (world.x - world.z) * kScale;
    screenY = kOriginY + (world.y - (world.x + world.z) * 0.5f) * kScale;
}

NkQuatf QuatFromAxisAngleZ(float angleRad) {
    float half = angleRad * 0.5f;
    return NkQuatf{0.0f, 0.0f, std::sin(half), std::cos(half)};
}

void DrawCircle(float cx, float cy, float radius, float r, float g, float b, float a) {
    constexpr int kSegments = 24;
    glColor4f(r, g, b, a);
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(cx, cy);
    for (int i = 0; i <= kSegments; ++i) {
        float theta = 2.0f * 3.14159265f * static_cast<float>(i) / kSegments;
        glVertex2f(cx + radius * std::cos(theta), cy + radius * std::sin(theta));
    }
    glEnd();
}

void DrawWorldSegment(const NkVec3f& from, const NkVec3f& to, float r, float g, float b,
                      float a) {
    float x0, y0, x1, y1;
    WorldToScreen(from, x0, y0);
    WorldToScreen(to, x1, y1);
    glColor4f(r, g, b, a);
    glBegin(GL_LINES);
    glVertex2f(x0, y0);
    glVertex2f(x1, y1);
    glEnd();
}

void DrawAxes(const NkVec3f& origin, float length) {
    DrawWorldSegment(origin, origin + NkVec3f{length, 0, 0}, 0.9f, 0.2f, 0.2f, 1.0f);
    DrawWorldSegment(origin, origin + NkVec3f{0, length, 0}, 0.2f, 0.85f, 0.3f, 1.0f);
    DrawWorldSegment(origin, origin + NkVec3f{0, 0, length}, 0.25f, 0.45f, 0.95f, 1.0f);
}

// Grand cercle de rayon kReachRadius dans le plan z=0, projete en
// isometrique (voir note en tete de fichier).
void DrawReachCircle(const NkVec3f& center) {
    constexpr int kSegments = 96;
    glColor4f(0.6f, 0.6f, 0.6f, 0.5f);
    glBegin(GL_LINE_LOOP);
    for (int i = 0; i < kSegments; ++i) {
        float theta = 2.0f * 3.14159265f * static_cast<float>(i) / kSegments;
        NkVec3f p = center + NkVec3f{kReachRadius * std::cos(theta), kReachRadius * std::sin(theta), 0.0f};
        float sx, sy;
        WorldToScreen(p, sx, sy);
        glVertex2f(sx, sy);
    }
    glEnd();
}

}  // namespace

int main() {
    NkQuatf shoulderOrientation{};
    NkQuatf elbowOrientation{};
    NkQuatf handOrientation{};

    int read = std::scanf("%f %f %f %f %f %f %f %f %f %f %f %f", &shoulderOrientation.x,
                           &shoulderOrientation.y, &shoulderOrientation.z, &shoulderOrientation.w,
                           &elbowOrientation.x, &elbowOrientation.y, &elbowOrientation.z,
                           &elbowOrientation.w, &handOrientation.x, &handOrientation.y,
                           &handOrientation.z, &handOrientation.w);
    if (read != 12) {
        std::fprintf(stderr,
                      "Usage : douze nombres reels separes par des espaces\n"
                      "  quaternion epaule (x y z w)\n"
                      "  quaternion coude local (x y z w)\n"
                      "  quaternion main locale (x y z w)\n"
                      "  ex: 0 0 0 1   0 0 0 1   0 0 0 1\n");
        return EXIT_FAILURE;
    }

    const ArmPoses arm = ComputeArm(shoulderOrientation, elbowOrientation, handOrientation);

    std::printf("%.4f\n", arm.elbowWorld.position.x);
    std::printf("%.4f\n", arm.elbowWorld.position.y);
    std::printf("%.4f\n", arm.elbowWorld.position.z);
    std::printf("%.4f\n", arm.handWorld.position.x);
    std::printf("%.4f\n", arm.handWorld.position.y);
    std::printf("%.4f\n", arm.handWorld.position.z);
    std::fflush(stdout);

    if (!glfwInit()) {
        std::fprintf(stderr, "Erreur : impossible d'initialiser GLFW.\n");
        return 1;
    }

    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    GLFWwindow* window = glfwCreateWindow(kWindowWidth, kWindowHeight,
                                           "Exo 6 (ch2) - Le bras en poses", nullptr, nullptr);
    if (!window) {
        std::fprintf(stderr, "Erreur : impossible de creer la fenetre.\n");
        glfwTerminate();
        return 1;
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    glViewport(0, 0, kWindowWidth, kWindowHeight);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0.0, kWindowWidth, 0.0, kWindowHeight, -1.0, 1.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glLineWidth(2.0f);

    // Angles interactifs, ajoutes par-dessus l'orientation lue en entree.
    // Fleches gauche/droite : epaule. A/E : coude. Rotation autour de Z,
    // ce qui garde le bras visible dans le plan de l'ecran.
    float shoulderAngleZ = 0.0f;
    float elbowAngleZ = 0.0f;

    Clock::time_point lastFrame = Clock::now();

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        Clock::time_point now = Clock::now();
        float dt = static_cast<float>(std::chrono::duration<double>(now - lastFrame).count());
        lastFrame = now;

        if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) {
            shoulderAngleZ += kAngularSpeedRadPerSec * dt;
        }
        if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) {
            shoulderAngleZ -= kAngularSpeedRadPerSec * dt;
        }
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
            elbowAngleZ += kAngularSpeedRadPerSec * dt;
        }
        if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) {
            elbowAngleZ -= kAngularSpeedRadPerSec * dt;
        }

        NkQuatf liveShoulder = Multiply(QuatFromAxisAngleZ(shoulderAngleZ), shoulderOrientation);
        NkQuatf liveElbow = Multiply(QuatFromAxisAngleZ(elbowAngleZ), elbowOrientation);
        const ArmPoses liveArm = ComputeArm(liveShoulder, liveElbow, handOrientation);

        glClearColor(0.08f, 0.08f, 0.10f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        DrawAxes(NkVec3f{0.0f, 0.0f, 0.0f}, kWorldAxisLength);
        DrawReachCircle(liveArm.shoulderWorld.position);

        DrawWorldSegment(liveArm.shoulderWorld.position, liveArm.elbowWorld.position, 0.9f, 0.9f,
                         0.9f, 1.0f);
        DrawWorldSegment(liveArm.elbowWorld.position, liveArm.handWorld.position, 0.9f, 0.9f, 0.9f,
                         1.0f);

        float sx, sy, ex, ey, hx, hy;
        WorldToScreen(liveArm.shoulderWorld.position, sx, sy);
        WorldToScreen(liveArm.elbowWorld.position, ex, ey);
        WorldToScreen(liveArm.handWorld.position, hx, hy);

        DrawCircle(sx, sy, 8.0f, 0.95f, 0.6f, 0.2f, 1.0f);   // epaule : orange
        DrawCircle(ex, ey, 7.0f, 0.2f, 0.85f, 0.95f, 1.0f);  // coude : cyan
        DrawCircle(hx, hy, 7.0f, 0.95f, 0.25f, 0.75f, 1.0f); // main : magenta

        glfwSwapBuffers(window);
    }

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
