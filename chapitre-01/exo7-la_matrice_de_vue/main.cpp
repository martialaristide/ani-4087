// Chapitre 2, exercice 7 : "La matrice de vue"
//
// Compare deux facons de calculer l'inverse de la matrice d'une pose :
//   InvertGeneral(Mat4)    -- inversion generique par cofacteurs/adjugate,
//                              qui renvoie l'identite SANS ERREUR sur une
//                              matrice singuliere (defaut reproduit a
//                              dessein, voir Mat4.hpp).
//   InvertAnalytical(Pose) -- construction directe via le conjugue du
//                              quaternion, qui ne peut jamais tomber dans
//                              ce piege.
//
// Pour une pose valide, les deux methodes s'accordent sur les 16
// coefficients (verifie par assert). Puis on passe une matrice
// deliberement corrompue (deux lignes identiques dans le bloc rotation,
// simulant un bug en amont -- PAS une pose bizarre : une vraie pose, meme
// avec un quaternion degenere, ne peut jamais etre singuliere) a
// InvertGeneral, pour montrer qu'elle renvoie l'identite sans jamais
// signaler que l'entree etait fausse.
//
// Pas de contrat de sortie strict impose par l'enonce ici : la sortie
// console utilise du texte pour rester lisible, contrairement aux
// exercices precedents.

#include "Mat4.hpp"
#include "NkQuatf.hpp"
#include "NkVec3f.hpp"
#include "Pose.hpp"

#include <GLFW/glfw3.h>

#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstdio>
#include <cstdlib>

