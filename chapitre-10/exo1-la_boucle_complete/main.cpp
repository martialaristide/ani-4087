// La boucle complete : relit, image par image, la suite des appels d'une boucle de realite virtuelle
// et trouve la premiere faute de chaque image.
// Entree : T, puis T lignes "n appel1 ... appeln" avec les appels WAIT, BEGIN, END, VUES, PREND_G, PREND_D, LACHE_G, LACHE_D.
// Sortie : "t OK" ou "t FAUTE p CODE" (p = place de l'appel fautif, a partir de 1), puis JUSTES j et FAUTIVES f.
// Chaque image repart de zero : rien n'est attendu, commence ni emprunte. Seule la premiere faute est rapportee.
#include <iostream>
#include <string>
#include <vector>

int main() {
    long long nbImages = 0;
    if (!(std::cin >> nbImages)) nbImages = 0;

    long long justes = 0, fautives = 0;

    for (long long t = 1; t <= nbImages; ++t) {
        long long n = 0;
        std::cin >> n;
        std::vector<std::string> appels(static_cast<size_t>(n));
        for (std::string& a : appels) std::cin >> a;

        bool attendu = false;   // au moins un WAIT avant
        bool commence = false;  // BEGIN accepte
        bool termine = false;   // END accepte
        bool vues = false;      // VUES accepte
        bool emprunte[2] = {false, false}; // une image empruntee par oeil (0 : gauche, 1 : droit)

        long long place = 0;
        std::string code;

        for (long long i = 0; i < n && code.empty(); ++i) {
            const std::string& a = appels[static_cast<size_t>(i)];
            const long long p = i + 1;

            if (termine) { place = p; code = "APRES_END"; break; } // tout appel apres un END accepte

            if (a == "WAIT") {
                if (commence) { place = p; code = "WAIT_APRES_BEGIN"; }
                else attendu = true; // deux WAIT avant le BEGIN ne sont pas une faute
            } else if (a == "BEGIN") {
                if (commence) { place = p; code = "DOUBLE_BEGIN"; }
                else if (!attendu) { place = p; code = "BEGIN_SANS_WAIT"; }
                else commence = true;
            } else if (a == "VUES") {
                if (!attendu) { place = p; code = "VUES_SANS_WAIT"; }
                else vues = true;
            } else if (a == "PREND_G" || a == "PREND_D") {
                const int oeil = (a == "PREND_G") ? 0 : 1;
                if (!commence) { place = p; code = "PREND_SANS_BEGIN"; }       // testes dans cet ordre
                else if (!vues) { place = p; code = "PREND_SANS_VUES"; }
                else if (emprunte[oeil]) { place = p; code = "DOUBLE_PREND"; }
                else emprunte[oeil] = true;
            } else if (a == "LACHE_G" || a == "LACHE_D") {
                const int oeil = (a == "LACHE_G") ? 0 : 1;
                if (!emprunte[oeil]) { place = p; code = "LACHE_SANS_PREND"; }
                else emprunte[oeil] = false;
            } else if (a == "END") {
                if (!commence) { place = p; code = "END_SANS_BEGIN"; }
                else if (emprunte[0] || emprunte[1]) { place = p; code = "END_IMAGE_PRISE"; }
                else termine = true;
            }
        }

        if (code.empty() && !termine) { place = n + 1; code = "SANS_END"; } // fin oubliee, a la place n + 1

        if (code.empty()) {
            std::cout << t << " OK\n";
            ++justes;
        } else {
            std::cout << t << " FAUTE " << place << " " << code << "\n";
            ++fautives;
        }
    }

    std::cout << "JUSTES " << justes << "\n";
    std::cout << "FAUTIVES " << fautives << "\n";
    return 0;
}
