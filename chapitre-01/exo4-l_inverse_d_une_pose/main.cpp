// Chapitre 2, exercice 4 : "L'inverse d'une pose"
//
// Verifie Inverser(pose) : appliquer la pose a un point, puis l'inverse au
// resultat, doit redonner le point de depart (aux arrondis flottants pres).
//
// Avant meme de lire l'entree utilisateur, le programme rejoue trois cas de
// test internes fixes (pose non triviale, identite, rotation autour de X) et
// s'arrete via assert() si l'un d'eux echoue -- un bug dans Inverser() ne
// doit jamais passer inapercu derriere une sortie qui a l'air normale.
//
// Sortie console : une seule ligne, l'ecart (norme de revenu - point),
// %.6f, rien d'autre.
//
// Note sur la visualisation par rapport a la description initiale de cet
// exercice : c'est bien le POINT DE DEPART et le POINT REVENU qui doivent se
// superposer visuellement (c'est litteralement ce que l'exercice verifie :
// Inverser() ramene au point de depart) -- pas le point transforme, qui est
// en general un endroit different. Voir la reponse envoyee dans le chat pour
// le detail de cette correction.

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

constexpr nk_float32 kQuatUnitTolerance = 1e-4f;
constexpr nk_float32 kRoundTripTolerance = 1e-3f;

bool IsUnitQuat(const NkQuatf& q) {
    nk_float32 lengthSq = q.x * q.x + q.y * q.y + q.z * q.z + q.w * q.w;
    return std::fabs(lengthSq - 1.0f) < kQuatUnitTolerance;
}

nk_float32 RoundTripGap(const Pose& pose, const NkVec3f& point) {
    NkVec3f transforme = ApplyPose(pose, point);
    Pose inverse = Inverser(pose);
    NkVec3f revenu = ApplyPose(inverse, transforme);
    return Length(revenu - point);
}

// Trois cas fixes, verifies manuellement avant d'ecrire ce fichier : une
// pose non triviale (rotation 90 deg autour de Z), l'identite, et une
// rotation autour d'un autre axe (X). Sert de filet de securite : si
// quelqu'un modifie Inverser() plus tard et casse la formule, le programme
// s'arrete ici au lieu de continuer avec un resultat faux.
void RunInternalSelfTests() {
    const Pose poseNonTriviale{NkVec3f{3.0f, -2.0f, 5.0f},
                                NkQuatf{0.0f, 0.0f, 0.7071068f, 0.7071068f}};
    const Pose poseIdentite{NkVec3f{0.0f, 0.0f, 0.0f}, NkQuatf{0.0f, 0.0f, 0.0f, 1.0f}};
    const Pose poseRotationX{NkVec3f{1.0f, 1.0f, 1.0f},
                              NkQuatf{0.7071068f, 0.0f, 0.0f, 0.7071068f}};

    const NkVec3f pointA{1.0f, 2.0f, 3.0f};
    const NkVec3f pointB{4.0f, -1.0f, 2.0f};
    const NkVec3f pointC{2.0f, 0.0f, 0.0f};

    assert(IsUnitQuat(poseNonTriviale.orientation) && "quaternion de test non unitaire");
    assert(IsUnitQuat(poseIdentite.orientation) && "quaternion de test non unitaire");
    assert(IsUnitQuat(poseRotationX.orientation) && "quaternion de test non unitaire");

    assert(RoundTripGap(poseNonTriviale, pointA) < kRoundTripTolerance &&
           "Inverser() casse sur le cas 'pose non triviale'");
    assert(RoundTripGap(poseIdentite, pointB) < kRoundTripTolerance &&
           "Inverser() casse sur le cas 'identite'");
    assert(RoundTripGap(poseRotationX, pointC) < kRoundTripTolerance &&
           "Inverser() casse sur le cas 'rotation autour de X'");
}

constexpr int kWindowWidth = 900;
constexpr int kWindowHeight = 700;
constexpr float kOriginX = kWindowWidth / 2.0f;
constexpr float kOriginY = kWindowHeight / 2.0f;
constexpr float kScale = 100.0f;  // pixels par metre

constexpr float kWorldAxisLength = 1.0f;
constexpr float kLocalAxisLength = 0.4f;

