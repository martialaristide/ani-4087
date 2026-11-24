#include <iostream>
#include <map>
#include <string>
#include <vector>

struct Reglage {
    bool ecrit = false;
    bool env = false;
    bool lit = false;
    bool getenv = false;
};

int main() {
    long long r = 0;
    if (!(std::cin >> r)) r = 0;

    std::vector<std::string> ordre;  // ordre de premiere apparition
    std::map<std::string, Reglage> reglages;

    for (long long i = 0; i < r; ++i) {
        std::string nom, action;
        if (!(std::cin >> nom >> action)) break;
        if (!reglages.count(nom)) ordre.push_back(nom);
        Reglage& g = reglages[nom];
        if (action == "ECRIT") g.ecrit = true;
        else if (action == "ENV") g.env = true;
        else if (action == "LIT") g.lit = true;
        else if (action == "GETENV") g.getenv = true;
        // DECLARE : le champ existe, mais ce n'est pas une ecriture.
    }

    long long menus = 0;
    for (const std::string& nom : ordre) {
        const Reglage& g = reglages[nom];
        std::string classe;
        if (g.getenv) classe = "CACHE";
        else if (!g.lit) classe = "FANTOME";
        else if (!g.ecrit && !g.env) classe = "CONSTANTE";
        else { classe = "MENU"; ++menus; }
        std::cout << nom << " " << classe << "\n";
    }
    std::cout << "MENU " << menus << " SUR " << ordre.size() << "\n";
    return 0;
}
