// Les deux mains : dit a quelle main et a quelle commande du bureau le simulateur lie chaque action, puis lit ces actions
// image par image.
// Entree : "A T" (nombre d'actions, nombre d'images), puis A lignes "nom USAGE", puis T lignes : k suivi des k commandes tenues.
// Sortie : une ligne "nom MAIN COMMANDE" par action, une ligne par image, puis DROITE d, GAUCHE g et TETE t.
#include <iostream>
#include <map>
#include <set>
#include <sstream>
#include <string>
#include <vector>

struct Liaison {
    std::string main;     // DROITE ou GAUCHE
    std::string commande; // SOURIS_G, ..., FLECHES, IJKL ou AUCUNE
};

// Vaut x,y a partir de quatre commandes : droite, gauche, haut, bas.
static std::string Manche(const std::set<std::string> &tenues, const std::string &droite, const std::string &gauche,
                          const std::string &haut, const std::string &bas) {
    const int x = (tenues.count(droite) ? 1 : 0) - (tenues.count(gauche) ? 1 : 0);
    const int y = (tenues.count(haut) ? 1 : 0) - (tenues.count(bas) ? 1 : 0);
    return std::to_string(x) + "," + std::to_string(y);
}

int main() {
    // la table du simulateur, usage par usage
    const std::map<std::string, Liaison> table = {
        {"SELECT", {"DROITE", "SOURIS_G"}},        {"GRAB", {"DROITE", "SOURIS_D"}},
        {"MENU", {"GAUCHE", "TAB"}},               {"MOVE", {"GAUCHE", "FLECHES"}},
        {"AIM_POSE", {"DROITE", "AUCUNE"}},        {"GRIP_POSE", {"DROITE", "AUCUNE"}},
        {"HAPTIC", {"DROITE", "AUCUNE"}},          {"SELECT_LEFT", {"GAUCHE", "SOURIS_M"}},
        {"GRAB_LEFT", {"GAUCHE", "MAJ_G"}},        {"AIM_POSE_LEFT", {"GAUCHE", "AUCUNE"}},
        {"GRIP_POSE_LEFT", {"GAUCHE", "AUCUNE"}},  {"HAPTIC_LEFT", {"GAUCHE", "AUCUNE"}},
        {"MOVE_RIGHT", {"DROITE", "IJKL"}}};

    int nbActions = 0, nbImages = 0;
    if (!(std::cin >> nbActions >> nbImages)) return 0;

    std::vector<std::string> noms;
    std::vector<Liaison> liaisons;
    long long droite = 0, gauche = 0;
    for (int i = 0; i < nbActions; ++i) {
        std::string nom, usage;
        std::cin >> nom >> usage;
        const auto it = table.find(usage);
        const Liaison l = (it != table.end()) ? it->second : Liaison{"DROITE", "AUCUNE"};
        noms.push_back(nom);
        liaisons.push_back(l);
        if (l.main == "DROITE") ++droite; else ++gauche;
        std::cout << nom << " " << l.main << " " << l.commande << "\n";
    }

    long long imagesTete = 0;
    for (int t = 1; t <= nbImages; ++t) {
        int k = 0;
        std::cin >> k;
        std::set<std::string> tenues;
        for (int j = 0; j < k; ++j) {
            std::string c;
            std::cin >> c;
            if (c.size() == 1 && c[0] >= 'a' && c[0] <= 'z') c[0] = static_cast<char>(c[0] - 'a' + 'A'); // les lettres en majuscule
            tenues.insert(c);
        }

        std::cout << t;
        for (const Liaison &l : liaisons) {
            std::string valeur;
            if (l.commande == "AUCUNE") {
                valeur = "-"; // les poses et la vibration ne se lisent pas ici
            } else if (l.commande == "FLECHES") {
                valeur = Manche(tenues, "F_DROITE", "F_GAUCHE", "F_HAUT", "F_BAS"); // les fleches, pas ZQSD
            } else if (l.commande == "IJKL") {
                valeur = Manche(tenues, "L", "J", "I", "K");
            } else {
                valeur = tenues.count(l.commande) ? "1" : "0";
            }
            std::cout << " " << valeur;
        }
        std::cout << "\n";

        // Z, Q, S, D, W et A deplacent la tete simulee : ils ne comptent pour aucune action
        for (const char *lettre : {"Z", "Q", "S", "D", "W", "A"}) {
            if (tenues.count(lettre)) {
                ++imagesTete;
                break;
            }
        }
    }

    std::cout << "DROITE " << droite << "\n";
    std::cout << "GAUCHE " << gauche << "\n";
    std::cout << "TETE " << imagesTete << "\n";
    return 0;
}
