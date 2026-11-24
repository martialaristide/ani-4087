// La premiere passe : lit un fichier source ligne par ligne, s'arrete a la fin de son en-tete, et dit ce que cet
// en-tete annonce.
// Entree : N (nombre de lignes du fichier), puis les N lignes telles quelles (vides, indentees, ou avec n'importe quel texte).
// Regle : l'en-tete est le bloc de lignes du haut qui commencent par "//" une fois otes les espaces et tabulations du
//   debut. Il s'arrete a la premiere ligne qui ne commence pas ainsi (une ligne vide l'arrete) ; tout ce qui suit est
//   ignore. Une variable est un nom NK_ suivi d'au moins un signe parmi A-Z, 0-9, _ , suivi tout de suite d'un "=" ;
//   le signe juste avant NK_ n'est ni une lettre, ni un chiffre, ni _. On ne cherche que dans l'en-tete, dans l'ordre de
//   la premiere apparition, chaque variable une seule fois. PART = 100 * e / N, arrondi a l'entier inferieur.
// Sortie : ENTETE e, IGNOREES i, une ligne VARIABLE nom par variable (ou VARIABLE AUCUNE), PART p.
#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>

static bool estLettreOuChiffre(char c) {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9');
}

static bool estSigneDeNom(char c) {
    return (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c == '_';
}

// Ajoute a "variables" les noms annonces dans la ligne, sans doublon, dans l'ordre d'apparition.
static void chercherVariables(const std::string& ligne, std::vector<std::string>& variables) {
    for (size_t p = ligne.find("NK_"); p != std::string::npos; p = ligne.find("NK_", p + 1)) {
        if (p > 0 && (estLettreOuChiffre(ligne[p - 1]) || ligne[p - 1] == '_')) continue; // NK_ colle a un autre nom
        size_t j = p + 3;
        while (j < ligne.size() && estSigneDeNom(ligne[j])) ++j;
        if (j == p + 3) continue;                       // au moins un signe apres NK_
        if (j >= ligne.size() || ligne[j] != '=') continue; // le nom doit etre suivi tout de suite d'un =
        const std::string nom = ligne.substr(p, j - p);
        bool dejaVu = false;
        for (const std::string& v : variables) {
            if (v == nom) { dejaVu = true; break; }
        }
        if (!dejaVu) variables.push_back(nom);
    }
}

int main() {
    std::string premiere;
    long long n = 0;
    if (std::getline(std::cin, premiere)) n = std::atoll(premiere.c_str());

    long long entete = 0;
    bool dansEntete = true;
    std::vector<std::string> variables;

    for (long long i = 0; i < n; ++i) {
        std::string ligne;
        if (!std::getline(std::cin, ligne)) break;
        if (!ligne.empty() && ligne.back() == '\r') ligne.pop_back(); // fichiers ecrits sous Windows

        if (!dansEntete) continue; // tout ce qui suit l'en-tete est ignore, meme un autre commentaire

        size_t debut = 0;
        while (debut < ligne.size() && (ligne[debut] == ' ' || ligne[debut] == '\t')) ++debut;
        if (ligne.compare(debut, 2, "//") == 0) {
            ++entete;
            chercherVariables(ligne, variables);
        } else {
            dansEntete = false; // premiere ligne qui ne commence pas par // (une ligne vide aussi)
        }
    }

    if (n <= 0) {
        std::cout << "ENTETE 0\nIGNOREES 0\nVARIABLE AUCUNE\nPART 0\n";
        return 0;
    }

    std::cout << "ENTETE " << entete << "\n";
    std::cout << "IGNOREES " << (n - entete) << "\n";
    if (variables.empty()) {
        std::cout << "VARIABLE AUCUNE\n";
    } else {
        for (const std::string& v : variables) std::cout << "VARIABLE " << v << "\n";
    }
    std::cout << "PART " << (100 * entete / n) << "\n";
    return 0;
}