namespace {

constexpr nk_float32 kQuatUnitTolerance = 1e-4f;
constexpr nk_float32 kMatrixTolerance = 1e-3f;

bool IsUnitQuat(const NkQuatf& q) {
    nk_float32 lengthSq = q.x * q.x + q.y * q.y + q.z * q.z + q.w * q.w;
    return std::fabs(lengthSq - 1.0f) < kQuatUnitTolerance;
}

void PrintMatrix(const char* label, const Mat4& m) {
    std::printf("%s\n", label);
    for (int r = 0; r < 4; ++r) {
        std::printf("  [ %9.5f  %9.5f  %9.5f  %9.5f ]\n", m.m[r][0], m.m[r][1], m.m[r][2],
                    m.m[r][3]);
    }
}

nk_float32 MaxAbsDiff(const Mat4& a, const Mat4& b) {
    nk_float32 maxDiff = 0.0f;
    for (int r = 0; r < 4; ++r) {
        for (int c = 0; c < 4; ++c) {
            maxDiff = std::max(maxDiff, std::fabs(a.m[r][c] - b.m[r][c]));
        }
    }
    return maxDiff;
}

void PrintDiffs(const Mat4& a, const Mat4& b) {
    std::printf("Ecarts absolus (InvertGeneral - InvertAnalytical) :\n");
    for (int r = 0; r < 4; ++r) {
        std::printf("  [ %9.6f  %9.6f  %9.6f  %9.6f ]\n", std::fabs(a.m[r][0] - b.m[r][0]),
                    std::fabs(a.m[r][1] - b.m[r][1]), std::fabs(a.m[r][2] - b.m[r][2]),
                    std::fabs(a.m[r][3] - b.m[r][3]));
    }
}

constexpr int kWindowWidth = 900;
constexpr int kWindowHeight = 700;
constexpr float kOriginX = kWindowWidth / 2.0f;
constexpr float kOriginY = kWindowHeight / 2.0f;
constexpr float kScale = 100.0f;
constexpr float kAxisLength = 0.6f;

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

// Dessine les axes portes par une matrice : les colonnes 0/1/2 sont les
// axes X/Y/Z tournes, la colonne 3 (rows 0-2) est l'origine.
void DrawAxesFromMatrix(const Mat4& m, float length) {
    NkVec3f origin = MatColumn3(m, 3);
    NkVec3f axisX = MatColumn3(m, 0) * length;
    NkVec3f axisY = MatColumn3(m, 1) * length;
    NkVec3f axisZ = MatColumn3(m, 2) * length;
    DrawWorldSegment(origin, origin + axisX, 0.9f, 0.2f, 0.2f, 1.0f);
    DrawWorldSegment(origin, origin + axisY, 0.2f, 0.85f, 0.3f, 1.0f);
    DrawWorldSegment(origin, origin + axisZ, 0.25f, 0.45f, 0.95f, 1.0f);
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

}  // namespace

int main() {
    NkVec3f position{};
    NkQuatf orientation{};

    int read = std::scanf("%f %f %f %f %f %f %f", &position.x, &position.y, &position.z,
                           &orientation.x, &orientation.y, &orientation.z, &orientation.w);
    if (read != 7) {
        std::fprintf(stderr,
                      "Usage : sept nombres reels separes par des espaces\n"
                      "  position (x y z)  quaternion (x y z w)\n"
                      "  ex: 1 2 3  0 0 0.7071068 0.7071068\n");
        return EXIT_FAILURE;
    }

    const Pose pose{position, orientation};
    assert(IsUnitQuat(pose.orientation) && "le quaternion lu n'est pas unitaire");

    const Mat4 poseMatrix = PoseToMatrix(pose);
    const Mat4 viaGeneral = InvertGeneral(poseMatrix);
    const Mat4 viaAnalytical = InvertAnalytical(pose);

    assert(MaxAbsDiff(viaGeneral, viaAnalytical) < kMatrixTolerance &&
           "InvertGeneral et InvertAnalytical ne s'accordent pas sur une pose valide");

    std::printf("=== Cas normal (pose valide) ===\n");
    PrintMatrix("InvertGeneral(PoseToMatrix(pose)) :", viaGeneral);
    PrintMatrix("InvertAnalytical(pose) :", viaAnalytical);
    PrintDiffs(viaGeneral, viaAnalytical);
    std::printf("Ecart maximal : %.6f (seuil : %.6f)\n\n", MaxAbsDiff(viaGeneral, viaAnalytical),
                kMatrixTolerance);

    // Matrice corrompue : deux lignes identiques dans le bloc rotation,
    // comme si un bug en amont avait duplique un axe. Une VRAIE pose ne
    // peut jamais produire ca -- ceci simule une corruption, pas une
    // entree utilisateur bizarre.
    Mat4 corrupted{};
    corrupted.m[0][0] = 1; corrupted.m[0][1] = 0; corrupted.m[0][2] = 0; corrupted.m[0][3] = 5;
    corrupted.m[1][0] = 1; corrupted.m[1][1] = 0; corrupted.m[1][2] = 0; corrupted.m[1][3] = 6;
    corrupted.m[2][0] = 0; corrupted.m[2][1] = 0; corrupted.m[2][2] = 1; corrupted.m[2][3] = 7;
    corrupted.m[3][0] = 0; corrupted.m[3][1] = 0; corrupted.m[3][2] = 0; corrupted.m[3][3] = 1;

    nk_float32 corruptedDet = ComputeDeterminant4(corrupted);
    const Mat4 corruptedResult = InvertGeneral(corrupted);
    bool isSilentlyIdentity = MaxAbsDiff(corruptedResult, Mat4Identity()) < 1e-6f;

    std::printf("=== Cas degenere (matrice corrompue, determinant = %.6f) ===\n", corruptedDet);
    PrintMatrix("InvertGeneral(corrupted) :", corruptedResult);
    std::printf(isSilentlyIdentity
                    ? "-> Aucune erreur affichee : la matrice singuliere a ete inversee en\n"
                      "   l'identite SANS AVERTISSEMENT. C'est le mensonge silencieux que le\n"
                      "   chapitre 2 denonce.\n\n"
                    : "-> Une erreur ou un comportement different de l'identite a ete detecte\n"
                      "   (inattendu pour cette demonstration).\n\n");
    std::fflush(stdout);

    if (!glfwInit()) {
        std::fprintf(stderr, "Erreur : impossible d'initialiser GLFW.\n");
        return 1;
    }

    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    GLFWwindow* window = glfwCreateWindow(kWindowWidth, kWindowHeight,
                                           "Exo 7 (ch2) - La matrice de vue", nullptr, nullptr);
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

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        glClearColor(0.08f, 0.08f, 0.10f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // Repere de la pose (monde) et repere de son inverse analytique.
        DrawAxesFromMatrix(poseMatrix, kAxisLength);
        DrawAxesFromMatrix(viaAnalytical, kAxisLength * 0.6f);

        // Indicateur rouge/vert : le cas corrompu a-t-il ete detecte comme
        // degenere ? (Ici toujours rouge par construction du defaut
        // reproduit -- voir Mat4.hpp.)
        if (isSilentlyIdentity) {
            DrawCircle(60.0f, kWindowHeight - 60.0f, 14.0f, 0.9f, 0.2f, 0.2f, 1.0f);
        } else {
            DrawCircle(60.0f, kWindowHeight - 60.0f, 14.0f, 0.2f, 0.85f, 0.3f, 1.0f);
        }

        glfwSwapBuffers(window);
    }

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
