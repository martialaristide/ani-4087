// La chaine a trois images : calcule quelle image de la chaine chaque trame emploie, et compte les trames qui peignent
// une image que le compositeur lit encore.
// Entree : une ligne K, puis K scenarios, un par ligne : "n L T" (n : nombre d'images de la chaine, de 1 a 4 ;
// L : nombre de trames pendant lesquelles le compositeur garde chaque image apres sa soumission ; T : nombre de trames).
// Modele : la trame u (de 1 a T) peint l'image (u - 1) % n, puis la soumet ; le compositeur la lit pendant les L trames suivantes.
// La trame u est en CONFLIT si l'image qu'elle peint a ete soumise par l'une des trames u - 1 ... u - L (seules comptent les
// trames de numero 1 ou plus). Une trame en conflit avec plusieurs trames d'avant ne compte qu'une fois.
// Sortie, par scenario : "CIBLES n L i1 i2 ... iT" puis "CONFLITS n L k".
#include <iostream>

int main() {
    long long nbScenarios = 0;
    if (!(std::cin >> nbScenarios)) nbScenarios = 0;

    for (long long s = 0; s < nbScenarios; ++s) {
        long long n = 1, L = 0, T = 0;
        if (!(std::cin >> n >> L >> T)) break;
        if (n < 1) n = 1;

        // les images peintes : la trame u peint l'image (u - 1) % n
        std::cout << "CIBLES " << n << " " << L;
        for (long long u = 1; u <= T; ++u) std::cout << " " << (u - 1) % n;
        std::cout << "\n";

        long long conflits = 0;
        for (long long u = 1; u <= T; ++u) {
            const long long image = (u - 1) % n;
            bool enConflit = false;
            // les trames d'avant que le compositeur lit encore : u - 1 ... u - L, a condition que leur numero soit 1 ou plus
            for (long long v = u - 1; v >= u - L && v >= 1; --v) {
                if ((v - 1) % n == image) {
                    enConflit = true;
                    break; // en conflit avec une ou plusieurs trames d'avant : elle ne compte qu'une fois
                }
            }
            if (enConflit) ++conflits;
        }
        std::cout << "CONFLITS " << n << " " << L << " " << conflits << "\n";
    }
    return 0;
}
