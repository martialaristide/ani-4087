#include <iostream>
#include <string>

struct Point3 {
    int x;
    int y;
    int z;
};

// Pose un objet de hauteur sy au sol : son centre monte a la moitie de sa hauteur.
Point3 PoserAuSol(int x, int z, int sy) {
    return {x, sy / 2, z};
}

// Pose un objet de hauteur sy sur un dessus situe a H : son centre monte de H,
// plus la moitie de sa propre hauteur.
Point3 PoserSurTable(int x, int z, int sy, int H) {
    return {x, H + sy / 2, z};
}

void afficher(const std::string& nom, const Point3& p) {
    std::cout << nom << " " << p.x << " " << p.y << " " << p.z << "\n";
}

int main() {
    int L = 0;
    int P = 0;
    int H = 0;
    int ep = 0;
    int pied = 0;
    int tx = 0;
    int tz = 0;
    std::cin >> L >> P >> H >> ep >> pied >> tx >> tz;

    // Le plateau est le seul morceau dont on connait le dessus (H), pas le centre :
    // son centre est donc en dessous de H, jamais par PoserAuSol ni PoserSurTable.
    const Point3 plateau{tx, H - ep / 2, tz};
    afficher("PLATEAU", plateau);

    // Les quatre pieds s'arretent sous le plateau : leur hauteur est H - ep,
    // et ils reposent au sol, donc PoserAuSol s'applique directement.
    const int hauteurPied = H - ep;
    const int decalageX = L / 2 - pied;
    const int decalageZ = P / 2 - pied;

    const int signesX[4] = {-1, 1, -1, 1};
    const int signesZ[4] = {-1, -1, 1, 1};

    for (int i = 0; i < 4; ++i) {
        const int x = tx + signesX[i] * decalageX;
        const int z = tz + signesZ[i] * decalageZ;
        afficher("PIED", PoserAuSol(x, z, hauteurPied));
    }

    int n = 0;
    std::cin >> n;

    for (int i = 0; i < n; ++i) {
        std::string nom;
        int sx = 0;
        int sy = 0;
        int sz = 0;
        int x = 0;
        int z = 0;
        std::string ou;
        std::cin >> nom >> sx >> sy >> sz >> x >> z >> ou;

        const Point3 centre = (ou == "TABLE") ? PoserSurTable(x, z, sy, H)
                                               : PoserAuSol(x, z, sy);
        afficher(nom, centre);
    }

    return 0;
}
