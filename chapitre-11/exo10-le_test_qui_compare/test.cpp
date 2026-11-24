#include <iostream>

// Test scripte : ni horloge, ni entree. Les memes entrees donnent le meme fichier.
int main() {
    long long hz = 0, ipd = 0, n = 0;
    long long x = 0, y = 0, z = 0, k0 = 0, dk = 0, vx = 0;
    std::cin >> hz >> ipd >> n;
    std::cin >> x >> y >> z >> k0 >> dk >> vx;

    if (hz <= 1) hz = 72;
    const long long periode = 1000000000LL / hz;
    std::cout << "PERIODE " << periode << "\n";

    // Droite de la tete en (x, z) pour un lacet de 0, 1, 2 puis 3 quarts de tour.
    const long long droiteX[4] = {1, 0, -1, 0};
    const long long droiteZ[4] = {0, -1, 0, 1};
    const long long demi = ipd / 2;

    for (long long t = 1; t <= n; ++t) {
        long long k = (k0 + (t - 1) * dk) % 4;
        if (k < 0) k += 4;
        const long long tx = x + (t - 1) * vx;
        const long long gx = tx - demi * droiteX[k];
        const long long gz = z - demi * droiteZ[k];
        const long long dx = tx + demi * droiteX[k];
        const long long dz = z + demi * droiteZ[k];
        std::cout << t << " G " << gx << " " << y << " " << gz
                  << " D " << dx << " " << y << " " << dz
                  << " I " << t * periode << "\n";
    }
    return 0;
}
