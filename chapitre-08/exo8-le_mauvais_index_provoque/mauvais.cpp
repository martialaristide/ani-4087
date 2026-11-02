// Le mauvais index provoque : simule une chaine d'echange ou la soumission lit son index au mauvais moment,
// et mesure ce que l'oeil voit quand la camera avance.
// Entree : une ligne "n T x0 v" (nombre d'images de la chaine de 1 a 4, nombre de trames, position de depart de la camera
// et pas, en millimetres), puis un mot de T lettres : la lettre t donne le mode de lecture de l'index a la trame t.
//   A : l'index soumis est l'index emprunte.
//   P : lu AVANT la liberation : (prochaine + n - 1) % n  -> l'image de la trame d'avant (le mauvais moment).
//   D : lu APRES la liberation, meme formule              -> l'image que l'on vient de peindre (le bon moment).
// Chaque emplacement garde le numero de la derniere trame peinte dedans (0 au depart).
// Sortie : une ligne par trame ("t position saut" ou "t NOIR -"), puis SAUT MIN, SAUT MAX, TREMBLE OUI|NON,
//          ou seulement SAUTS AUCUN s'il n'y a eu aucun saut.
#include <algorithm>
#include <iostream>
#include <string>
#include <vector>

int main() {
    long long n = 1, nbTrames = 0, x0 = 0, v = 0;
    if (!(std::cin >> n >> nbTrames >> x0 >> v)) return 0;
    std::string modes;
    std::cin >> modes;
    if (n < 1) n = 1;

    std::vector<long long> emplacements(static_cast<size_t>(n), 0); // numero de la derniere trame peinte, 0 au depart
    long long prochaine = 0;
    bool precedenteMontree = false;   // la trame precedente a-t-elle montre quelque chose ?
    long long positionPrecedente = 0;
    bool aSaut = false;
    long long sautMin = 0, sautMax = 0;

    for (long long t = 1; t <= nbTrames; ++t) {
        const char mode = (t - 1 < static_cast<long long>(modes.size())) ? modes[static_cast<size_t>(t - 1)] : 'D';

        // on emprunte la prochaine image et l'on y peint la trame t
        const long long emprunte = prochaine;
        emplacements[static_cast<size_t>(emprunte)] = t;

        long long soumis = emprunte;                       // mode A : l'index emprunte
        if (mode == 'P') soumis = (prochaine + n - 1) % n; // lu avant la liberation
        prochaine = (prochaine + 1) % n;                   // on rend l'image
        if (mode == 'D') soumis = (prochaine + n - 1) % n; // lu apres la liberation

        const long long f = emplacements[static_cast<size_t>(soumis)]; // la trame montree
        if (f == 0) {
            std::cout << t << " NOIR -\n";
            precedenteMontree = false;
            continue;
        }
        const long long position = x0 + v * f;
        if (t == 1 || !precedenteMontree) {
            std::cout << t << " " << position << " -\n";
        } else {
            const long long saut = position - positionPrecedente;
            std::cout << t << " " << position << " " << saut << "\n";
            if (!aSaut) {
                sautMin = sautMax = saut;
                aSaut = true;
            } else {
                sautMin = std::min(sautMin, saut);
                sautMax = std::max(sautMax, saut);
            }
        }
        precedenteMontree = true;
        positionPrecedente = position;
    }

    if (!aSaut) {
        std::cout << "SAUTS AUCUN\n";
    } else {
        std::cout << "SAUT MIN " << sautMin << "\n";
        std::cout << "SAUT MAX " << sautMax << "\n";
        std::cout << "TREMBLE " << (sautMin != sautMax ? "OUI" : "NON") << "\n";
    }
    return 0;
}
