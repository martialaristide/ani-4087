// Les trois champs : calcule, image par image, les trois champs d'une action booleenne (current, changed, active),
// et repere les relachements qui n'en sont pas.
// Entree : T (nombre d'images) puis, si T > 0, un mot de T signes : 1 (lie et appuye), 0 (lie et relache), x (aucune source liee).
// Sortie : une ligne "t current changed active" par image, puis APPUIS, RELACHES, FAUX RELACHES et TENUES.
#include <iostream>
#include <string>

int main() {
    long long nbImages = 0;
    if (!(std::cin >> nbImages)) nbImages = 0;

    std::string signes;
    if (nbImages > 0) std::cin >> signes;

    int courantAvant = 0; // avant l'image 1, current vaut 0
    long long appuis = 0, relaches = 0, fauxRelaches = 0, tenues = 0;

    for (long long t = 1; t <= nbImages; ++t) {
        const char signe = (t - 1 < static_cast<long long>(signes.size())) ? signes[static_cast<size_t>(t - 1)] : 'x';

        const int active = (signe == 'x') ? 0 : 1;   // x : aucune source liee
        const int current = (signe == '1') ? 1 : 0;  // une action inactive vaut donc toujours 0
        const int changed = (current != courantAvant) ? 1 : 0; // par rapport a l'image d'avant

        std::cout << t << " " << current << " " << changed << " " << active << "\n";

        if (current == 1 && changed == 1) ++appuis;
        if (current == 0 && changed == 1) {
            ++relaches; // un faux relachement se compte aussi dans RELACHES
            if (active == 0) ++fauxRelaches;
        }
        if (current == 1) ++tenues;

        courantAvant = current;
    }

    std::cout << "APPUIS " << appuis << "\n";
    std::cout << "RELACHES " << relaches << "\n";
    std::cout << "FAUX RELACHES " << fauxRelaches << "\n";
    std::cout << "TENUES " << tenues << "\n";
    return 0;
}
