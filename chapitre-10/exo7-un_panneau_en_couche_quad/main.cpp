// Un panneau en couche quad : place des panneaux en couche quad dans la salle, calcule leurs quatre coins dans STAGE
// et leur distance a l'oeil.
// Entree : "ax ay az ak" (ancre de l'espace LOCAL, exprimee dans STAGE), "ex ey ez" (oeil dans STAGE), N, puis N lignes
//   "E x y z k w h" : E = S (STAGE), L (LOCAL) ou - (non precise : dans ce module, un panneau sans espace est dans LOCAL) ;
//   centre x y z en mm, lacet k en quarts de tour, largeur w et hauteur h en mm (pairs).
// Regle : (x, z) tourne de k quarts de tour : 0 : (x,z) ; 1 : (z,-x) ; 2 : (-x,-z) ; 3 : (-z,x) ; y ne tourne pas.
//   Centre dans STAGE : tel quel pour S ; pour L, ancre + (centre tourne de ak), lacet (ak + k) % 4.
//   Coins avant rotation, dans cet ordre : (-w/2,-h/2), (w/2,-h/2), (w/2,h/2), (-w/2,h/2) en (x, y), z = 0.
//   Chaque coin dans STAGE : (x, z) tourne du lacet du centre, plus le centre ; son y = y du centre + y du coin.
//   DISTANCE : du centre a l'oeil, arrondie a l'entier le plus proche.
// Sortie par panneau : "PANNEAU i STAGE|LOCAL", quatre coins "x y z", "DISTANCE d". A la fin : "PANNEAUX n".
#include <cmath>
#include <iostream>

// Tourne (x, z) de k quarts de tour ; y ne tourne pas.
static void tourner(long long x, long long z, int k, long long& rx, long long& rz) {
    switch (k) {
        case 0: rx = x;  rz = z;  break;
        case 1: rx = z;  rz = -x; break;
        case 2: rx = -x; rz = -z; break;
        default: rx = -z; rz = x; break;
    }
}

static int lacet(long long k) { return static_cast<int>(((k % 4) + 4) % 4); }

int main() {
    long long ax = 0, ay = 0, az = 0, akBrut = 0, ex = 0, ey = 0, ez = 0, n = 0;
    std::cin >> ax >> ay >> az >> akBrut >> ex >> ey >> ez;
    if (!(std::cin >> n)) n = 0;
    const int ak = lacet(akBrut);

    for (long long i = 1; i <= n; ++i) {
        char espace = '-';
        long long x = 0, y = 0, z = 0, kBrut = 0, w = 0, h = 0;
        std::cin >> espace >> x >> y >> z >> kBrut >> w >> h;
        int k = lacet(kBrut);

        const bool local = (espace != 'S'); // sans espace precise ('-'), le module met le panneau dans LOCAL
        long long cx = x, cy = y, cz = z;
        if (local) {
            long long rx = 0, rz = 0;
            tourner(x, z, ak, rx, rz);      // on tourne la position du panneau avant d'ajouter l'ancre
            cx = ax + rx;
            cy = ay + y;
            cz = az + rz;
            k = (ak + k) % 4;
        }

        std::cout << "PANNEAU " << i << " " << (local ? "LOCAL" : "STAGE") << "\n";

        const long long coinsX[4] = {-w / 2, w / 2, w / 2, -w / 2};
        const long long coinsY[4] = {-h / 2, -h / 2, h / 2, h / 2};
        for (int c = 0; c < 4; ++c) {
            long long rx = 0, rz = 0;
            tourner(coinsX[c], 0, k, rx, rz); // lacet du centre ; la hauteur du coin ne tourne jamais
            std::cout << (cx + rx) << " " << (cy + coinsY[c]) << " " << (cz + rz) << "\n";
        }

        const double dx = static_cast<double>(cx - ex), dy = static_cast<double>(cy - ey), dz = static_cast<double>(cz - ez);
        std::cout << "DISTANCE " << std::llround(std::sqrt(dx * dx + dy * dy + dz * dz)) << "\n";
    }

    std::cout << "PANNEAUX " << n << "\n";
    return 0;
}
