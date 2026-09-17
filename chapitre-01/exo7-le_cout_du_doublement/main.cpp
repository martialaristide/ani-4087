// Chapitre 1, exercice 7 : "Le cout du doublement"
//
// Reprend le programme de l'exercice 6 (meme fenetre, meme boucle de 1000
// images) mais scinde la mesure de chaque image en deux temps distincts :
//
//   T_rendu   : glClear() + dessin du graphique en barres + glfwSwapBuffers().
//               C'est la partie qui, en VR, s'execute deux fois (une fois
//               par oeil).
//   T_logique : glfwPollEvents() + mise a jour des tableaux de mesure et
//               des statistiques. Cette partie ne se double pas en VR.
//
// A la fin des 1000 images, on estime le cout du rendu fait deux fois et ce
// qu'il resterait dans le budget de 11,1 ms (90 Hz).

#include <GLFW/glfw3.h>

#include <algorithm>
#include <chrono>
#include <cstdio>
#include <vector>

namespace {

constexpr int kWindowWidth = 800;
constexpr int kWindowHeight = 600;
constexpr int kFrameCount = 1000;
constexpr double kThresholdMs = 11.0;   // seuil affiche sur le graphique (repere visuel, cf. exo6)
constexpr double kBudgetMs = 11.1;      // budget de reference 90 Hz, utilise pour le calcul final
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

// Dessine les N dernieres images de l'historique sous forme de barres
// empilees : portion basse = T_rendu, portion haute = T_logique.
void DrawGraph(const std::vector<double>& renderTimesMs,
               const std::vector<double>& logicTimesMs) {
    glClearColor(0.08f, 0.08f, 0.10f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    size_t total = renderTimesMs.size();
    size_t visible = std::min(total, static_cast<size_t>(kVisibleBars));
    size_t start = total - visible;

    float graphWidth = kGraphRight - kGraphLeft;
    float barWidth = graphWidth / static_cast<float>(kVisibleBars);

    for (size_t i = 0; i < visible; ++i) {
        double renderMs = renderTimesMs[start + i];
        double logicMs = logicTimesMs[start + i];

        float x0 = kGraphLeft + static_cast<float>(i) * barWidth;
        float x1 = x0 + barWidth * 0.85f;

        float yRenderTop = kGraphBottom + MsToBarHeight(renderMs);
        float yLogicTop = kGraphBottom + MsToBarHeight(renderMs + logicMs);

        // Portion basse : T_rendu (bleu).
        glColor3f(0.25f, 0.55f, 0.9f);
        DrawFilledRect(x0, kGraphBottom, x1, yRenderTop);

        // Portion haute : T_logique (orange), empilee sur le rendu.
        glColor3f(0.95f, 0.6f, 0.2f);
        DrawFilledRect(x0, yRenderTop, x1, yLogicTop);
    }

    // Axe de base.
    glColor3f(0.6f, 0.6f, 0.6f);
    glBegin(GL_LINES);
    glVertex2f(kGraphLeft, kGraphBottom);
    glVertex2f(kGraphRight, kGraphBottom);
    glEnd();

    // Ligne de seuil a 11 ms (repere visuel, cf. exercice 6).
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
        kWindowWidth, kWindowHeight, "Exo 7 - Le cout du doublement", nullptr, nullptr);
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

    std::vector<double> renderTimesMs;
    std::vector<double> logicTimesMs;
    renderTimesMs.reserve(kFrameCount);
    logicTimesMs.reserve(kFrameCount);

    bool closedEarly = false;

    for (int i = 0; i < kFrameCount; ++i) {
        auto tPollStart = std::chrono::high_resolution_clock::now();

        glfwPollEvents();
        if (glfwWindowShouldClose(window)) {
            closedEarly = true;
            break;
        }

        auto tRenderStart = std::chrono::high_resolution_clock::now();
        double pollMs =
            std::chrono::duration<double, std::milli>(tRenderStart - tPollStart).count();

        // --- T_rendu : uniquement l'affichage. ---
        DrawGraph(renderTimesMs, logicTimesMs);
        glfwSwapBuffers(window);

        auto tRenderEnd = std::chrono::high_resolution_clock::now();
        double renderMs =
            std::chrono::duration<double, std::milli>(tRenderEnd - tRenderStart).count();

        // --- T_logique (suite) : mise a jour des mesures. ---
        renderTimesMs.push_back(renderMs);
        // logicTimesMs est mis a jour juste apres, une fois son propre cout connu.

        auto tLogicEnd = std::chrono::high_resolution_clock::now();
        double bookkeepingMs =
            std::chrono::duration<double, std::milli>(tLogicEnd - tRenderEnd).count();

        double logicMs = pollMs + bookkeepingMs;
        logicTimesMs.push_back(logicMs);
    }

    if (!closedEarly) {
        double renderSum = 0.0;
        double logicSum = 0.0;
        for (double v : renderTimesMs) renderSum += v;
        for (double v : logicTimesMs) logicSum += v;

        double renderAvg = renderSum / static_cast<double>(renderTimesMs.size());
        double logicAvg = logicSum / static_cast<double>(logicTimesMs.size());
        double doubledRenderCost = 2.0 * renderAvg;
        double remaining = kBudgetMs - doubledRenderCost - logicAvg;

        std::printf("T_rendu moyen   : %.4f ms\n", renderAvg);
        std::printf("T_logique moyen : %.4f ms\n", logicAvg);
        std::printf("Cout du rendu double (2 x T_rendu) : %.4f ms\n", doubledRenderCost);
        std::printf("Reste pour le reste (budget %.1f ms - rendu double - T_logique) : %.4f ms\n",
                     kBudgetMs, remaining);
        std::fflush(stdout);
    }

    // Garde la fenetre ouverte avec le graphe final fige, pour verification visuelle.
    while (!glfwWindowShouldClose(window)) {
        glfwWaitEvents();
        DrawGraph(renderTimesMs, logicTimesMs);
        glfwSwapBuffers(window);
    }

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
