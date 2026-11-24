#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>

static bool lireLigne(std::string& ligne) {
    if (!std::getline(std::cin, ligne)) return false;
    if (!ligne.empty() && ligne.back() == '\r') ligne.pop_back();
    return true;
}

// Recopie la valeur signe par signe, en sautant un antislash suivi d'un autre.
static std::string nettoyer(const std::string& brut) {
    std::string sortie;
    for (size_t i = 0; i < brut.size(); ++i) {
        if (brut[i] == '\\' && i + 1 < brut.size() && brut[i + 1] == '\\') continue;
        sortie += brut[i];
    }
    return sortie;
}

static bool estSeparateur(char c) { return c == '\\' || c == '/'; }

int main() {
    std::string ligne;
    if (!lireLigne(ligne)) return 0;
    const long long k = std::atoll(ligne.c_str());

    for (long long i = 1; i <= k; ++i) {
        std::string registre;
        if (!lireLigne(registre)) break;
        long long l = 0;
        if (lireLigne(ligne)) l = std::atoll(ligne.c_str());

        std::string texte;
        for (long long j = 0; j < l; ++j) {
            std::string ligneManifeste;
            if (!lireLigne(ligneManifeste)) break;
            texte += ligneManifeste;
            texte += '\n';
        }

        if (registre == "-") {
            std::cout << i << " REGISTRE AUCUN\n";
            continue;
        }
        std::cout << i << " MANIFESTE " << registre << "\n";

        if (l == -1) { std::cout << i << " ECHEC ILLISIBLE\n"; continue; }

        const std::string cle = "\"library_path\"";
        const size_t posCle = texte.find(cle);
        if (posCle == std::string::npos) { std::cout << i << " ECHEC CLE\n"; continue; }

        const size_t posDeuxPoints = texte.find(':', posCle + cle.size());
        if (posDeuxPoints == std::string::npos) { std::cout << i << " ECHEC DEUX_POINTS\n"; continue; }

        const size_t g1 = texte.find('"', posDeuxPoints);
        const size_t g2 = (g1 == std::string::npos) ? std::string::npos : texte.find('"', g1 + 1);
        if (g1 == std::string::npos || g2 == std::string::npos) {
            std::cout << i << " ECHEC GUILLEMET\n";
            continue;
        }

        const std::string valeur = nettoyer(texte.substr(g1 + 1, g2 - g1 - 1));
        if (valeur.empty()) { std::cout << i << " ECHEC VIDE\n"; continue; }

        std::cout << i << " VALEUR " << valeur << "\n";
        const bool absolu = (valeur.size() >= 2 && valeur[1] == ':') || estSeparateur(valeur[0]);
        if (absolu) {
            std::cout << i << " ABSOLU\n";
            std::cout << i << " CHARGE " << valeur << "\n";
        } else {
            std::cout << i << " RELATIF\n";
            size_t dernier = std::string::npos;
            for (size_t p = 0; p < registre.size(); ++p)
                if (estSeparateur(registre[p])) dernier = p;
            const std::string dossier = (dernier == std::string::npos) ? "" : registre.substr(0, dernier + 1);
            std::cout << i << " CHARGE " << dossier << valeur << "\n";
        }
    }
    return 0;
}
