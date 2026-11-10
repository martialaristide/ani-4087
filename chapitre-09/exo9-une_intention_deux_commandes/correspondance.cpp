// Une intention, deux commandes : rejoue les memes actions avec deux tables de correspondance differentes.
// Seule la table (usage -> commande) change d'une saisie a l'autre ; le reste du code ne nomme aucun bouton.
// Entree : A, puis A lignes "nom USAGE" (actions declarees une seule fois). Puis deux fois :
//   une table "U" suivie de U lignes "USAGE COMMANDE", puis "T" suivi de T lignes "k c1 ... ck" (commandes tenues a l'image).
// Sortie par table i : "TABLE i", "INACTIVE nom" par action dont l'usage manque (dans l'ordre des actions),
//   puis par image "t" suivi des actions venant d'etre appuyees, ou "t -". Enfin "IDENTIQUES OUI|NON".
#include <iostream>
#include <map>
#include <set>
#include <string>
#include <vector>

struct Action {
    std::string nom;
    std::string usage;
};

// Lit une table et ses images, affiche le resultat de la table et renvoie les lignes d'images pour la comparaison.
static std::vector<std::string> jouerTable(int numero, const std::vector<Action>& actions) {
    long long u = 0;
    if (!(std::cin >> u)) u = 0;

    // la table : usage -> commande (seule donnee qui depend du materiel)
    std::map<std::string, std::string> table;
    for (long long i = 0; i < u; ++i) {
        std::string usage, commande;
        std::cin >> usage >> commande;
        table[usage] = commande;
    }

    std::cout << "TABLE " << numero << "\n";
    for (const Action& a : actions) {
        if (table.find(a.usage) == table.end()) std::cout << "INACTIVE " << a.nom << "\n";
    }

    long long t = 0;
    if (!(std::cin >> t)) t = 0;

    std::vector<bool> appuyeAvant(actions.size(), false); // avant la premiere image, rien n'est appuye
    std::vector<std::string> lignes;

    for (long long image = 1; image <= t; ++image) {
        long long k = 0;
        std::cin >> k;
        std::set<std::string> tenues;
        for (long long i = 0; i < k; ++i) {
            std::string commande;
            std::cin >> commande;
            tenues.insert(commande);
        }

        std::string ligne = std::to_string(image);
        bool aucune = true;
        for (size_t i = 0; i < actions.size(); ++i) {
            const auto it = table.find(actions[i].usage);
            const bool appuye = (it != table.end()) && (tenues.count(it->second) > 0); // inactive : jamais appuyee
            if (appuye && !appuyeAvant[i]) {                                           // vient d'etre appuyee
                ligne += " " + actions[i].nom;
                aucune = false;
            }
            appuyeAvant[i] = appuye;
        }
        if (aucune) ligne += " -";

        std::cout << ligne << "\n";
        lignes.push_back(ligne);
    }
    return lignes;
}

int main() {
    long long a = 0;
    if (!(std::cin >> a)) a = 0;

    std::vector<Action> actions(static_cast<size_t>(a));
    for (Action& act : actions) std::cin >> act.nom >> act.usage;

    const std::vector<std::string> premiere = jouerTable(1, actions);
    const std::vector<std::string> seconde = jouerTable(2, actions); // ne garde rien de la premiere

    std::cout << "IDENTIQUES " << (premiere == seconde ? "OUI" : "NON") << "\n";
    return 0;
}
