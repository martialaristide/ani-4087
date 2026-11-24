#include <iostream>
#include <string>
#include <vector>

static bool estSigneDeNom(char c) {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c == '_';
}

// Vrai si le nom apparait entier dans le texte (signes voisins : ni lettre, ni chiffre, ni _).
static bool contientNom(const std::string& texte, const std::string& nom) {
    if (nom.empty()) return false;
    for (size_t p = texte.find(nom); p != std::string::npos; p = texte.find(nom, p + 1)) {
        const bool avantOk = (p == 0) || !estSigneDeNom(texte[p - 1]);
        const size_t fin = p + nom.size();
        const bool apresOk = (fin >= texte.size()) || !estSigneDeNom(texte[fin]);
        if (avantOk && apresOk) return true;
    }
    return false;
}

static bool contientUnDes(const std::string& texte, const std::vector<std::string>& noms) {
    for (const std::string& nom : noms)
        if (contientNom(texte, nom)) return true;
    return false;
}

static std::vector<std::string> lireListe() {
    std::string etiquette;
    long long n = 0;
    std::vector<std::string> noms;
    if (!(std::cin >> etiquette >> n)) return noms;
    for (long long i = 0; i < n; ++i) {
        std::string nom;
        if (!(std::cin >> nom)) break;
        noms.push_back(nom);
    }
    return noms;
}

int main() {
    const std::vector<std::string> champs = lireListe();
    const std::vector<std::string> definis = lireListe();
    long long n = 0;
    std::cin >> n;
    std::string reste;
    std::getline(std::cin, reste);  // fin de la ligne de N

    long long decisions = 0, fuites = 0, definisFautifs = 0;

    for (long long l = 1; l <= n; ++l) {
        std::string ligne;
        if (!std::getline(std::cin, ligne)) break;
        if (!ligne.empty() && ligne.back() == '\r') ligne.pop_back();

        size_t i = 0;
        while (i < ligne.size() && ligne[i] != ' ' && ligne[i] != '\t') ++i;
        const std::string zone = ligne.substr(0, i);
        const std::string texte = (i < ligne.size()) ? ligne.substr(i + 1) : "";

        size_t debut = 0;
        while (debut < texte.size() && (texte[debut] == ' ' || texte[debut] == '\t')) ++debut;
        if (texte.compare(debut, 2, "//") == 0) continue;  // commentaire

        const bool nommeChamp = contientUnDes(texte, champs);
        const bool nommeDefini = contientUnDes(texte, definis);

        if (nommeChamp) {
            if (zone == "INIT") {
                ++decisions;
            } else if (zone == "BOUCLE" || zone == "SCENE") {
                ++fuites;
                std::cout << "FUITE " << l << " " << zone << "\n";
            }
        }
        if (nommeDefini && zone != "FABRIQUE") {
            ++definisFautifs;
            std::cout << "DEFINI " << l << " " << zone << "\n";
        }
    }

    std::cout << "DECISIONS " << decisions << "\n";
    std::cout << "FUITES " << fuites << "\n";
    std::cout << "DEFINIS " << definisFautifs << "\n";
    std::cout << ((fuites == 0 && definisFautifs == 0) ? "ABSTRACTION TENUE" : "ABSTRACTION FUIT") << "\n";
    return 0;
}
