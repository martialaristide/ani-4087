#include <iostream>
#include <string>
#include <cstdlib>

int main() {
    int n = 0;
    std::cin >> n;

    int aCorriger = 0;
    int pire = 0;

    for (int i = 0; i < n; ++i) {
        std::string nom;
        int e = 0;
        int y = 0;
        std::cin >> nom >> e >> y;

        int demi = e / 2;
        int bas = y - demi;
        int haut = y + demi;

        std::string verdict;
        if (haut <= 0) {
            verdict = "SOUS LE SOL";
        } else if (bas < 0) {
            verdict = "ENTERRE";
        } else if (bas == 0) {
            verdict = "POSE";
        } else {
            verdict = "FLOTTE";
        }

        if (verdict != "POSE") {
            ++aCorriger;
        }

        int ecart = std::abs(bas);
        if (ecart > pire) {
            pire = ecart;
        }

        std::cout << nom << " " << bas << " " << haut << " " << verdict << " " << demi << "\n";
    }

    std::cout << "A CORRIGER " << aCorriger << "\n";
    std::cout << "PIRE " << pire << "\n";

    return 0;
}
