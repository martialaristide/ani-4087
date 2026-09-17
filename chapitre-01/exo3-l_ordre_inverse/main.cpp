// Chapitre 2, exercice 3 : "L'ordre inverse"
//
// Reprend la pose et le point de l'exercice 2, et calcule le meme point
// transforme dans les deux ordres possibles :
//   ApplyPose()                 : rotation puis translation (convention du module)
//   ApplyPoseTranslationFirst() : translation puis rotation (l'inverse)
//
// Sortie console : exactement 6 lignes, 4 decimales, sans texte melange
// (3 lignes ApplyPose, puis 3 lignes ApplyPoseTranslationFirst).
//
// La fenetre montre les deux chemins comme deux trajectoires distinctes
// (cyan = rotation puis translation, orange = translation puis rotation).
// Avec une translation alignee sur l'axe de rotation (ex: rotation autour de
// Z + translation le long de Z), les deux chemins se confondent visuellement
// -- preuve visuelle que l'ordre ne compte plus dans ce cas particulier.

#include "ApplyPoseReversed.hpp"
#include "NkQuatf.hpp"
#include "NkVec3f.hpp"
#include "Pose.hpp"

#include <GLFW/glfw3.h>

#include <chrono>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <vector>

namespace {

constexpr int kWindowWidth = 900;
constexpr int kWindowHeight = 700;
constexpr float kOriginX = kWindowWidth / 2.0f;
constexpr float kOriginY = kWindowHeight / 2.0f;
constexpr float kScale = 100.0f;  // pixels par metre

constexpr float kWorldAxisLength = 1.0f;  // metres
constexpr float kLocalAxisLength = 0.4f;  // metres

constexpr double kPhase1DurationMs = 750.0;
constexpr double kPhase2DurationMs = 750.0;

using Clock = std::chrono::high_resolution_clock;

const NkQuatf kIdentityQuat{0.0f, 0.0f, 0.0f, 1.0f};

// Interpolation lineaire normalisee (nlerp) : purement visuelle, le resultat
// exact affiche en console vient uniquement des deux fonctions ApplyPose*.
NkQuatf Nlerp(const NkQuatf& a, const NkQuatf& b, float t) {
    NkQuatf bb = b;
    float dot = a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;
    if (dot < 0.0f) {
        bb.x = -bb.x;
        bb.y = -bb.y;
        bb.z = -bb.z;
        bb.w = -bb.w;
    }
    NkQuatf r{
        a.x + (bb.x - a.x) * t,
        a.y + (bb.y - a.y) * t,
        a.z + (bb.z - a.z) * t,
        a.w + (bb.w - a.w) * t,
    };
    float len = std::sqrt(r.x * r.x + r.y * r.y + r.z * r.z + r.w * r.w);
    if (len > 1e-8f) {
        r.x /= len;
        r.y /= len;
        r.z /= len;
        r.w /= len;
    }
    return r;
}

void WorldToScreen(const NkVec3f& world, float& screenX, float& screenY) {
    screenX = kOriginX + (world.x - world.z) * kScale;
    screenY = kOriginY + (world.y - (world.x + world.z) * 0.5f) * kScale;
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

void DrawAxes(const NkVec3f& origin, const NkQuatf& orientation, float length) {
    NkVec3f axisX = RotateVectorByQuat(orientation, NkVec3f{1.0f, 0.0f, 0.0f}) * length;
    NkVec3f axisY = RotateVectorByQuat(orientation, NkVec3f{0.0f, 1.0f, 0.0f}) * length;
    NkVec3f axisZ = RotateVectorByQuat(orientation, NkVec3f{0.0f, 0.0f, 1.0f}) * length;
    DrawWorldSegment(origin, origin + axisX, 0.9f, 0.2f, 0.2f, 1.0f);
    DrawWorldSegment(origin, origin + axisY, 0.2f, 0.85f, 0.3f, 1.0f);
    DrawWorldSegment(origin, origin + axisZ, 0.25f, 0.45f, 0.95f, 1.0f);
}

void DrawTrail(const std::vector<NkVec3f>& trail, float r, float g, float b) {
    if (trail.size() < 2) {
        return;
    }
    glColor4f(r, g, b, 0.85f);
    glBegin(GL_LINE_STRIP);
    for (const NkVec3f& p : trail) {
        float sx, sy;
        WorldToScreen(p, sx, sy);
        glVertex2f(sx, sy);
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
    NkVec3f position{};
    NkQuatf orientation{};
    NkVec3f point{};

    int read = std::scanf("%f %f %f %f %f %f %f %f %f %f", &position.x, &position.y,
                           &position.z, &orientation.x, &orientation.y, &orientation.z,
                           &orientation.w, &point.x, &point.y, &point.z);
    if (read != 10) {
        std::fprintf(stderr,
                      "Usage : dix nombres reels separes par des espaces\n"
                      "  position (x y z)  quaternion (x y z w)  point (x y z)\n"
                      "  ex: 0 0 0  0 0 0.7071068 0.7071068  1 0 0\n");
        return EXIT_FAILURE;
    }

    const Pose pose{position, orientation};
    const NkVec3f resultA = ApplyPose(pose, point);                    // rotation puis translation
    const NkVec3f resultB = ApplyPoseTranslationFirst(pose, point);    // translation puis rotation

    std::printf("%.4f\n", resultA.x);
    std::printf("%.4f\n", resultA.y);
    std::printf("%.4f\n", resultA.z);
    std::printf("%.4f\n", resultB.x);
    std::printf("%.4f\n", resultB.y);
    std::printf("%.4f\n", resultB.z);
    std::fflush(stdout);

    if (!glfwInit()) {
        std::fprintf(stderr, "Erreur : impossible d'initialiser GLFW.\n");
        return 1;
    }

    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    GLFWwindow* window = glfwCreateWindow(kWindowWidth, kWindowHeight,
                                           "Exo 3 (ch2) - L'ordre inverse", nullptr, nullptr);
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
    std::vector<NkVec3f> trailA;
    std::vector<NkVec3f> trailB;

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        if (state.replayRequested) {
            animStart = Clock::now();
            animating = true;
            trailA.clear();
            trailB.clear();
            state.replayRequested = false;
        }

        NkVec3f posA = resultA;
        NkVec3f posB = resultB;

        if (animating) {
            double elapsedMs =
                std::chrono::duration<double, std::milli>(Clock::now() - animStart).count();

            if (elapsedMs < kPhase1DurationMs) {
                float t = static_cast<float>(elapsedMs / kPhase1DurationMs);
                // Chemin A : tourne d'abord autour de l'origine locale.
                posA = RotateVectorByQuat(Nlerp(kIdentityQuat, pose.orientation, t), point);
                // Chemin B : translate d'abord.
                posB = point + pose.position * t;
            } else if (elapsedMs < kPhase1DurationMs + kPhase2DurationMs) {
                float s = static_cast<float>((elapsedMs - kPhase1DurationMs) / kPhase2DurationMs);
                // Chemin A : translate ensuite.
                NkVec3f rotatedPoint = RotateVectorByQuat(pose.orientation, point);
                posA = rotatedPoint + pose.position * s;
                // Chemin B : tourne ensuite, autour de l'origine du monde.
                NkVec3f translatedPoint = point + pose.position;
                posB = RotateVectorByQuat(Nlerp(kIdentityQuat, pose.orientation, s), translatedPoint);
            } else {
                animating = false;
                posA = resultA;
                posB = resultB;
            }

            trailA.push_back(posA);
            trailB.push_back(posB);
        }

        glClearColor(0.08f, 0.08f, 0.10f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        DrawAxes(NkVec3f{0.0f, 0.0f, 0.0f}, kIdentityQuat, kWorldAxisLength);
        DrawAxes(pose.position, pose.orientation, kLocalAxisLength);

        // Point de depart, commun aux deux chemins.
        float sx, sy;
        WorldToScreen(point, sx, sy);
        DrawCircle(sx, sy, 5.0f, 0.9f, 0.9f, 0.9f, 1.0f);

        DrawTrail(trailA, 0.2f, 0.85f, 0.95f);   // cyan
        DrawTrail(trailB, 0.95f, 0.6f, 0.2f);    // orange

        float xa, ya, xb, yb;
        WorldToScreen(posA, xa, ya);
        WorldToScreen(posB, xb, yb);
        DrawCircle(xa, ya, 8.0f, 0.2f, 0.85f, 0.95f, 1.0f);
        DrawCircle(xb, yb, 8.0f, 0.95f, 0.6f, 0.2f, 1.0f);

        glfwSwapBuffers(window);
    }

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
