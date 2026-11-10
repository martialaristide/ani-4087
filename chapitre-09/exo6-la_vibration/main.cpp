// La vibration : relit des appels a ApplyHaptic (amplitude, duree, frequence), remet chaque valeur dans ses bornes
// et juge si la vibration est bien reglee.
// Entree : N, puis N lignes "a d f" (amplitude en %, duree en ms, frequence en Hz), entiers possiblement nuls ou negatifs.
// Regle : A = a remis entre 0 et 100 ; D = d si d > 0, sinon MIN ; F = f si f > 0, sinon DEFAUT.
//   Verdict : A == 0 -> MUETTE ; sinon d < 50 -> BREVE (MIN compris) ; sinon d <= 200 -> BONNE ; sinon LONGUE.
//   TEMPS additionne les durees D ecrites en nombre (muettes comprises), MIN compte pour 0, sur 64 bits.
// Sortie : "i A D F verdict" par appel, puis BONNES, LONGUES, MUETTES, TEMPS.
#include <iostream>
#include <string>

int main() {
    long long n = 0;
    if (!(std::cin >> n)) n = 0;

    long long bonnes = 0, longues = 0, muettes = 0, temps = 0;

    for (long long i = 1; i <= n; ++i) {
        long long a = 0, d = 0, f = 0; // ordre du code : amplitude, duree, frequence
        std::cin >> a >> d >> f;

        long long amplitude = a;
        if (amplitude < 0) amplitude = 0;
        if (amplitude > 100) amplitude = 100;

        const std::string duree = (d > 0) ? std::to_string(d) : "MIN";
        const std::string frequence = (f > 0) ? std::to_string(f) : "DEFAUT";
        if (d > 0) temps += d; // MIN compte pour 0

        const char* verdict = nullptr;
        if (amplitude == 0) { verdict = "MUETTE"; ++muettes; } // jugee avant tout le reste
        else if (d < 50) verdict = "BREVE";                    // MIN compris
        else if (d <= 200) { verdict = "BONNE"; ++bonnes; }    // 50 et 200 compris
        else { verdict = "LONGUE"; ++longues; }

        std::cout << i << " " << amplitude << " " << duree << " " << frequence << " " << verdict << "\n";
    }

    std::cout << "BONNES " << bonnes << "\n";
    std::cout << "LONGUES " << longues << "\n";
    std::cout << "MUETTES " << muettes << "\n";
    std::cout << "TEMPS " << temps << "\n";
    return 0;
}
