// Chapitre 2, exercice 5 : "La composition"
//
// Verifie que composer deux poses puis appliquer le resultat a un point
// donne exactement le meme resultat qu'appliquer l'enfant puis le parent
// separement : ApplyPose(Composer(P, C), point) == ApplyPose(P, ApplyPose(C, point)).
//
// Un cas de test fixe (verifie manuellement) tourne au demarrage via assert,
// avant meme de lire l'entree utilisateur.
//
// Sortie console : exactement 7 lignes, sans texte mefngle -- resultA (3
// lignes, 4 decimales), resultB (3 lignes, 4 decimales), ecart (1 ligne, 6
// decimales).
//
// Note sur la visualisation : le repere "enfant place dans le monde" et le
// repere de la "pose composee" sont mathematiquement la meme chose ici
// (Composer() calcule exactement ApplyPose(parent, child.position) et
// Multiply(parent.orientation, child.orientation) -- il n'y a pas une
// deuxieme formule independante a comparer). Un seul reperer est donc
// dessine pour les deux ; leur coincidence est garantie par construction,
// pas verifiee visuellement par un doublon superflu.

#include "Composer.hpp"
#include "NkQuatf.hpp"
#include "NkVec3f.hpp"
#include "Pose.hpp"

#include <GLFW/glfw3.h>

#include <cassert>
#include <chrono>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <vector>

