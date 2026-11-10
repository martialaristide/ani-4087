// Le premier objet saisi : saisit l'objet le plus proche de la paume, le garde exactement la ou il a ete pris
// (attache = inverse(paume) * objet, calculee une seule fois a l'appui) et le fait suivre la main (paume * attache).
// Entree : "M T", M lignes "x y k" (objets), T lignes "x y k b" (paume puis bouton). Vue de dessus, en mm ;
//   k = quarts de tour (sens inverse des aiguilles d'une montre).
// Sortie : "t TIENT j x y k" / "t LIBRE" / "t RATE" par image, puis SAISIES s, RATEES r, puis "OBJET j x y k" par objet.
#include <iostream>
#include <vector>

struct Pose {
    long long x = 0, y = 0;
    int k = 0;
};

// Tourne le vecteur (u, v) de k quarts de tour.
static void tourner(long long u, long long v, int k, long long& ru, long long& rv) {
    switch (k) {
        case 0: ru = u;  rv = v;  break;
        case 1: ru = -v; rv = u;  break;
        case 2: ru = -u; rv = -v; break;
        default: ru = v; rv = -u; break;
    }
}

// A * B : position A + (B tourne de A.k), angle (A.k + B.k) % 4.
static Pose composer(const Pose& a, const Pose& b) {
    Pose r;
    long long u = 0, v = 0;
    tourner(b.x, b.y, a.k, u, v);
    r.x = a.x + u;
    r.y = a.y + v;
    r.k = (a.k + b.k) % 4;
    return r;
}

// Inverse de A : angle (4 - A.k) % 4, position (-A.x, -A.y) tournee de cet angle.
static Pose inverse(const Pose& a) {
    Pose r;
    r.k = (4 - a.k) % 4;
    tourner(-a.x, -a.y, r.k, r.x, r.y);
    return r;
}

int main() {
    long long m = 0, t = 0;
    if (!(std::cin >> m >> t)) { m = 0; t = 0; }

    std::vector<Pose> objets(static_cast<size_t>(m));
    for (long long i = 0; i < m; ++i) {
        Pose& o = objets[static_cast<size_t>(i)];
        std::cin >> o.x >> o.y >> o.k;
        o.k = ((o.k % 4) + 4) % 4;
    }

    int boutonAvant = 0;      // avant l'image 1, b vaut 0
    long long tenu = -1;      // indice de l'objet tenu, -1 si aucun
    Pose attache;
    long long saisies = 0, ratees = 0;

    for (long long i = 1; i <= t; ++i) {
        Pose paume;
        int b = 0;
        std::cin >> paume.x >> paume.y >> paume.k >> b;
        paume.k = ((paume.k % 4) + 4) % 4;

        const bool appui = (b == 1 && boutonAvant == 0);
        const bool relachement = (b == 0 && boutonAvant == 1);
        boutonAvant = b;

        bool rate = false;
        if (relachement) {
            tenu = -1; // l'objet reste ou il est
        } else if (appui) {
            long long meilleur = -1, meilleureDist = 0;
            for (long long j = 0; j < m; ++j) {
                const long long dx = objets[static_cast<size_t>(j)].x - paume.x;
                const long long dy = objets[static_cast<size_t>(j)].y - paume.y;
                const long long d2 = dx * dx + dy * dy;
                if (d2 <= 22500 && (meilleur < 0 || d2 < meilleureDist)) { // egalite : le plus petit numero reste
                    meilleur = j;
                    meilleureDist = d2;
                }
            }
            if (meilleur >= 0) {
                tenu = meilleur;
                attache = composer(inverse(paume), objets[static_cast<size_t>(tenu)]); // une seule fois, a l'appui
                ++saisies;
            } else {
                rate = true;
                ++ratees;
            }
        }

        if (tenu >= 0) {
            objets[static_cast<size_t>(tenu)] = composer(paume, attache);
            const Pose& o = objets[static_cast<size_t>(tenu)];
            std::cout << i << " TIENT " << (tenu + 1) << " " << o.x << " " << o.y << " " << o.k << "\n";
        } else if (rate) {
            std::cout << i << " RATE\n";
        } else {
            std::cout << i << " LIBRE\n";
        }
    }

    std::cout << "SAISIES " << saisies << "\n";
    std::cout << "RATEES " << ratees << "\n";
    for (long long j = 0; j < m; ++j) {
        const Pose& o = objets[static_cast<size_t>(j)];
        std::cout << "OBJET " << (j + 1) << " " << o.x << " " << o.y << " " << o.k << "\n";
    }
    return 0;
}
