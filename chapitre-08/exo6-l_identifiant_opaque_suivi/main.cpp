// L'identifiant opaque suivi : range des poignees de textures dans une chaine, les relit, et dit si la texture existe encore.
// Entree : "n N" (nombre d'images, nombre d'ordres), puis N ordres : cree h, detruit h, set i h, get i.
// Les poignees et les index sont lus sur 64 bits non signes (jusqu'a 18446744073709551615).
// Sortie : des lignes SET IGNORE / GET au fil des ordres, puis MORTES m et LARGES l.
#include <cstdint>
#include <iostream>
#include <set>
#include <string>
#include <vector>

int main() {
    unsigned long long n = 1;
    long long nbOrdres = 0;
    if (!(std::cin >> n >> nbOrdres)) nbOrdres = 0;

    std::vector<uint64_t> emplacements(static_cast<size_t>(n), 0); // les n emplacements valent 0 au depart
    std::set<uint64_t> textures;                                   // les textures qui existent
    long long mortes = 0, larges = 0;

    for (long long k = 0; k < nbOrdres; ++k) {
        std::string ordre;
        if (!(std::cin >> ordre)) break;

        if (ordre == "cree") {
            uint64_t h = 0;
            std::cin >> h;
            textures.insert(h);   // creer une texture qui existe deja ne change rien
        } else if (ordre == "detruit") {
            uint64_t h = 0;
            std::cin >> h;
            textures.erase(h);    // detruire une texture absente ne change rien
        } else if (ordre == "set") {
            uint64_t i = 0, h = 0;
            std::cin >> i >> h;
            if (i >= n) {
                std::cout << "SET IGNORE " << i << "\n";
            } else {
                emplacements[static_cast<size_t>(i)] = h; // detruire plus tard ne videra pas l'emplacement
                if (h > 4294967295ULL) ++larges;
            }
        } else if (ordre == "get") {
            uint64_t i = 0;
            std::cin >> i;
            const uint64_t h = (i >= n) ? 0 : emplacements[static_cast<size_t>(i)];
            if (h == 0) {
                std::cout << "GET " << i << " 0 VIDE\n";
            } else if (textures.count(h) != 0) {
                // l'existence est verifiee au moment du get
                std::cout << "GET " << i << " " << h << " VIVANTE\n";
            } else {
                std::cout << "GET " << i << " " << h << " MORTE\n";
                ++mortes;
            }
        }
    }

    std::cout << "MORTES " << mortes << "\n";
    std::cout << "LARGES " << larges << "\n";
    return 0;
}
