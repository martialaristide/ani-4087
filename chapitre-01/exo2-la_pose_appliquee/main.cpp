// Chapitre 2, exercice 2 : "La pose appliquee" (avec visualisation)
//
// Lit une pose (position + quaternion) et un point, affiche en console le
// point transforme par ApplyPose (rotation puis translation), puis ouvre une
// fenetre qui montre pourquoi cet ordre compte : un point translate seul
// (sans rotation) fige au mauvais endroit par rapport au meme point tourne
// puis translate.
//
// Convention d'entree retenue (l'enonce ne la precisait pas) : 10 reels
// separes par des espaces, dans l'ordre
//   position (x y z)  quaternion (x y z w)  point (x y z)
//
// Projection isometrique simple (pas de camera 3D perspective a ce stade) :
//   ecran_x = origine_x + (monde_x - monde_z) * echelle
//   ecran_y = origine_y + (monde_y - (monde_x + monde_z) * 0.5) * echelle

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
constexpr float kScale = 100.0f;  // pixels par metre

constexpr float kWorldAxisLength = 1.0f;   // metres
constexpr float kLocalAxisLength = 0.4f;   // metres

constexpr double kPhase1DurationMs = 750.0;  // rotation autour de l'origine locale
constexpr double kPhase2DurationMs = 750.0;  // translation vers la position finale

using Clock = std::chrono::high_resolution_clock;

const NkQuatf kIdentityQuat{0.0f, 0.0f, 0.0f, 1.0f};

// Interpolation lineaire normalisee (nlerp), suffisante pour une animation
// purement visuelle : le resultat exact affiche en console vient uniquement
// de ApplyPose(), jamais de cet etat intermediaire.
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

void DrawCircle(float cx, float cy, float radius, float r, float g, float b, float a,
                bool filled) {
    constexpr int kSegments = 24;
    glColor4f(r, g, b, a);
    glBegin(filled ? GL_TRIANGLE_FAN : GL_LINE_LOOP);
    if (filled) {
        glVertex2f(cx, cy);
    }
    for (int i = 0; i <= (filled ? kSegments : kSegments - 1); ++i) {
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
    DrawCircle(x1, y1, 3.0f, r, g, b, a, true);
}

// Axes d'un repere (monde ou local) : X rouge, Y vert, Z bleu, en projection
// isometrique, partant de `origin` et tournes par `orientation`.
void DrawAxes(const NkVec3f& origin, const NkQuatf& orientation, float length, float alpha) {
    NkVec3f axisX = RotateVectorByQuat(orientation, NkVec3f{1.0f, 0.0f, 0.0f}) * length;
    NkVec3f axisY = RotateVectorByQuat(orientation, NkVec3f{0.0f, 1.0f, 0.0f}) * length;
    NkVec3f axisZ = RotateVectorByQuat(orientation, NkVec3f{0.0f, 0.0f, 1.0f}) * length;

    DrawWorldSegment(origin, origin + axisX, 0.9f, 0.2f, 0.2f, alpha);
    DrawWorldSegment(origin, origin + axisY, 0.2f, 0.85f, 0.3f, alpha);
    DrawWorldSegment(origin, origin + axisZ, 0.25f, 0.45f, 0.95f, alpha);
}

void DrawScene(const Pose& pose, const NkVec3f& point, const NkVec3f& pointNoRotation,
               const NkVec3f& pointFinal, bool animating, const NkVec3f& animatedPoint) {
    glClearColor(0.08f, 0.08f, 0.10f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    // Reperes : monde a l'origine, local a la position de la pose.
    DrawAxes(NkVec3f{0.0f, 0.0f, 0.0f}, kIdentityQuat, kWorldAxisLength, 1.0f);
    DrawAxes(pose.position, pose.orientation, kLocalAxisLength, 1.0f);

    // Vecteurs reliant la pose aux deux points compares (montre ce que la
    // rotation deplace par rapport a une simple translation).
    DrawWorldSegment(pose.position, pointNoRotation, 0.5f, 0.5f, 0.5f, 0.5f);
    DrawWorldSegment(pose.position, pointFinal, 0.9f, 0.9f, 0.9f, 0.4f);

    // Point translate seul (sans rotation), attenue.
    float xNoRot, yNoRot;
    WorldToScreen(pointNoRotation, xNoRot, yNoRot);
    DrawCircle(xNoRot, yNoRot, 7.0f, 0.7f, 0.7f, 0.7f, 0.6f, false);

    // Point final reel : rotation puis translation (ApplyPose).
    float xFinal, yFinal;
    WorldToScreen(pointFinal, xFinal, yFinal);
    DrawCircle(xFinal, yFinal, 8.0f, 0.95f, 0.25f, 0.75f, 1.0f, true);

    // Marqueur d'animation : trajectoire rotation-puis-translation.
    if (animating) {
        float xAnim, yAnim;
        WorldToScreen(animatedPoint, xAnim, yAnim);
        DrawCircle(xAnim, yAnim, 6.0f, 0.95f, 0.85f, 0.2f, 1.0f, true);
    }

    (void)point;
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
                      "  ex: 0 0 0  0 0 0 1  1 0 0\n");
        return EXIT_FAILURE;
    }

    const Pose pose{position, orientation};
    const NkVec3f pointFinal = ApplyPose(pose, point);

    std::printf("%.4f\n", pointFinal.x);
    std::printf("%.4f\n", pointFinal.y);
    std::printf("%.4f\n", pointFinal.z);
    std::fflush(stdout);

    const NkVec3f pointNoRotation = pose.position + point;

    if (!glfwInit()) {
        std::fprintf(stderr, "Erreur : impossible d'initialiser GLFW.\n");
        return 1;
    }

    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    GLFWwindow* window = glfwCreateWindow(kWindowWidth, kWindowHeight,
                                           "Exo 2 (ch2) - La pose appliquee", nullptr, nullptr);
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

        NkVec3f animatedPoint = pointFinal;
        if (animating) {
            double elapsedMs =
                std::chrono::duration<double, std::milli>(Clock::now() - animStart).count();

            if (elapsedMs < kPhase1DurationMs) {
                float t = static_cast<float>(elapsedMs / kPhase1DurationMs);
                NkQuatf q = Nlerp(kIdentityQuat, pose.orientation, t);
                animatedPoint = RotateVectorByQuat(q, point);
            } else if (elapsedMs < kPhase1DurationMs + kPhase2DurationMs) {
                float s = static_cast<float>((elapsedMs - kPhase1DurationMs) / kPhase2DurationMs);
                NkVec3f rotatedPoint = RotateVectorByQuat(pose.orientation, point);
                animatedPoint = rotatedPoint + pose.position * s;
            } else {
                animating = false;
            }
        }

        DrawScene(pose, point, pointNoRotation, pointFinal, animating, animatedPoint);
        glfwSwapBuffers(window);
    }

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
