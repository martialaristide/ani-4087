// Le deplacement au manche : deplace le joueur dans la salle avec l'action MOVE, par un decalage du monde,
// sans jamais ecrire dans la pose de la tete (le lacet k n'est que lu).
// Entree : "v T" (pas en mm par image, manche a fond ; nombre d'images), puis T lignes "k n c1 ... cn"
//   (lacet de la tete en quarts de tour, puis les n fleches tenues pendant l'image).
// Manche : x = +1 si F_DROITE, -1 si F_GAUCHE ; y = +1 si F_HAUT, -1 si F_BAS (deux fleches opposees s'annulent).
// Avant / droite au sol selon le lacet k : 0 : (0,-1)/(1,0) ; 1 : (-1,0)/(0,-1) ; 2 : (0,1)/(-1,0) ; 3 : (1,0)/(0,1).
// A chaque image : decalage += v * (y * avant + x * droite). Le decalage vaut (0, 0) au depart.
// Sortie : "t x,y X Z" par image, puis "DECALAGE X Z" et "DIAGONALES d" (images ou x et y sont tous deux non nuls).
#include <iostream>
#include <set>
#include <string>

int main() {
    long long v = 0, t = 0;
    if (!(std::cin >> v >> t)) { v = 0; t = 0; }

    static const int avantX[4] = {0, -1, 0, 1};
    static const int avantZ[4] = {-1, 0, 1, 0};
    static const int droiteX[4] = {1, 0, -1, 0};
    static const int droiteZ[4] = {0, -1, 0, 1};

    long long decalageX = 0, decalageZ = 0; // le monde est decale ; la tete n'est jamais modifiee
    long long diagonales = 0;

    for (long long image = 1; image <= t; ++image) {
        long long k = 0, n = 0;
        std::cin >> k >> n;
        std::set<std::string> fleches;
        for (long long i = 0; i < n; ++i) {
            std::string f;
            std::cin >> f;
            fleches.insert(f);
        }
        const int lacet = static_cast<int>(((k % 4) + 4) % 4); // lu seulement, jamais ecrit

        int x = 0, y = 0;
        if (fleches.count("F_DROITE")) x += 1;
        if (fleches.count("F_GAUCHE")) x -= 1;
        if (fleches.count("F_HAUT")) y += 1;
        if (fleches.count("F_BAS")) y -= 1;
        if (x != 0 && y != 0) ++diagonales;

        decalageX += v * (y * avantX[lacet] + x * droiteX[lacet]);
        decalageZ += v * (y * avantZ[lacet] + x * droiteZ[lacet]);

        std::cout << image << " " << x << "," << y << " " << decalageX << " " << decalageZ << "\n";
    }

    std::cout << "DECALAGE " << decalageX << " " << decalageZ << "\n";
    std::cout << "DIAGONALES " << diagonales << "\n";
    return 0;
}
