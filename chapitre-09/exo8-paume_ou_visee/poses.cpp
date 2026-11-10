// Paume ou visee : calcule ou le simulateur place la paume et la visee de chaque main, puis dit laquelle des deux
// peut saisir un objet pose devant soi.
// Entree : "H M", H lignes "x y z k" (tete en mm, lacet k en quarts de tour), M lignes "x y z" (objets).
// Repere de la tete (tete droite) : paume droite (180, -300, -250), visee droite (180, -250, -400).
// Main gauche : x change de signe AVANT la rotation. x vers la droite, y vers le haut, l'avant est vers z negatif.
// Passage dans la salle : on tourne (x, z) de k quarts de tour (0 : (x,z) ; 1 : (z,-x) ; 2 : (-x,-z) ; 3 : (-z,x)),
// puis on ajoute la position de la tete, y compris.
// Sortie par tete : TETE h, PAUME D, VISEE D, PAUME G, VISEE G (x y z), SAISIE PAUME j|AUCUNE, SAISIE VISEE j|AUCUNE.
// A la fin : ECART e (distance paume droite - visee droite, arrondie a l'entier le plus proche).
#include <cmath>
#include <iostream>
#include <vector>

struct Vec3 {
    long long x = 0, y = 0, z = 0;
};

// Passe un point du repere de la tete au repere de la salle.
static Vec3 versSalle(const Vec3& local, const Vec3& tete, int k) {
    long long x = 0, z = 0;
    switch (k) {
        case 0: x = local.x;  z = local.z;  break;
        case 1: x = local.z;  z = -local.x; break;
        case 2: x = -local.x; z = -local.z; break;
        default: x = -local.z; z = local.x; break;
    }
    Vec3 r;
    r.x = x + tete.x;
    r.y = local.y + tete.y;
    r.z = z + tete.z;
    return r;
}

static void afficher(const char* nom, const Vec3& p) {
    std::cout << nom << " " << p.x << " " << p.y << " " << p.z << "\n";
}

// Objet le plus proche de p, a 150 mm au plus (distance au carre <= 22500) ; egalite : plus petit numero. 0 si aucun.
static long long plusProche(const Vec3& p, const std::vector<Vec3>& objets) {
    long long meilleur = 0, meilleureDist = 0;
    for (size_t j = 0; j < objets.size(); ++j) {
        const long long dx = objets[j].x - p.x, dy = objets[j].y - p.y, dz = objets[j].z - p.z;
        const long long d2 = dx * dx + dy * dy + dz * dz;
        if (d2 <= 22500 && (meilleur == 0 || d2 < meilleureDist)) {
            meilleur = static_cast<long long>(j) + 1;
            meilleureDist = d2;
        }
    }
    return meilleur;
}

static void afficherSaisie(const char* nom, long long j) {
    if (j == 0) std::cout << nom << " AUCUNE\n";
    else std::cout << nom << " " << j << "\n";
}

int main() {
    long long h = 0, m = 0;
    if (!(std::cin >> h >> m)) { h = 0; m = 0; }

    std::vector<Vec3> tetes(static_cast<size_t>(h));
    std::vector<int> lacets(static_cast<size_t>(h), 0);
    for (long long i = 0; i < h; ++i) {
        long long k = 0;
        std::cin >> tetes[static_cast<size_t>(i)].x >> tetes[static_cast<size_t>(i)].y
                 >> tetes[static_cast<size_t>(i)].z >> k;
        lacets[static_cast<size_t>(i)] = static_cast<int>(((k % 4) + 4) % 4);
    }
    std::vector<Vec3> objets(static_cast<size_t>(m));
    for (long long j = 0; j < m; ++j)
        std::cin >> objets[static_cast<size_t>(j)].x >> objets[static_cast<size_t>(j)].y >> objets[static_cast<size_t>(j)].z;

    const Vec3 paumeD{180, -300, -250}, viseeD{180, -250, -400};
    const Vec3 paumeG{-paumeD.x, paumeD.y, paumeD.z}, viseeG{-viseeD.x, viseeD.y, viseeD.z}; // signe de x avant la rotation

    for (long long i = 0; i < h; ++i) {
        const Vec3& tete = tetes[static_cast<size_t>(i)];
        const int k = lacets[static_cast<size_t>(i)];
        const Vec3 pD = versSalle(paumeD, tete, k), vD = versSalle(viseeD, tete, k);
        const Vec3 pG = versSalle(paumeG, tete, k), vG = versSalle(viseeG, tete, k);
        std::cout << "TETE " << (i + 1) << "\n";
        afficher("PAUME D", pD);
        afficher("VISEE D", vD);
        afficher("PAUME G", pG);
        afficher("VISEE G", vG);
        afficherSaisie("SAISIE PAUME", plusProche(pD, objets));
        afficherSaisie("SAISIE VISEE", plusProche(vD, objets));
    }

    // L'ecart ne depend pas de la tete (rotation et translation conservent les distances) : on le calcule dans le repere local.
    const double dx = static_cast<double>(viseeD.x - paumeD.x), dy = static_cast<double>(viseeD.y - paumeD.y),
                 dz = static_cast<double>(viseeD.z - paumeD.z);
    std::cout << "ECART " << std::llround(std::sqrt(dx * dx + dy * dy + dz * dz)) << "\n";
    return 0;
}
