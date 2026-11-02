// La discipline respectee : suit les appels faits a une chaine d'echange et refuse ceux qui violent la discipline
// acquire -> draw -> release -> submit.
// Entree : "n N" (nombre d'images de la chaine, nombre d'ordres), puis N ordres (acquire, draw, release, submit).
// Sortie : une ligne par ordre, puis SOUMISES, REFUS et HORS EMPRUNT.
#include <iostream>
#include <string>

int main() {
    int n = 1, nbOrdres = 0;
    if (!(std::cin >> n >> nbOrdres)) nbOrdres = 0;
    if (n < 1) n = 1;

    int prochaine = 0;                 // l'image que la chaine preterait ensuite
    bool empruntee = false;            // une image est-elle empruntee ?
    int imageEmpruntee = 0;            // laquelle
    bool dessinee = false;             // l'image empruntee a-t-elle ete dessinee ?
    bool renduDepuisSoumission = false; // une image a-t-elle ete rendue depuis la derniere soumission acceptee ?
    bool derniereRendueVide = false;   // la derniere image rendue l'a-t-elle ete sans aucun draw ?
    int soumises = 0, refus = 0, horsEmprunt = 0;

    for (int i = 0; i < nbOrdres; ++i) {
        std::string ordre;
        if (!(std::cin >> ordre)) break;

        if (ordre == "acquire") {
            if (empruntee) {
                std::cout << "ACQUIRE REFUSE\n";
                ++refus;
            } else {
                empruntee = true;
                imageEmpruntee = prochaine;
                dessinee = false;
                std::cout << "ACQUIRE " << imageEmpruntee << "\n";
            }
        } else if (ordre == "draw") {
            if (empruntee) {
                dessinee = true;
                std::cout << "DRAW " << imageEmpruntee << "\n";
            } else {
                std::cout << "DRAW HORS EMPRUNT\n";
                ++horsEmprunt;
            }
        } else if (ordre == "release") {
            if (!empruntee) {
                // un release refuse ne fait pas avancer la prochaine image
                std::cout << "RELEASE REFUSE\n";
                ++refus;
            } else {
                prochaine = (prochaine + 1) % n;
                empruntee = false;
                renduDepuisSoumission = true;
                derniereRendueVide = !dessinee;
                std::cout << "RELEASE OK\n";
            }
        } else if (ordre == "submit") {
            if (empruntee) {
                std::cout << "SUBMIT REFUSE EMPRUNTEE\n";
                ++refus;
            } else if (!renduDepuisSoumission) {
                std::cout << "SUBMIT REFUSE RIEN\n";
                ++refus;
            } else {
                // on soumet la derniere image relachee : un cran en arriere de la prochaine
                const int j = (prochaine + n - 1) % n;
                std::cout << "SUBMIT " << j << (derniereRendueVide ? " VIDE" : "") << "\n";
                ++soumises;
                renduDepuisSoumission = false;
            }
        }
    }

    std::cout << "SOUMISES " << soumises << "\n";
    std::cout << "REFUS " << refus << "\n";
    std::cout << "HORS EMPRUNT " << horsEmprunt << "\n";
    return 0;
}
