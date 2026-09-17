// Chapitre 2, exercice 9 : "Le chemin court"
//
// Calcule la vitesse angulaire moyenne entre deux orientations q1 et q2 sur
// dt secondes, avec et sans le forcage du chemin court (voir ShortPath.hpp
// pour l'explication du double revetement des quaternions).
//
// Sortie console : deux blocs de 3 lignes, 4 decimales, sans texte
// mefngle -- d'abord SANS forcage, puis AVEC forcage.

#include "NkQuatf.hpp"
#include "NkVec3f.hpp"
#include "ShortPath.hpp"

#include <GLFW/glfw3.h>

#include <cassert>
#include <chrono>
#include <cmath>
#include <cstdio>
#include <cstdlib>

namespace {

constexpr nk_float32 kQuatUnitTolerance = 1e-3f;

bool IsUnitQuat(const NkQuatf& q) {
    return std::fabs(Dot(q, q) - 1.0f) < kQuatUnitTolerance;
}

// Exemple deja verifie manuellement : q1 = identite, q2 = -(rotation de
// 10 deg autour de Z) -- deux orientations presque identiques en realite
// (10 deg d'ecart), mais prises sur des hemispheres opposes du double
// revetement (produit scalaire negatif). Sans forcage, la vitesse
// angulaire calculee est absurde (environ 350 deg/s au lieu de 10 deg/s).
void RunInternalSelfTest() {
    const NkQuatf q1{0.0f, 0.0f, 0.0f, 1.0f};
    const NkQuatf q2{-0.0f, -0.0f, -0.0871557f, -0.9961947f};
    constexpr nk_float32 dt = 1.0f;

    assert(Dot(q1, q2) < 0.0f && "Le cas de test ne reproduit plus un produit scalaire negatif");

    NkVec3f withoutForcing = AverageAngularVelocity(q1, q2, dt, false);
    NkVec3f withForcing = AverageAngularVelocity(q1, q2, dt, true);

    assert(Length(withoutForcing) > 3.0f &&
           "Sans forcage : la norme devrait etre absurdement grande (chemin long)");
    assert(std::fabs(Length(withForcing) - 0.1745f) < 1e-2f &&
           "Avec forcage : la norme devrait etre proche de 0.1745 rad/s (chemin court, 10 deg/s)");
}

constexpr int kWindowWidth = 900;
constexpr int kWindowHeight = 700;
constexpr float kOriginX = kWindowWidth / 2.0f;
constexpr float kOriginY = kWindowHeight / 2.0f;
constexpr float kScale = 250.0f;
constexpr float kAxisLength = 0.35f;
constexpr float kArcRadius = 0.6f;
constexpr NkVec3f kQ2Offset{0.15f, 0.15f, 0.0f};  // decalage purement visuel, pour distinguer q1/q2

using Clock = std::chrono::high_resolution_clock;

NkQuatf QuatFromAxisAngle(NkVec3f axis, nk_float32 angle) {
    nk_float32 len = Length(axis);
    if (len > 1e-8f) {
        axis = axis * (1.0f / len);
    }
    nk_float32 half = angle * 0.5f;
    nk_float32 s = std::sin(half);
    return NkQuatf{axis.x * s, axis.y * s, axis.z * s, std::cos(half)};
}

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

void DrawAxes(const NkVec3f& origin, const NkQuatf& orientation, float length) {
    NkVec3f axisX = RotateVectorByQuat(orientation, NkVec3f{1.0f, 0.0f, 0.0f}) * length;
    NkVec3f axisY = RotateVectorByQuat(orientation, NkVec3f{0.0f, 1.0f, 0.0f}) * length;
    NkVec3f axisZ = RotateVectorByQuat(orientation, NkVec3f{0.0f, 0.0f, 1.0f}) * length;
    DrawWorldSegment(origin, origin + axisX, 0.9f, 0.2f, 0.2f, 1.0f);
    DrawWorldSegment(origin, origin + axisY, 0.2f, 0.85f, 0.3f, 1.0f);
    DrawWorldSegment(origin, origin + axisZ, 0.25f, 0.45f, 0.95f, 1.0f);
}

// Dessine l'arc balaye en tournant referencePoint (autour de origin) de 0 a
// angle radians autour de axis, en escalier de segments courts.
void DrawArc(const NkVec3f& origin, const NkVec3f& referencePoint, NkVec3f axis, float angle,
             float r, float g, float b) {
    constexpr int kSteps = 64;
    float len = Length(axis);
    if (len < 1e-8f) {
        return;
    }
    axis = axis * (1.0f / len);

    glColor4f(r, g, b, 0.9f);
    glBegin(GL_LINE_STRIP);
    for (int i = 0; i <= kSteps; ++i) {
        float theta = angle * static_cast<float>(i) / static_cast<float>(kSteps);
        NkQuatf q = QuatFromAxisAngle(axis, theta);
        NkVec3f p = origin + RotateVectorByQuat(q, referencePoint - origin);
        float sx, sy;
        WorldToScreen(p, sx, sy);
        glVertex2f(sx, sy);
    }
    glEnd();
}

}  // namespace