namespace {

constexpr nk_float32 kRoundTripTolerance = 1e-3f;

// Cas verifie manuellement avant d'ecrire ce fichier : parent = position
// (1,0,0) + rotation 90 deg autour de Z, enfant = position (0,2,0) +
// rotation 90 deg autour de Y, point = (1,1,1). Attendu :
// resultA == resultB == (-2, 1, -1), ecart ~ 0.
void RunInternalSelfTest() {
    const Pose parent{NkVec3f{1.0f, 0.0f, 0.0f}, NkQuatf{0.0f, 0.0f, 0.7071068f, 0.7071068f}};
    const Pose child{NkVec3f{0.0f, 2.0f, 0.0f}, NkQuatf{0.0f, 0.7071068f, 0.0f, 0.7071068f}};
    const NkVec3f point{1.0f, 1.0f, 1.0f};

    const Pose composed = Composer(parent, child);
    const NkVec3f resultA = ApplyPose(composed, point);
    const NkVec3f resultB = ApplyPose(parent, ApplyPose(child, point));
    const nk_float32 gap = Length(resultA - resultB);

    assert(gap < kRoundTripTolerance && "Composer() casse sur le cas de test fixe");
}

constexpr int kWindowWidth = 900;
constexpr int kWindowHeight = 700;
constexpr float kOriginX = kWindowWidth / 2.0f;
constexpr float kOriginY = kWindowHeight / 2.0f;
constexpr float kScale = 80.0f;  // pixels par metre

constexpr float kWorldAxisLength = 1.0f;
constexpr float kLocalAxisLength = 0.4f;

constexpr double kPhaseDurationMs = 600.0;  // 4 phases : enfant (rot+trans), parent (rot+trans)

using Clock = std::chrono::high_resolution_clock;

const NkQuatf kIdentityQuat{0.0f, 0.0f, 0.0f, 1.0f};

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

void DrawTrail(const std::vector<NkVec3f>& trail) {
    if (trail.size() < 2) {
        return;
    }
    glColor4f(0.95f, 0.85f, 0.2f, 0.8f);
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
    RunInternalSelfTest();

    Pose parent{};
    Pose child{};
    NkVec3f point{};

    int read = std::scanf(
        "%f %f %f %f %f %f %f  %f %f %f %f %f %f %f  %f %f %f", &parent.position.x,
        &parent.position.y, &parent.position.z, &parent.orientation.x, &parent.orientation.y,
        &parent.orientation.z, &parent.orientation.w, &child.position.x, &child.position.y,
        &child.position.z, &child.orientation.x, &child.orientation.y, &child.orientation.z,
        &child.orientation.w, &point.x, &point.y, &point.z);
    if (read != 17) {
        std::fprintf(stderr,
                      "Usage : dix-sept nombres reels separes par des espaces\n"
                      "  position parent (x y z)  quaternion parent (x y z w)\n"
                      "  position enfant (x y z)  quaternion enfant (x y z w)\n"
                      "  point (x y z)\n"
                      "  ex: 1 0 0  0 0 0.7071068 0.7071068  0 2 0  0 0.7071068 0 0.7071068  1 1 1\n");
        return EXIT_FAILURE;
    }

    const Pose composed = Composer(parent, child);
    const NkVec3f resultA = ApplyPose(composed, point);
    const NkVec3f resultB = ApplyPose(parent, ApplyPose(child, point));
    const nk_float32 gap = Length(resultA - resultB);

    std::printf("%.4f\n", resultA.x);
    std::printf("%.4f\n", resultA.y);
    std::printf("%.4f\n", resultA.z);
    std::printf("%.4f\n", resultB.x);
    std::printf("%.4f\n", resultB.y);
    std::printf("%.4f\n", resultB.z);
    std::printf("%.6f\n", gap);
    std::fflush(stdout);

    if (!glfwInit()) {
        std::fprintf(stderr, "Erreur : impossible d'initialiser GLFW.\n");
        return 1;
    }

    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    GLFWwindow* window = glfwCreateWindow(kWindowWidth, kWindowHeight,
                                           "Exo 5 (ch2) - La composition", nullptr, nullptr);
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
    std::vector<NkVec3f> trail;

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        if (state.replayRequested) {
            animStart = Clock::now();
            animating = true;
            trail.clear();
            state.replayRequested = false;
        }

        NkVec3f animatedPoint = resultB;

        if (animating) {
            double elapsedMs =
                std::chrono::duration<double, std::milli>(Clock::now() - animStart).count();

            if (elapsedMs < kPhaseDurationMs) {
                // Etape enfant, phase 1 : rotation autour de l'origine locale de l'enfant.
                float t = static_cast<float>(elapsedMs / kPhaseDurationMs);
                animatedPoint = RotateVectorByQuat(Nlerp(kIdentityQuat, child.orientation, t), point);
            } else if (elapsedMs < 2.0 * kPhaseDurationMs) {
                // Etape enfant, phase 2 : translation jusqu'a ApplyPose(child, point).
                float s = static_cast<float>((elapsedMs - kPhaseDurationMs) / kPhaseDurationMs);
                NkVec3f rotatedPoint = RotateVectorByQuat(child.orientation, point);
                animatedPoint = rotatedPoint + child.position * s;
            } else if (elapsedMs < 3.0 * kPhaseDurationMs) {
                // Etape parent, phase 1 : rotation autour de l'origine du monde.
                float t = static_cast<float>((elapsedMs - 2.0 * kPhaseDurationMs) / kPhaseDurationMs);
                NkVec3f afterChild = RotateVectorByQuat(child.orientation, point) + child.position;
                animatedPoint = RotateVectorByQuat(Nlerp(kIdentityQuat, parent.orientation, t), afterChild);
            } else if (elapsedMs < 4.0 * kPhaseDurationMs) {
                // Etape parent, phase 2 : translation jusqu'a resultB.
                float s = static_cast<float>((elapsedMs - 3.0 * kPhaseDurationMs) / kPhaseDurationMs);
                NkVec3f afterChild = RotateVectorByQuat(child.orientation, point) + child.position;
                NkVec3f rotatedAfterChild = RotateVectorByQuat(parent.orientation, afterChild);
                animatedPoint = rotatedAfterChild + parent.position * s;
            } else {
                animating = false;
                animatedPoint = resultB;
            }

            trail.push_back(animatedPoint);
        }

        glClearColor(0.08f, 0.08f, 0.10f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        DrawAxes(NkVec3f{0.0f, 0.0f, 0.0f}, kIdentityQuat, kWorldAxisLength);
        DrawAxes(parent.position, parent.orientation, kLocalAxisLength);
        // Repere de l'enfant place dans le monde == repere de la pose composee
        // (memes formules par construction, voir la note en tete de fichier).
        DrawAxes(composed.position, composed.orientation, kLocalAxisLength);

        DrawTrail(trail);

        // Point de depart (avant toute transformation).
        float xStart, yStart;
        WorldToScreen(point, xStart, yStart);
        DrawCircle(xStart, yStart, 5.0f, 0.9f, 0.9f, 0.9f, 1.0f);

        // Point final unique (resultA == resultB par construction).
        float xFinal, yFinal;
        WorldToScreen(resultA, xFinal, yFinal);
        DrawCircle(xFinal, yFinal, 8.0f, 0.95f, 0.25f, 0.75f, 1.0f);

        if (animating) {
            float xa, ya;
            WorldToScreen(animatedPoint, xa, ya);
            DrawCircle(xa, ya, 6.0f, 0.95f, 0.85f, 0.2f, 1.0f);
        }

        glfwSwapBuffers(window);
    }

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
