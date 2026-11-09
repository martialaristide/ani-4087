// Synchroniser une fois par image : suit les appels a SyncActions d'une boucle de rendu et compte les saisies qu'elle
// declenche vraiment.
// Entree : T (nombre d'images), puis T lignes "p s f" :
//   p : 1 si la gachette est appuyee pendant l'image ; s : nombre d'appels a SyncActions (0 a 3) ; f : 1 si la session est FOCUSED.
// Modele : un appel sans le focus est refuse et ne change rien. Un appel accepte fait, dans cet ordre :
//   changed = (p != current), puis current = p, puis active = 1.
// Apres les s appels de l'image, on lit l'etat : on saisit si current et changed valent 1.
// Sortie : une ligne "t current changed active X" (X : SAISIT ou -) par image, puis APPUIS REELS, SAISIES, SYNC REFUSEES.
#include <iostream>

int main() {
    long long nbImages = 0;
    if (!(std::cin >> nbImages)) nbImages = 0;

    int current = 0, changed = 0, active = 0; // au depart, les trois champs valent 0
    int pAvant = 0;                           // avant l'image 1, p vaut 0
    long long appuisReels = 0, saisies = 0, syncRefusees = 0;

    for (long long t = 1; t <= nbImages; ++t) {
        int p = 0, s = 0, f = 0;
        if (!(std::cin >> p >> s >> f)) break;

        // un vrai appui : p vaut 1 alors qu'il valait 0 a l'image d'avant (compte a part des saisies)
        if (p == 1 && pAvant == 0) ++appuisReels;
        pAvant = p;

        for (int i = 0; i < s; ++i) {
            if (f == 0) {
                ++syncRefusees; // sans le focus : refuse, l'etat ne change pas, pas meme changed
                continue;
            }
            changed = (p != current) ? 1 : 0; // calcule a chaque appel accepte : le second appel efface le front du premier
            current = p;
            active = 1;
        }

        const bool saisit = (current == 1 && changed == 1);
        if (saisit) ++saisies;
        std::cout << t << " " << current << " " << changed << " " << active << " " << (saisit ? "SAISIT" : "-") << "\n";
    }

    std::cout << "APPUIS REELS " << appuisReels << "\n";
    std::cout << "SAISIES " << saisies << "\n";
    std::cout << "SYNC REFUSEES " << syncRefusees << "\n";
    return 0;
}
