#include <iostream>
#include <map>
#include <queue>
#include <set>
#include <string>
#include <vector>

static bool finitParCpp(const std::string& nom) {
    return nom.size() >= 4 && nom.compare(nom.size() - 4, 4, ".cpp") == 0;
}

int main() {
    long long e = 0;
    std::set<std::string> externes;
    if (std::cin >> e) {
        for (long long i = 0; i < e; ++i) {
            std::string nom;
            if (!(std::cin >> nom)) break;
            externes.insert(nom);
        }
    }

    long long f = 0;
    std::vector<std::string> ordre;
    std::map<std::string, std::vector<std::string>> inclusions;
    if (std::cin >> f) {
        for (long long i = 0; i < f; ++i) {
            std::string nom;
            long long k = 0;
            if (!(std::cin >> nom >> k)) break;
            std::vector<std::string> liste;
            for (long long j = 0; j < k; ++j) {
                std::string inclus;
                if (!(std::cin >> inclus)) break;
                liste.push_back(inclus);
            }
            if (!inclusions.count(nom)) {
                ordre.push_back(nom);
                inclusions[nom] = liste;
            }
        }
    }

    long long unites = 0, touchees = 0;
    for (const std::string& unite : ordre) {
        if (!finitParCpp(unite)) continue;
        ++unites;

        std::map<std::string, std::string> pere;  // nom -> fichier qui l'a fait rencontrer
        std::set<std::string> vus;
        std::queue<std::string> file;
        vus.insert(unite);
        file.push(unite);
        std::string trouve;

        while (!file.empty() && trouve.empty()) {
            const std::string courant = file.front();
            file.pop();
            const auto it = inclusions.find(courant);
            if (it == inclusions.end()) continue;  // en-tete du systeme ou externe
            for (const std::string& inclus : it->second) {
                if (vus.count(inclus)) continue;
                vus.insert(inclus);
                pere[inclus] = courant;
                if (externes.count(inclus)) { trouve = inclus; break; }
                file.push(inclus);
            }
        }

        if (trouve.empty()) {
            std::cout << unite << " NON\n";
        } else {
            std::vector<std::string> chemin;
            for (std::string x = trouve; x != unite; x = pere[x]) chemin.push_back(x);
            chemin.push_back(unite);
            std::cout << unite << " OUI ";
            for (size_t i = chemin.size(); i-- > 0;) {
                std::cout << chemin[i];
                if (i > 0) std::cout << ">";
            }
            std::cout << "\n";
            ++touchees;
        }
    }
    std::cout << "TOUCHEES " << touchees << " SUR " << unites << "\n";
    return 0;
}
