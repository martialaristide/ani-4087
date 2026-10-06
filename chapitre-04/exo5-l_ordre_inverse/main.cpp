#include <iostream>
#include <string>
#include <cstdlib>

int main() {
    int n = 0;
    std::cin >> n;

    int deplaces = 0;
    int pire = 0;

    for (int i = 0; i < n; ++i) {
        std::string nom;
        int tx = 0;
        int ty = 0;
        int tz = 0;
        int sx = 0;
        int sy = 0;
        int sz = 0;
        std::cin >> nom >> tx >> ty >> tz >> sx >> sy >> sz;

        // Mauvais ordre (echelle * translation) : chaque axe reste chez lui,
        // on multiplie d'abord (sinon la division entiere ecrase la precision),
        // et la division entiere de C++ tronque deja vers zero.
        const int mauvaisX = sx * tx / 1000;
        const int mauvaisY = sy * ty / 1000;
        const int mauvaisZ = sz * tz / 1000;

        const int ecartX = std::abs(tx - mauvaisX);
        const int ecartY = std::abs(ty - mauvaisY);
        const int ecartZ = std::abs(tz - mauvaisZ);

        int ecart = ecartX;
        if (ecartY > ecart) ecart = ecartY;
        if (ecartZ > ecart) ecart = ecartZ;

        if (ecart != 0) {
            ++deplaces;
        }
        if (ecart > pire) {
            pire = ecart;
        }

        std::cout << nom << " " << mauvaisX << " " << mauvaisY << " " << mauvaisZ
                  << " " << ecart << "\n";
    }

    std::cout << "DEPLACES " << deplaces << "\n";
    std::cout << "PIRE " << pire << "\n";

    return 0;
}