int main() {
    RunInternalSelfTest();

    NkQuatf q1{}, q2{};
    nk_float32 dt = 0.0f;

    int read = std::scanf("%f %f %f %f %f %f %f %f %f", &q1.x, &q1.y, &q1.z, &q1.w, &q2.x, &q2.y,
                           &q2.z, &q2.w, &dt);
    if (read != 9) {
        std::fprintf(stderr,
                      "Usage : neuf nombres reels separes par des espaces\n"
                      "  q1 (x y z w)  q2 (x y z w)  dt\n"
                      "  ex: 0 0 0 1  -0.0000 -0.0000 -0.0871557 -0.9961947  1\n");
        return EXIT_FAILURE;
    }

    assert(IsUnitQuat(q1) && "q1 n'est pas unitaire");
    assert(IsUnitQuat(q2) && "q2 n'est pas unitaire");

    const NkVec3f withoutForcing = AverageAngularVelocity(q1, q2, dt, false);
    const NkVec3f withForcing = AverageAngularVelocity(q1, q2, dt, true);

    std::printf("%.4f\n", withoutForcing.x);
    std::printf("%.4f\n", withoutForcing.y);
    std::printf("%.4f\n", withoutForcing.z);
    std::printf("%.4f\n", withForcing.x);
    std::printf("%.4f\n", withForcing.y);
    std::printf("%.4f\n", withForcing.z);
    std::fflush(stdout);

    if (!glfwInit()) {
        std::fprintf(stderr, "Erreur : impossible d'initialiser GLFW.\n");
        return 1;
    }

    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    GLFWwindow* window = glfwCreateWindow(kWindowWidth, kWindowHeight,
                                           "Exo 9 (ch2) - Le chemin court", nullptr, nullptr);
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

    // Axe et angle reellement utilises par chaque calcul, extraits
    // directement des vitesses angulaires ci-dessus (pas re-derives a la
    // main) : c'est exactement ce que le programme vient d'imprimer.
    const NkVec3f origin{0.0f, 0.0f, 0.0f};
    const NkVec3f referencePoint = origin + NkVec3f{kArcRadius, 0.0f, 0.0f};
    const float angleWithout = Length(withoutForcing) * dt;
    const float angleWith = Length(withForcing) * dt;

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        glClearColor(0.08f, 0.08f, 0.10f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        DrawAxes(origin, q1, kAxisLength);
        DrawAxes(origin + kQ2Offset, q2, kAxisLength);

        // Chemin long (sans forcage, rouge) puis chemin court (avec
        // forcage, vert) par-dessus, pour que le petit arc reste visible
        // meme s'il est en grande partie recouvert par le grand.
        DrawArc(origin, referencePoint, withoutForcing, angleWithout, 0.9f, 0.25f, 0.25f);
        DrawArc(origin, referencePoint, withForcing, angleWith, 0.25f, 0.9f, 0.35f);

        glfwSwapBuffers(window);
    }

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
