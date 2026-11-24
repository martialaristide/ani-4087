#include <cstdlib>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

static bool lireLigne(std::string& ligne) {
    if (!std::getline(std::cin, ligne)) return false;
    if (!ligne.empty() && ligne.back() == '\r') ligne.pop_back();
    return true;
}

static bool estChiffre(char c) { return c >= '0' && c <= '9'; }

// Lit un nombre a partir de pos : saute les espaces, signe facultatif, chiffres.
// Rend false s'il n'y a aucun chiffre (la valeur vaut alors 0). pos avance jusqu'au premier autre signe.
static bool lireNombre(const std::string& s, size_t& pos, long long& valeur) {
    size_t i = pos;
    while (i < s.size() && s[i] == ' ') ++i;
    bool negatif = false;
    if (i < s.size() && (s[i] == '+' || s[i] == '-')) {
        negatif = (s[i] == '-');
        ++i;
    }
    unsigned long long acc = 0;
    bool chiffres = false;
    while (i < s.size() && estChiffre(s[i])) {
        acc = acc * 10ULL + static_cast<unsigned long long>(s[i] - '0');
        chiffres = true;
        ++i;
    }
    pos = i;
    if (!chiffres) { valeur = 0; return false; }
    valeur = negatif ? -static_cast<long long>(acc) : static_cast<long long>(acc);
    return true;
}

// Interrupteur : NON si le premier signe est 0, ou si la valeur vaut exactement "false".
static std::string interrupteur(const std::string& valeur) {
    if (valeur == "false" || valeur[0] == '0') return "NON";
    return "OUI";
}

int main() {
    std::string ligne;

    std::string backend, demi, profondeur, liste;
    long long hz = 0, latence = 0;
    if (lireLigne(ligne)) {
        std::istringstream flux(ligne);
        flux >> backend >> hz >> demi >> profondeur >> liste >> latence;
    }

    std::string loader = "-";
    if (lireLigne(ligne)) loader = ligne;

    bool poseFixe = false;
    long long pose[5] = {0, 0, 0, 0, 0};
    if (lireLigne(ligne)) {
        std::istringstream flux(ligne);
        std::string mot;
        flux >> mot;
        if (mot == "FIXE") {
            poseFixe = true;
            for (int i = 0; i < 5; ++i) flux >> pose[i];
        }
    }

    long long m = 0;
    if (lireLigne(ligne)) m = std::atoll(ligne.c_str());

    long long appliquees = 0, ignorees = 0;
    for (long long v = 0; v < m; ++v) {
        if (!lireLigne(ligne)) break;
        const size_t egal = ligne.find('=');
        if (egal == std::string::npos) { ++ignorees; continue; }
        const std::string nom = ligne.substr(0, egal);
        const std::string valeur = ligne.substr(egal + 1);

        bool pose_ok = false;  // vrai si cette variable a pose une valeur
        const bool vide = valeur.empty();

        if (nom == "NK_XR_BACKEND") {
            if (valeur == "openxr") { backend = "OPENXR"; pose_ok = true; }
        } else if (nom == "NK_XR_HZ" || nom == "NK_XR_SIM_LATENCY_MS") {
            if (!vide) {
                size_t pos = 0;
                long long n = 0;
                lireNombre(valeur, pos, n);
                if (nom == "NK_XR_HZ") hz = n; else latence = n;
                pose_ok = true;
            }
        } else if (nom == "NK_XR_HALF_RATE") {
            if (!vide) { demi = interrupteur(valeur); pose_ok = true; }
        } else if (nom == "NK_XR_DEPTH_LAYER") {
            if (!vide) { profondeur = interrupteur(valeur); pose_ok = true; }
        } else if (nom == "NK_XR_LIST_EXT") {
            if (!vide) { liste = interrupteur(valeur); pose_ok = true; }
        } else if (nom == "NK_XR_OPENXR_LOADER") {
            if (!vide) { loader = valeur; pose_ok = true; }
        } else if (nom == "NK_XR_SIM_POSE") {
            if (!vide) {
                long long lus[5];
                int compte = 0;
                size_t pos = 0;
                while (compte < 5 && pos < valeur.size()) {
                    long long n = 0;
                    if (!lireNombre(valeur, pos, n)) break;
                    lus[compte++] = n;
                    while (pos < valeur.size() && (valeur[pos] == ',' || valeur[pos] == ' ')) ++pos;
                }
                if (compte == 5) {
                    poseFixe = true;
                    for (int i = 0; i < 5; ++i) pose[i] = lus[i];
                    pose_ok = true;
                }
            }
        }

        if (pose_ok) ++appliquees; else ++ignorees;
    }

    std::cout << "BACKEND " << backend << "\n";
    std::cout << "HZ " << hz << "\n";
    std::cout << "DEMI " << demi << "\n";
    std::cout << "PROFONDEUR " << profondeur << "\n";
    std::cout << "LISTE " << liste << "\n";
    std::cout << "LOADER " << loader << "\n";
    std::cout << "LATENCE " << latence << "\n";
    if (poseFixe) {
        std::cout << "POSE FIXE";
        for (int i = 0; i < 5; ++i) std::cout << " " << pose[i];
        std::cout << "\n";
    } else {
        std::cout << "POSE LIBRE\n";
    }
    std::cout << "APPLIQUEES " << appliquees << "\n";
    std::cout << "IGNOREES " << ignorees << "\n";
    return 0;
}
