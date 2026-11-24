#include <iostream>
#include <string>

int main() {
    long long k = 0;
    if (!(std::cin >> k) || k <= 0) {
        std::cout << "AUCUNE MACHINE\n";
        return 0;
    }

    for (long long i = 1; i <= k; ++i) {
        std::string d, v, l, r, b, g;
        if (!(std::cin >> d >> v >> l >> r >> b >> g)) break;

        long long chargees = 0, liberees = 0;
        bool bibliothequeTenue = false;  // une bibliotheque est chargee en ce moment
        bool pointEntree = false;
        std::string chemin;
        bool liaisonReussie = false;

        if (d == "SIMULATEUR") {
            std::cout << i << " BACKEND SIMULATEUR DEMANDE\n";
        } else {
            // La variable d'environnement.
            if (v == "ABSENT") {
                std::cout << i << " VARIABLE IGNOREE\n";
            } else if (v != "-") {
                std::cout << i << " VARIABLE CHARGEE\n";
                ++chargees;
                bibliothequeTenue = true;
                if (v == "BON") { pointEntree = true; chemin = "VARIABLE"; }
            }

            // Le loader standard, seulement si rien n'est charge a ce moment.
            if (!bibliothequeTenue) {
                if (l == "ABSENT") {
                    std::cout << i << " LOADER ABSENT\n";
                } else {
                    std::cout << i << " LOADER CHARGE\n";
                    ++chargees;
                    bibliothequeTenue = true;
                    if (l == "BON") { pointEntree = true; chemin = "LOADER"; }
                }
            }

            // Une bibliotheque chargee sans point d'entree est liberee.
            if (bibliothequeTenue && !pointEntree) {
                std::cout << i << " SANS_ENTREE\n";
                ++liberees;
                bibliothequeTenue = false;
            }

            // La decouverte directe, s'il n'y a toujours pas de point d'entree.
            if (!pointEntree) {
                std::cout << i << " DIRECT " << r << "\n";
                if (r == "NEGO_KO" || r == "BON") {
                    ++chargees;
                    bibliothequeTenue = true;
                    if (r == "BON") { pointEntree = true; chemin = "DIRECT"; }
                }
            }

            if (!pointEntree) {
                std::cout << i << " BACKEND INDISPONIBLE\n";
                if (bibliothequeTenue) { ++liberees; bibliothequeTenue = false; }
            } else {
                std::cout << i << " BACKEND OPENXR " << chemin << "\n";
                if (b == "NON") {
                    std::cout << i << " LIAISON ECHOUEE\n";
                    ++liberees;
                    bibliothequeTenue = false;
                } else {
                    liaisonReussie = true;
                }
            }
        }

        if (liaisonReussie)
            std::cout << i << " NIVEAU 1 CASQUE\n";
        else if (g == "OUI")
            std::cout << i << " NIVEAU 2 SIMULATEUR CARTE\n";
        else
            std::cout << i << " NIVEAU 3 SIMULATEUR LOGICIEL\n";
        std::cout << i << " CHARGEES " << chargees << " LIBEREES " << liberees << "\n";
    }
    return 0;
}
