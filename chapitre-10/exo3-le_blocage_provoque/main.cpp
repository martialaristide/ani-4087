// Le blocage provoque : suit la session du moteur image par image et montre ce qui arrive quand la boucle saute
// la fin de trame pendant une pause.
// Entree : "P T" (P : TOUJOURS ou SAUTE, la facon de soumettre la fin de trame ; T : nombre d'images), puis, si T > 0,
//   un mot de T signes : le signe t est ce que le runtime repond a WaitFrame a l'image t (1 : rendre, 0 : rien).
// Modele : la session garde un drapeau "trame ouverte", faux au depart. A chaque image, un etat neuf (rendre = faux).
//   WaitFrame : refuse si la trame est ouverte (l'etat reste neuf), sinon "rendre" prend la valeur du signe.
//   BeginFrame : refuse si la trame est ouverte, sinon accepte et la trame s'ouvre.
//   EndFrame : TOUJOURS l'appelle a chaque image ; SAUTE ne l'appelle que si "rendre" vaut vrai.
//     Un appel accepte ferme la trame ; un appel sans trame ouverte est refuse.
// Sortie : "t W B R E" par image (W, B : OK|REFUSE ; R : OUI|NON ; E : OK|REFUSE|-), puis RENDUES, PERDUES, ERREURS,
//   PREMIER REFUS (premiere image ou WaitFrame est refuse, ou AUCUN).
#include <iostream>
#include <string>

int main() {
    std::string facon;
    long long nbImages = 0;
    if (!(std::cin >> facon >> nbImages)) { facon = "TOUJOURS"; nbImages = 0; }

    std::string signes;
    if (nbImages > 0) std::cin >> signes;

    const bool saute = (facon == "SAUTE");
    bool trameOuverte = false;
    long long rendues = 0, perdues = 0, erreurs = 0, premierRefus = 0;

    for (long long t = 1; t <= nbImages; ++t) {
        const char signe = (t - 1 < static_cast<long long>(signes.size())) ? signes[static_cast<size_t>(t - 1)] : '0';
        bool rendre = false; // etat neuf a chaque image

        // WaitFrame
        const char* w = "OK";
        if (trameOuverte) {
            w = "REFUSE"; // l'etat reste neuf : on ne prend pas le signe de l'image
            ++erreurs;
            if (premierRefus == 0) premierRefus = t;
        } else {
            rendre = (signe == '1');
        }

        // BeginFrame
        const char* b = "OK";
        if (trameOuverte) {
            b = "REFUSE";
            ++erreurs;
        } else {
            trameOuverte = true;
        }

        if (rendre) ++rendues;
        else if (signe == '1') ++perdues; // le runtime voulait un rendu, la session n'a pas suivi

        // EndFrame
        const char* e = "-";
        if (!saute || rendre) {
            if (trameOuverte) {
                e = "OK";
                trameOuverte = false;
            } else {
                e = "REFUSE";
                ++erreurs;
            }
        }

        std::cout << t << " " << w << " " << b << " " << (rendre ? "OUI" : "NON") << " " << e << "\n";
    }

    std::cout << "RENDUES " << rendues << "\n";
    std::cout << "PERDUES " << perdues << "\n";
    std::cout << "ERREURS " << erreurs << "\n";
    if (premierRefus == 0) std::cout << "PREMIER REFUS AUCUN\n";
    else std::cout << "PREMIER REFUS " << premierRefus << "\n";
    return 0;
}
