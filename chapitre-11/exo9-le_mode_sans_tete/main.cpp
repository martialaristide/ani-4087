#include <iostream>
#include <string>
#include <vector>

int main() {
    long long hz = 0;
    std::cin >> hz;
    if (hz <= 1) hz = 72;  // le simulateur prend 72 images par seconde
    const long long periode = 1000000000LL / hz;

    // Sans fenetre, la pose ne bouge pas : LIBRE = pose de depart, FIXE = pose donnee.
    long long pose[5] = {0, 0, 0, 1700, 0};
    std::string mode;
    std::cin >> mode;
    if (mode == "FIXE") {
        for (int i = 0; i < 5; ++i) std::cin >> pose[i];
    }

    long long t = 0;
    std::cin >> t;
    if (t < 0) t = 0;

    std::vector<long long> a(static_cast<size_t>(t)), b(static_cast<size_t>(t));
    for (long long i = 0; i < t; ++i) std::cin >> a[static_cast<size_t>(i)];
    for (long long i = 0; i < t; ++i) std::cin >> b[static_cast<size_t>(i)];

    std::cout << "PERIODE " << periode << "\n";
    std::cout << "POSE " << pose[0] << " " << pose[1] << " " << pose[2] << " "
              << pose[3] << " " << pose[4] << "\n";

    long long premierBrut = 0, premierRelatif = 0;  // 0 = aucune difference trouvee
    for (long long i = 0; i < t; ++i) {
        const size_t k = static_cast<size_t>(i);
        const long long predA = a[k] + periode;
        const long long predB = b[k] + periode;
        const long long predA1 = a[0] + periode;
        const long long predB1 = b[0] + periode;

        const long long brut = predB - predA;
        const long long relatif = (predB - predB1) - (predA - predA1);
        const long long scripte = (i + 1) * periode;

        if (brut != 0 && premierBrut == 0) premierBrut = i + 1;
        if (relatif != 0 && premierRelatif == 0) premierRelatif = i + 1;

        std::cout << (i + 1) << " BRUT " << brut << " RELATIF " << relatif
                  << " SCRIPTE " << scripte << "\n";
    }

    if (premierBrut == 0) std::cout << "BRUT IDENTIQUE\n";
    else std::cout << "BRUT DIFFERENT " << premierBrut << "\n";
    if (premierRelatif == 0) std::cout << "RELATIF IDENTIQUE\n";
    else std::cout << "RELATIF DIFFERENT " << premierRelatif << "\n";
    return 0;
}
