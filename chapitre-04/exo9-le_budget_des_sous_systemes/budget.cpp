#include <algorithm>
#include <cstdint>
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>

namespace {

std::unordered_map<std::string, std::uint32_t> tableDesDrapeaux() {
    return {
        {"RENDER2D", 1u},
        {"RENDER3D", 2u},
        {"TEXT", 4u},
        {"UI", 8u},
        {"SHADOW", 16u},
        {"POST_PROCESS", 32u},
        {"ALL", 4294967295u},
    };
}

struct Configuration {
    std::string nom;
    std::uint32_t valeur = 0;
    int mediane = 0;
    int moyenne = 0;
};

Configuration lireConfiguration(const std::unordered_map<std::string, std::uint32_t>& drapeaux) {
    Configuration config;
    int k = 0;
    std::cin >> config.nom >> k;

    for (int i = 0; i < k; ++i) {
        std::string nomDrapeau;
        std::cin >> nomDrapeau;
        config.valeur |= drapeaux.at(nomDrapeau);
    }

    std::vector<int> temps(10);
    long long somme = 0;
    for (int i = 0; i < 10; ++i) {
        std::cin >> temps[i];
        somme += temps[i];
    }

    std::sort(temps.begin(), temps.end());

    // Mediane de dix valeurs triees : moyenne des cinquieme et sixieme
    // (indices 4 et 5 en base zero), en division entiere.
    config.mediane = (temps[4] + temps[5]) / 2;
    config.moyenne = static_cast<int>(somme / 10);

    return config;
}

void afficherConfiguration(const Configuration& config) {
    std::cout << config.nom << " VALEUR " << config.valeur << "\n";
    std::cout << config.nom << " MEDIANE " << config.mediane << "\n";
    std::cout << config.nom << " MOYENNE " << config.moyenne << "\n";
}

} // namespace

int main() {
    const std::unordered_map<std::string, std::uint32_t> drapeaux = tableDesDrapeaux();

    const Configuration config1 = lireConfiguration(drapeaux);
    const Configuration config2 = lireConfiguration(drapeaux);

    afficherConfiguration(config1);
    afficherConfiguration(config2);

    std::cout << "ECART MEDIANES " << (config1.mediane - config2.mediane) << "\n";
    std::cout << "ECART MOYENNES " << (config1.moyenne - config2.moyenne) << "\n";

    return 0;
}
