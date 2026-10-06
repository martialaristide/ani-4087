// =============================================================================
// Exercice — UN CUBE ET RIEN D'AUTRE (chapitre 4, exo 2)
// Diagnostique, dans un ordre fixe, pourquoi un cube ne s'affiche pas.
// =============================================================================
#include <iostream>
#include <string>

// Les sous-systemes sont des bits ; NK_SS_RENDER3D vaut 2. Le masque complet
// (NK_SS_ALL = 4294967295) deborde un int signe de 32 bits : il faut un type
// non signe d'au moins 32 bits pour le lire sans corruption silencieuse.
using Drapeaux = unsigned long long;
constexpr Drapeaux BIT_RENDER3D = 2;

int main() {
    std::ios::sync_with_stdio(false);

    int n = 0;
    if (!(std::cin >> n) || n < 0) {
        return 0;
    }

    int visibles = 0;
    int enPanne = 0;

    for (int i = 0; i < n; ++i) {
        std::string nom;
        Drapeaux drapeaux = 0;
        long long sx = 0, sy = 0, sz = 0;
        long long distance = 0;
        long long lumieres = 0;
        long long ambiante = 0;
        long long proche = 0;

        std::cin >> nom >> drapeaux >> sx >> sy >> sz
                 >> distance >> lumieres >> ambiante >> proche;

        std::string verdict;

        if ((drapeaux & BIT_RENDER3D) == 0) {
            verdict = "RENDER3D ETEINT";
        } else if (sx == 0 || sy == 0 || sz == 0) {
            verdict = "ECHELLE NULLE";
        } else {
            const long long faceAvant = distance - sz / 2;

            if (faceAvant <= 0) {
                verdict = "CAMERA DANS LE CUBE";
            } else if (faceAvant < proche) {
                verdict = "COUPE PAR LE PLAN PROCHE";
            } else if (lumieres == 0 && ambiante == 0) {
                verdict = "PAS DE LUMIERE";
            } else {
                verdict = "VISIBLE";
            }
        }

        if (verdict == "VISIBLE") {
            ++visibles;
        } else {
            ++enPanne;
        }

        std::cout << nom << ' ' << verdict << '\n';
    }

    std::cout << "VISIBLES " << visibles << '\n';
    std::cout << "EN PANNE " << enPanne << '\n';

    return 0;
}
