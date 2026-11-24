#include <iostream>
#include <string>
#include <vector>
#include <set>

// Lit une liste : un entier n, puis n noms separes par des blancs.
// Les doublons internes sont ignores (premiere apparition gardee).
static std::vector<std::string> lireListe() {
    std::vector<std::string> noms;
    std::set<std::string> vus;
    long long n = 0;
    if (!(std::cin >> n)) return noms;
    for (long long i = 0; i < n; ++i) {
        std::string nom;
        if (!(std::cin >> nom)) break;
        if (vus.insert(nom).second) noms.push_back(nom);
    }
    return noms;
}

int main() {
    const std::vector<std::string> a = lireListe();
    const std::vector<std::string> b = lireListe();
    const std::set<std::string> ensA(a.begin(), a.end());
    const std::set<std::string> ensB(b.begin(), b.end());

    long long communs = 0;
    for (const std::string& nom : a)
        if (ensB.count(nom)) ++communs;

    std::cout << "DEUX " << communs << "\n";
    long long seulA = 0, seulB = 0;
    for (const std::string& nom : a)
        if (!ensB.count(nom)) { std::cout << "SEUL_A " << nom << "\n"; ++seulA; }
    for (const std::string& nom : b)
        if (!ensA.count(nom)) { std::cout << "SEUL_B " << nom << "\n"; ++seulB; }

    const long long d = seulA + seulB;
    const long long u = communs + d;
    std::cout << "DIFFERENTES " << d << " SUR " << u << "\n";
    return 0;
}
