// Declarer et verifier : declare des actions, attache le jeu d'actions a la session, et dit ce que chaque handle designe.
// Entree : N, puis N ordres : "cree nom TYPE USAGE", "attache", "lit h" (h jusqu'a 4294967295).
// Sortie : une ligne par ordre, puis ACTIONS n, ATTACHEES k et NOMS EN DOUBLE d.
#include <iostream>
#include <map>
#include <set>
#include <string>
#include <vector>

int main() {
    long long nbOrdres = 0;
    if (!(std::cin >> nbOrdres)) nbOrdres = 0;

    // le numero de chaque usage
    const std::map<std::string, int> usages = {
        {"SELECT", 0},        {"GRAB", 1},           {"MENU", 2},           {"MOVE", 3},
        {"AIM_POSE", 4},      {"GRIP_POSE", 5},      {"HAPTIC", 6},         {"SELECT_LEFT", 7},
        {"GRAB_LEFT", 8},     {"AIM_POSE_LEFT", 9},  {"GRIP_POSE_LEFT", 10}, {"HAPTIC_LEFT", 11},
        {"MOVE_RIGHT", 12}};

    std::vector<std::string> noms;       // le nom de l'action de handle h est noms[h - 1]
    std::set<std::string> nomsDejaPris;
    long long nomsEnDouble = 0;
    bool attache = false;
    unsigned long long k = 0;            // le nombre d'actions que la session connait (0 tant qu'il n'y a pas d'attache)

    for (long long i = 0; i < nbOrdres; ++i) {
        std::string ordre;
        if (!(std::cin >> ordre)) break;

        if (ordre == "cree") {
            std::string nom, type, usage;
            std::cin >> nom >> type >> usage;
            noms.push_back(nom);
            const unsigned long long handle = noms.size(); // l'index de l'action plus un : 0 reste invalide
            if (!nomsDejaPris.insert(nom).second) ++nomsEnDouble; // un nom repete ne change rien aux handles
            const auto it = usages.find(usage);
            const int numeroUsage = (it != usages.end()) ? it->second : -1;
            std::cout << "ACTION " << handle << " " << nom << " " << type << " " << numeroUsage << "\n";
        } else if (ordre == "attache") {
            if (attache) {
                std::cout << "ATTACHE REFUSE\n"; // une seule attache, la suivante ne change rien
            } else {
                attache = true;
                k = noms.size(); // la session ne connait que les actions declarees a ce moment
                std::cout << "ATTACHE " << k << "\n";
            }
        } else if (ordre == "lit") {
            unsigned long long h = 0;
            std::cin >> h;
            if (h == 0 || h > noms.size()) {
                std::cout << "LIT " << h << " INVALIDE\n";
            } else if (!attache || h > k) {
                std::cout << "LIT " << h << " INCONNUE\n"; // le handle existe, mais la session ne le connait pas
            } else {
                std::cout << "LIT " << h << " " << noms[h - 1] << "\n";
            }
        }
    }

    std::cout << "ACTIONS " << noms.size() << "\n";
    std::cout << "ATTACHEES " << k << "\n";
    std::cout << "NOMS EN DOUBLE " << nomsEnDouble << "\n";
    return 0;
}