constexpr double kPhaseDurationMs = 600.0;  // 4 phases : aller (rotation+translation), retour (idem)

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
    RunInternalSelfTests();

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
                      "  ex: 3 -2 5  0 0 0.7071068 0.7071068  1 2 3\n");
        return EXIT_FAILURE;
    }

    const Pose pose{position, orientation};
    assert(IsUnitQuat(pose.orientation) && "le quaternion lu n'est pas unitaire");

    const NkVec3f transforme = ApplyPose(pose, point);
    const Pose inverse = Inverser(pose);
    const NkVec3f revenu = ApplyPose(inverse, transforme);
    const nk_float32 gap = Length(revenu - point);

    assert(gap < kRoundTripTolerance && "l'ecart apres aller-retour depasse le seuil attendu");

    std::printf("%.6f\n", gap);
    std::fflush(stdout);

    if (!glfwInit()) {
        std::fprintf(stderr, "Erreur : impossible d'initialiser GLFW.\n");
        return 1;
    }

    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    GLFWwindow* window = glfwCreateWindow(kWindowWidth, kWindowHeight,
                                           "Exo 4 (ch2) - L'inverse d'une pose", nullptr, nullptr);
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

        NkVec3f animatedPoint = revenu;

        if (animating) {
            double elapsedMs =
                std::chrono::duration<double, std::milli>(Clock::now() - animStart).count();

            if (elapsedMs < kPhaseDurationMs) {
                // Aller, phase 1 : rotation autour de l'origine locale.
                float t = static_cast<float>(elapsedMs / kPhaseDurationMs);
                animatedPoint = RotateVectorByQuat(Nlerp(kIdentityQuat, pose.orientation, t), point);
            } else if (elapsedMs < 2.0 * kPhaseDurationMs) {
                // Aller, phase 2 : translation jusqu'a "transforme".
                float s = static_cast<float>((elapsedMs - kPhaseDurationMs) / kPhaseDurationMs);
                NkVec3f rotatedPoint = RotateVectorByQuat(pose.orientation, point);
                animatedPoint = rotatedPoint + pose.position * s;
            } else if (elapsedMs < 3.0 * kPhaseDurationMs) {
                // Retour, phase 1 : rotation par l'inverse.
                float t = static_cast<float>((elapsedMs - 2.0 * kPhaseDurationMs) / kPhaseDurationMs);
                animatedPoint = RotateVectorByQuat(Nlerp(kIdentityQuat, inverse.orientation, t), transforme);
            } else if (elapsedMs < 4.0 * kPhaseDurationMs) {
                // Retour, phase 2 : translation jusqu'a "revenu".
                float s = static_cast<float>((elapsedMs - 3.0 * kPhaseDurationMs) / kPhaseDurationMs);
                NkVec3f rotatedTransforme = RotateVectorByQuat(inverse.orientation, transforme);
                animatedPoint = rotatedTransforme + inverse.position * s;
            } else {
                animating = false;
                animatedPoint = revenu;
            }

            trail.push_back(animatedPoint);
        }

        glClearColor(0.08f, 0.08f, 0.10f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        DrawAxes(NkVec3f{0.0f, 0.0f, 0.0f}, kIdentityQuat, kWorldAxisLength);
        DrawAxes(pose.position, pose.orientation, kLocalAxisLength);
        DrawAxes(inverse.position, inverse.orientation, kLocalAxisLength);

        DrawTrail(trail);

        // Point de depart (blanc, plein).
        float xStart, yStart;
        WorldToScreen(point, xStart, yStart);
        DrawCircle(xStart, yStart, 6.0f, 0.9f, 0.9f, 0.9f, 1.0f, true);

        // Point transforme par la pose (orange, plein).
        float xT, yT;
        WorldToScreen(transforme, xT, yT);
        DrawCircle(xT, yT, 7.0f, 0.95f, 0.6f, 0.2f, 1.0f, true);

        // Point revenu apres l'inverse (anneau vert) : doit se superposer au
        // point de depart, c'est ce que cet exercice verifie.
        float xR, yR;
        WorldToScreen(revenu, xR, yR);
        DrawCircle(xR, yR, 10.0f, 0.25f, 0.9f, 0.4f, 1.0f, false);

        if (animating) {
            float xa, ya;
            WorldToScreen(animatedPoint, xa, ya);
            DrawCircle(xa, ya, 6.0f, 0.95f, 0.85f, 0.2f, 1.0f, true);
        }

        glfwSwapBuffers(window);
    }

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
