// Chapitre 2, exercice 8 : "L'extrapolation"
//
// Avance une pose de dt secondes a vitesses lineaire et angulaire
// constantes (voir Extrapolate.hpp pour le detail, notamment le cas de la
// vitesse angulaire nulle, protege contre la division par zero).
//
// Sortie console : la pose extrapolee, 7 lignes (position xyz, quaternion
// xyzw), 4 decimales, sans texte mefngle.

#include "Extrapolate.hpp"
#include "NkQuatf.hpp"
#include "NkVec3f.hpp"
#include "Pose.hpp"

#include <GLFW/glfw3.h>

#include <cassert>
#include <chrono>
#include <cmath>
#include <cstdio>
#include <cstdlib>

namespace {

constexpr nk_float32 kTolerance = 1e-3f;

bool AnyNan(const Pose& pose) {
    return std::isnan(pose.position.x) || std::isnan(pose.position.y) ||
           std::isnan(pose.position.z) || std::isnan(pose.orientation.x) ||
           std::isnan(pose.orientation.y) || std::isnan(pose.orientation.z) ||
           std::isnan(pose.orientation.w);
}

// Deux cas verifies manuellement avant d'ecrire ce fichier : rotation
// autour de Z a pi rad/s pendant 1 s, et vitesse angulaire nulle (le cas a
// proteger contre la division par zero).
void RunInternalSelfTests() {
    // Cas 1 : rotation.
    {
        const Pose pose{NkVec3f{1.0f, 2.0f, 3.0f}, NkQuatf{0.0f, 0.0f, 0.0f, 1.0f}};
        const NkVec3f linVel{0.5f, -1.0f, 2.0f};
        const NkVec3f angVel{0.0f, 0.0f, 3.14159265f};
        const Pose result = ExtrapolatePose(pose, linVel, angVel, 1.0f);

        assert(Length(result.position - NkVec3f{1.5f, 1.0f, 5.0f}) < kTolerance &&
               "Test 'rotation autour de Z' : position incorrecte");
        assert(Length(NkVec3f{result.orientation.x, result.orientation.y, result.orientation.z} -
                       NkVec3f{0.0f, 0.0f, 1.0f}) < kTolerance &&
               std::fabs(result.orientation.w) < kTolerance &&
               "Test 'rotation autour de Z' : quaternion incorrect");
    }

    // Cas 2 : vitesse angulaire nulle -- ne doit produire aucun NaN, et
    // laisser l'orientation strictement inchangee.
    {
        const Pose pose{NkVec3f{0.0f, 0.0f, 0.0f}, NkQuatf{0.1f, 0.2f, 0.3f, 0.9273f}};
        const NkVec3f linVel{1.0f, 1.0f, 1.0f};
        const NkVec3f angVel{0.0f, 0.0f, 0.0f};
        const Pose result = ExtrapolatePose(pose, linVel, angVel, 2.0f);

        assert(!AnyNan(result) && "Test 'vitesse angulaire nulle' : NaN produit");
        assert(Length(result.position - NkVec3f{2.0f, 2.0f, 2.0f}) < kTolerance &&
               "Test 'vitesse angulaire nulle' : position incorrecte");
        assert(result.orientation.x == pose.orientation.x &&
               result.orientation.y == pose.orientation.y &&
               result.orientation.z == pose.orientation.z &&
               result.orientation.w == pose.orientation.w &&
               "Test 'vitesse angulaire nulle' : orientation modifiee alors qu'elle ne devrait pas l'etre");
    }
}

constexpr int kWindowWidth = 900;
constexpr int kWindowHeight = 700;
constexpr float kOriginX = kWindowWidth / 2.0f;
constexpr float kOriginY = kWindowHeight / 2.0f;
constexpr float kScale = 80.0f;
constexpr float kAxisLength = 0.4f;
constexpr double kAnimDurationMs = 1500.0;  // duree reelle (a l'ecran) de l'animation

using Clock = std::chrono::high_resolution_clock;

void WorldToScreen(const NkVec3f& world, float& screenX, float& screenY) {
    screenX = kOriginX + (world.x - world.z) * kScale;
    screenY = kOriginY + (world.y - (world.x + world.z) * 0.5f) * kScale;
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

void DrawAxes(const NkVec3f& origin, const NkQuatf& orientation, float length, float alpha) {
    NkVec3f axisX = RotateVectorByQuat(orientation, NkVec3f{1.0f, 0.0f, 0.0f}) * length;
    NkVec3f axisY = RotateVectorByQuat(orientation, NkVec3f{0.0f, 1.0f, 0.0f}) * length;
    NkVec3f axisZ = RotateVectorByQuat(orientation, NkVec3f{0.0f, 0.0f, 1.0f}) * length;
    DrawWorldSegment(origin, origin + axisX, 0.9f, 0.2f, 0.2f, alpha);
    DrawWorldSegment(origin, origin + axisY, 0.2f, 0.85f, 0.3f, alpha);
    DrawWorldSegment(origin, origin + axisZ, 0.25f, 0.45f, 0.95f, alpha);
}

void DrawCircle(float cx, float cy, float radius, float r, float g, float b, float a) {
    constexpr int kSegments = 20;
    glColor4f(r, g, b, a);
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(cx, cy);
    for (int i = 0; i <= kSegments; ++i) {
        float theta = 2.0f * 3.14159265f * static_cast<float>(i) / kSegments;
        glVertex2f(cx + radius * std::cos(theta), cy + radius * std::sin(theta));
    }
    glEnd();
}

struct AppState {
    bool replayRequested = false;
};

void KeyCallback(GLFWwindow* window, int key, int /*scancode*/, int action, int /*mods*/) {
    auto* state = static_cast<AppState*>(glfwGetWindowUserPointer(window));
    if (!state) {
        return;
    }
    if (action == GLFW_PRESS && key == GLFW_KEY_SPACE) {
        state->replayRequested = true;
    }
}

}  // namespace

int main() {
    RunInternalSelfTests();

    NkVec3f position{};
    NkQuatf orientation{};
    NkVec3f linVel{};
    NkVec3f angVel{};
    nk_float32 dt = 0.0f;

    int read = std::scanf("%f %f %f %f %f %f %f %f %f %f %f %f %f %f", &position.x, &position.y,
                           &position.z, &orientation.x, &orientation.y, &orientation.z,
                           &orientation.w, &linVel.x, &linVel.y, &linVel.z, &angVel.x, &angVel.y,
                           &angVel.z, &dt);
    if (read != 14) {
        std::fprintf(stderr,
                      "Usage : quatorze nombres reels separes par des espaces\n"
                      "  position (x y z)  quaternion (x y z w)\n"
                      "  vitesse lineaire (x y z)  vitesse angulaire (x y z)  dt\n"
                      "  ex: 1 2 3  0 0 0 1  0.5 -1 2  0 0 3.14159265  1\n");
        return EXIT_FAILURE;
    }

    const Pose pose{position, orientation};
    const Pose extrapolated = ExtrapolatePose(pose, linVel, angVel, dt);

    std::printf("%.4f\n", extrapolated.position.x);
    std::printf("%.4f\n", extrapolated.position.y);
    std::printf("%.4f\n", extrapolated.position.z);
    std::printf("%.4f\n", extrapolated.orientation.x);
    std::printf("%.4f\n", extrapolated.orientation.y);
    std::printf("%.4f\n", extrapolated.orientation.z);
    std::printf("%.4f\n", extrapolated.orientation.w);
    std::fflush(stdout);

    if (!glfwInit()) {
        std::fprintf(stderr, "Erreur : impossible d'initialiser GLFW.\n");
        return 1;
    }

    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    GLFWwindow* window = glfwCreateWindow(kWindowWidth, kWindowHeight,
                                           "Exo 8 (ch2) - L'extrapolation", nullptr, nullptr);
    if (!window) {
        std::fprintf(stderr, "Erreur : impossible de creer la fenetre.\n");
        glfwTerminate();
        return 1;
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    AppState state;
    glfwSetWindowUserPointer(window, &state);
    glfwSetKeyCallback(window, KeyCallback);

    glViewport(0, 0, kWindowWidth, kWindowHeight);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0.0, kWindowWidth, 0.0, kWindowHeight, -1.0, 1.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glLineWidth(2.0f);

    Clock::time_point animStart = Clock::now();
    bool animating = true;

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        if (state.replayRequested) {
            animStart = Clock::now();
            animating = true;
            state.replayRequested = false;
        }

        Pose animatedPose = extrapolated;
        if (animating) {
            double elapsedMs =
                std::chrono::duration<double, std::milli>(Clock::now() - animStart).count();
            if (elapsedMs < kAnimDurationMs) {
                float u = static_cast<float>(elapsedMs / kAnimDurationMs);
                // Evalue la vraie extrapolation continue au temps u*dt, pas
                // une simple interpolation lineaire entre les deux reperes
                // (l'orientation suit un arc, pas une ligne droite).
                animatedPose = ExtrapolatePose(pose, linVel, angVel, u * dt);
            } else {
                animating = false;
                animatedPose = extrapolated;
            }
        }

        glClearColor(0.08f, 0.08f, 0.10f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        DrawAxes(pose.position, pose.orientation, kAxisLength, 1.0f);
        DrawAxes(extrapolated.position, extrapolated.orientation, kAxisLength, 1.0f);

        float xs, ys, xe, ye;
        WorldToScreen(pose.position, xs, ys);
        WorldToScreen(extrapolated.position, xe, ye);
        DrawCircle(xs, ys, 5.0f, 0.9f, 0.9f, 0.9f, 1.0f);
        DrawCircle(xe, ye, 6.0f, 0.95f, 0.6f, 0.2f, 1.0f);

        if (animating) {
            DrawAxes(animatedPose.position, animatedPose.orientation, kAxisLength * 0.7f, 0.9f);
            float xa, ya;
            WorldToScreen(animatedPose.position, xa, ya);
            DrawCircle(xa, ya, 5.0f, 0.95f, 0.85f, 0.2f, 1.0f);
        }

        glfwSwapBuffers(window);
    }

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
