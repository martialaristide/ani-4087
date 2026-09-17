// Chapitre 1, exercice 6 : "La pire image"
//
// Boucle de rendu reelle (fenetre + contexte OpenGL) qui n'efface que
// l'ecran, chronometree image par image. On mesure la duree de la PIRE
// image sur 1000 images (pas la cadence moyenne), et le nombre d'images
// qui depassent 11 ms -- le budget approximatif d'un casque a 90 Hz.
//
// Le VSync est desactive volontairement : on ne veut pas mesurer le taux
// de rafraichissement de l'ecran, mais le cout reel de la boucle
// (glClear + swap), pour reveler les a-coups (page faults, ordonnanceur,
// pilote graphique...).

#include <GLFW/glfw3.h>

#include <algorithm>
#include <chrono>
#include <cstdio>
#include <vector>

namespace {

constexpr int kWindowWidth = 800;
constexpr int kWindowHeight = 600;
constexpr int kFrameCount = 1000;
constexpr double kThresholdMs = 11.0;
constexpr double kGraphScaleMs = 30.0;  // pleine hauteur du graphe = 30 ms
constexpr int kVisibleBars = 200;       // barres affichees a l'ecran

// Zone du graphe, en pixels, origine en bas a gauche (voir glOrtho ci-dessous).
constexpr float kGraphLeft = 50.0f;
constexpr float kGraphRight = 750.0f;
constexpr float kGraphBottom = 50.0f;
constexpr float kGraphTop = 550.0f;

float MsToBarHeight(double ms) {
    double clamped = std::min(ms, kGraphScaleMs);
    return static_cast<float>((clamped / kGraphScaleMs) * (kGraphTop - kGraphBottom));
}

void DrawFilledRect(float x0, float y0, float x1, float y1) {
    glBegin(GL_QUADS);
    glVertex2f(x0, y0);
    glVertex2f(x1, y0);
    glVertex2f(x1, y1);
    glVertex2f(x0, y1);
    glEnd();
}

// Dessine les N dernieres images de l'historique, plus la ligne de seuil.
void DrawGraph(const std::vector<double>& frameTimesMs) {
    glClearColor(0.08f, 0.08f, 0.10f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    size_t total = frameTimesMs.size();
    size_t visible = std::min(total, static_cast<size_t>(kVisibleBars));
    size_t start = total - visible;

    float graphWidth = kGraphRight - kGraphLeft;
    float barWidth = graphWidth / static_cast<float>(kVisibleBars);

    for (size_t i = 0; i < visible; ++i) {
        double ms = frameTimesMs[start + i];
        float x0 = kGraphLeft + static_cast<float>(i) * barWidth;
        float x1 = x0 + barWidth * 0.85f;
        float y1 = kGraphBottom + MsToBarHeight(ms);

        if (ms > kThresholdMs) {
            glColor3f(0.85f, 0.15f, 0.15f);
        } else {
            glColor3f(0.15f, 0.75f, 0.25f);
        }
        DrawFilledRect(x0, kGraphBottom, x1, y1);
    }

    // Axe de base.
    glColor3f(0.6f, 0.6f, 0.6f);
    glBegin(GL_LINES);
    glVertex2f(kGraphLeft, kGraphBottom);
    glVertex2f(kGraphRight, kGraphBottom);
    glEnd();

    // Ligne de seuil a 11 ms.
    float thresholdY = kGraphBottom + MsToBarHeight(kThresholdMs);
    glColor3f(0.95f, 0.85f, 0.2f);
    glBegin(GL_LINES);
    glVertex2f(kGraphLeft, thresholdY);
    glVertex2f(kGraphRight, thresholdY);
    glEnd();
}

}  // namespace

int main() {
    if (!glfwInit()) {
        std::fprintf(stderr, "Erreur : impossible d'initialiser GLFW.\n");
        return 1;
    }

    GLFWwindow* window = glfwCreateWindow(
        kWindowWidth, kWindowHeight, "Exo 6 - La pire image", nullptr, nullptr);
    if (!window) {
        std::fprintf(stderr, "Erreur : impossible de creer la fenetre.\n");
        glfwTerminate();
        return 1;
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(0);  // VSync desactive : on mesure le cout reel, pas l'ecran.

    glViewport(0, 0, kWindowWidth, kWindowHeight);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0.0, kWindowWidth, 0.0, kWindowHeight, -1.0, 1.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    std::vector<double> frameTimesMs;
    frameTimesMs.reserve(kFrameCount);

    double worstMs = 0.0;
    int overThresholdCount = 0;
    bool closedEarly = false;

    for (int i = 0; i < kFrameCount; ++i) {
        auto t0 = std::chrono::high_resolution_clock::now();

        glfwPollEvents();
        if (glfwWindowShouldClose(window)) {
            closedEarly = true;
            break;
        }

        DrawGraph(frameTimesMs);
        glfwSwapBuffers(window);

        auto t1 = std::chrono::high_resolution_clock::now();
        double ms = std::chrono::duration<double, std::milli>(t1 - t0).count();

        frameTimesMs.push_back(ms);
        worstMs = std::max(worstMs, ms);
        if (ms > kThresholdMs) {
            ++overThresholdCount;
        }
    }

    if (!closedEarly) {
        std::printf("Pire image (sur %d) : %.3f ms\n", kFrameCount, worstMs);
        std::printf("Images au-dessus de %.0f ms : %d / %d\n", kThresholdMs,
                     overThresholdCount, kFrameCount);
        std::printf(worstMs <= kThresholdMs && overThresholdCount == 0
                         ? "=> Tiendrait dans un casque (aucun depassement du budget "
                           "11 ms).\n"
                         : "=> Ne tiendrait PAS dans un casque en l'etat (au moins une "
                           "image depasse 11 ms).\n");
        std::fflush(stdout);
    }

    // Garde la fenetre ouverte avec le graphe final fige, pour verification visuelle.
    while (!glfwWindowShouldClose(window)) {
        glfwWaitEvents();
        DrawGraph(frameTimesMs);
        glfwSwapBuffers(window);
    }

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
