// Chapitre 1, exercice 12 : "Vingt millisecondes senties"
//
// Un curseur retarde suit la souris avec un delai reglable (0 a 200 ms).
// Le but n'est pas de mesurer une machine mais de mesurer une personne :
// a partir de quel retard quelqu'un dit-il sentir que le curseur traine ?
//
// Aucune police de caracteres n'est utilisee : le retard actuellement
// regle se lit uniquement sur la position d'un curseur mobile par rapport
// aux graduations de la barre du haut (coherent avec le style des
// exercices 6 et 7 de ce depot).

#include <GLFW/glfw3.h>

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdio>
#include <deque>
#include <utility>
#include <vector>

namespace {

constexpr int kWindowWidth = 900;
constexpr int kWindowHeight = 700;

constexpr double kMinDelayMs = 0.0;
constexpr double kMaxDelayMs = 200.0;
constexpr double kDelayStepMs = 5.0;
constexpr double kPurgeAgeMs = 250.0;  // marge de securite au-dela des 200 ms max
constexpr size_t kTrailLength = 18;

// Barre de reglage graduee (0-200 ms), en haut de la fenetre.
constexpr float kBarLeft = 60.0f;
constexpr float kBarRight = 840.0f;
constexpr float kBarY = 40.0f;
constexpr int kTickCount = 11;  // 0, 20, 40 ... 200

using Clock = std::chrono::high_resolution_clock;

struct Sample {
    double timeMs;
    double x;
    double y;
};

struct AppState {
    double delayMs = 0.0;
    bool clearTrailRequested = false;
};

double NowMs(const Clock::time_point& start) {
    return std::chrono::duration<double, std::milli>(Clock::now() - start).count();
}

// Interpole la position enregistree dans l'historique la plus proche de
// targetMs. Si targetMs est hors de la plage connue, on renvoie
// l'echantillon le plus proche disponible (pas d'extrapolation).
Sample FindDelayedSample(const std::deque<Sample>& history, double targetMs) {
    if (history.empty()) {
        return {targetMs, 0.0, 0.0};
    }
    if (targetMs <= history.front().timeMs) {
        return history.front();
    }
    if (targetMs >= history.back().timeMs) {
        return history.back();
    }
    for (size_t i = 0; i + 1 < history.size(); ++i) {
        const Sample& s0 = history[i];
        const Sample& s1 = history[i + 1];
        if (s0.timeMs <= targetMs && targetMs <= s1.timeMs) {
            double span = s1.timeMs - s0.timeMs;
            double alpha = (span > 0.0) ? (targetMs - s0.timeMs) / span : 0.0;
            double x = s0.x + (s1.x - s0.x) * alpha;
            double y = s0.y + (s1.y - s0.y) * alpha;
            return {targetMs, x, y};
        }
    }
    return history.back();
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

// Barre de reglage : ligne graduee 0-200 ms + curseur triangulaire mobile
// indiquant le retard actuellement regle. Aucun chiffre affiche.
void DrawDelayBar(double delayMs) {
    glColor4f(0.7f, 0.7f, 0.7f, 1.0f);
    glBegin(GL_LINES);
    glVertex2f(kBarLeft, kBarY);
    glVertex2f(kBarRight, kBarY);
    glEnd();

    glBegin(GL_LINES);
    for (int i = 0; i < kTickCount; ++i) {
        float t = static_cast<float>(i) / static_cast<float>(kTickCount - 1);
        float x = kBarLeft + t * (kBarRight - kBarLeft);
        glVertex2f(x, kBarY - 8.0f);
        glVertex2f(x, kBarY + 8.0f);
    }
    glEnd();

    float t = static_cast<float>((delayMs - kMinDelayMs) / (kMaxDelayMs - kMinDelayMs));
    float markerX = kBarLeft + t * (kBarRight - kBarLeft);

    glColor4f(0.95f, 0.85f, 0.2f, 1.0f);
    glBegin(GL_TRIANGLES);
    glVertex2f(markerX, kBarY - 4.0f);
    glVertex2f(markerX - 7.0f, kBarY - 18.0f);
    glVertex2f(markerX + 7.0f, kBarY - 18.0f);
    glEnd();
}

void DrawScene(double delayMs, const std::deque<std::pair<double, double>>& trail,
               double delayedX, double delayedY, double realX, double realY) {
    glClearColor(0.08f, 0.08f, 0.10f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    DrawDelayBar(delayMs);

    // Trainee du curseur retarde : plus ancien = plus transparent.
    size_t n = trail.size();
    for (size_t i = 0; i < n; ++i) {
        float alpha = 0.5f * (static_cast<float>(i) + 1.0f) / static_cast<float>(n);
        DrawCircle(static_cast<float>(trail[i].first), static_cast<float>(trail[i].second),
                   6.0f, 0.95f, 0.6f, 0.15f, alpha);
    }

    // Curseur retarde (orange, au-dessus de la trainee).
    DrawCircle(static_cast<float>(delayedX), static_cast<float>(delayedY), 9.0f, 0.95f, 0.6f,
               0.15f, 1.0f);

    // Curseur reel (blanc, reference).
    DrawCircle(static_cast<float>(realX), static_cast<float>(realY), 6.0f, 1.0f, 1.0f, 1.0f,
               1.0f);
}

void KeyCallback(GLFWwindow* window, int key, int /*scancode*/, int action, int /*mods*/) {
    auto* state = static_cast<AppState*>(glfwGetWindowUserPointer(window));
    if (!state) {
        return;
    }

    bool pressedOrRepeat = (action == GLFW_PRESS || action == GLFW_REPEAT);

    if (pressedOrRepeat &&
        (key == GLFW_KEY_UP || key == GLFW_KEY_EQUAL || key == GLFW_KEY_KP_ADD)) {
        state->delayMs = std::min(kMaxDelayMs, state->delayMs + kDelayStepMs);
    }
    if (pressedOrRepeat &&
        (key == GLFW_KEY_DOWN || key == GLFW_KEY_MINUS || key == GLFW_KEY_KP_SUBTRACT)) {
        state->delayMs = std::max(kMinDelayMs, state->delayMs - kDelayStepMs);
    }
    if (action == GLFW_PRESS && key == GLFW_KEY_ENTER) {
        std::printf("Seuil enregistre : %.0f ms\n", state->delayMs);
        std::fflush(stdout);
    }
    if (action == GLFW_PRESS && key == GLFW_KEY_R) {
        state->delayMs = 0.0;
        state->clearTrailRequested = true;
    }
}

}  // namespace

int main() {
    if (!glfwInit()) {
        std::fprintf(stderr, "Erreur : impossible d'initialiser GLFW.\n");
        return 1;
    }

    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    GLFWwindow* window = glfwCreateWindow(
        kWindowWidth, kWindowHeight, "Exo 12 - Vingt millisecondes senties", nullptr, nullptr);
    if (!window) {
        std::fprintf(stderr, "Erreur : impossible de creer la fenetre.\n");
        glfwTerminate();
        return 1;
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);  // VSync : outil interactif, pas un banc de mesure.

    AppState state;
    glfwSetWindowUserPointer(window, &state);
    glfwSetKeyCallback(window, KeyCallback);

    glViewport(0, 0, kWindowWidth, kWindowHeight);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    // Origine en haut a gauche, y vers le bas : correspond directement aux
    // coordonnees de glfwGetCursorPos, aucune conversion necessaire.
    glOrtho(0.0, kWindowWidth, kWindowHeight, 0.0, -1.0, 1.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    const Clock::time_point startTime = Clock::now();
    std::deque<Sample> history;
    std::deque<std::pair<double, double>> trail;

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        if (state.clearTrailRequested) {
            trail.clear();
            state.clearTrailRequested = false;
        }

        double nowMs = NowMs(startTime);

        double mx = 0.0, my = 0.0;
        glfwGetCursorPos(window, &mx, &my);
        history.push_back({nowMs, mx, my});

        while (history.size() >= 2 && history[1].timeMs < nowMs - kPurgeAgeMs) {
            history.pop_front();
        }

        double targetMs = nowMs - state.delayMs;
        Sample delayed = FindDelayedSample(history, targetMs);

        trail.push_back({delayed.x, delayed.y});
        if (trail.size() > kTrailLength) {
            trail.pop_front();
        }

        DrawScene(state.delayMs, trail, delayed.x, delayed.y, mx, my);
        glfwSwapBuffers(window);
    }

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
