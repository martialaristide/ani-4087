// L'index qui recule : calcule l'index de la derniere image relachee et le compare a deux ecritures plus courtes
// qui paraissent justes (l'une en int signe, l'autre en uint32_t).
// Entree : N, puis N lignes "n r" (nombre d'images de la chaine, nombre de liberations deja faites).
// Sortie : "n r prochaine juste signe nonsigne" ou "n r REFUSEE" par ligne, puis REFUSEES, FAUX SIGNE, FAUX NON SIGNE.
#include <cstdint>
#include <iostream>

int main() {
    long long nbEtats = 0;
    if (!(std::cin >> nbEtats)) nbEtats = 0;

    long long refusees = 0, fauxSigne = 0, fauxNonSigne = 0;

    for (long long i = 0; i < nbEtats; ++i) {
        long long n = 0, r = 0;
        if (!(std::cin >> n >> r)) break;

        if (n == 0 || n > 4) {
            // la chaine n'existe pas
            std::cout << n << " " << r << " REFUSEE\n";
            ++refusees;
            continue;
        }

        // prochaine = r % n, calcule sur 64 bits (r peut depasser un int)
        const long long prochaine = r % n;
        // l'ecriture du moteur : on ajoute n avant de retirer 1, on ne passe jamais sous zero
        const long long juste = (prochaine + n - 1) % n;
        // ecriture signee : le reste d'un nombre negatif est negatif ou nul en C++ (-1 % 3 vaut -1)
        const int signe = (static_cast<int>(prochaine) - 1) % static_cast<int>(n);
        // ecriture non signee sur 32 bits : 0 - 1 vaut 4294967295
        const uint32_t nonsigne = (static_cast<uint32_t>(prochaine) - 1u) % static_cast<uint32_t>(n);

        std::cout << n << " " << r << " " << prochaine << " " << juste << " " << signe << " " << nonsigne << "\n";
        if (signe != juste) ++fauxSigne;
        if (static_cast<long long>(nonsigne) != juste) ++fauxNonSigne;
    }

    std::cout << "REFUSEES " << refusees << "\n";
    std::cout << "FAUX SIGNE " << fauxSigne << "\n";
    std::cout << "FAUX NON SIGNE " << fauxNonSigne << "\n";
    return 0;
}
