#include <iostream>
#include <string>

int main() {
    int W = 0;
    int H = 0;
    int seuil = 0;
    std::cin >> W >> H >> seuil;

    int n = 0;
    std::cin >> n;

    int ok = 0;
    int aReprendre = 0;

    for (int i = 0; i < n; ++i) {
        std::string nom;
        int u = 0;
        int y = 0;
        int l = 0;
        int h = 0;
        int e = 0;
        int d = 0;
        std::cin >> nom >> u >> y >> l >> h >> e >> d;

        const int saillie = d + e / 2;
        const int faceArriere = d - e / 2;

        const bool debordeLargeur = (u - l / 2 < -W / 2) || (u + l / 2 > W / 2);
        const bool debordeHauteur = (y - h / 2 < 0) || (y + h / 2 > H);

        std::string verdict;
        if (debordeLargeur || debordeHauteur) {
            verdict = "DEBORDE";
        } else if (saillie <= 0) {
            verdict = "INVISIBLE";
        } else if (saillie < seuil) {
            verdict = "CLIGNOTE";
        } else if (faceArriere > seuil) {
            verdict = "DECOLLE";
        } else {
            verdict = "OK";
        }

        if (verdict == "OK") {
            ++ok;
        } else {
            ++aReprendre;
        }

        std::cout << nom << " " << saillie << " " << verdict << "\n";
    }

    std::cout << "OK " << ok << "\n";
    std::cout << "A REPRENDRE " << aReprendre << "\n";

    return 0;
}
