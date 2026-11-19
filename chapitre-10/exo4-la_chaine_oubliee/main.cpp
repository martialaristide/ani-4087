// La chaine oubliee : rejoue les appels d'une boucle un par un et dit ce que la session, le simulateur et les chaines
// d'images repondent a chacun.
// Entree : "K N" (images par chaine, de 1 a 4 ; nombre d'appels), puis N appels, un par ligne :
//   wait | begin | acquire G|D | release G|D | end - (projection nulle) | end g d (g, d : chaine renseignee ou non, 1 ou 0).
// Modele : la session garde "trame ouverte" (faux au depart) ; chaque chaine garde un index d'image (0) et "empruntee".
//   wait : REFUSE si la trame est ouverte, sinon OK. begin : REFUSE si ouverte, sinon OK et la trame s'ouvre.
//   acquire : REFUSE si deja empruntee, sinon OK i et empruntee. release : REFUSE si rien d'emprunte, sinon OK, index (i+1)%K.
//   end : sans trame ouverte, REFUSE AUCUNE. Sinon, avec projection, le premier oeil (G puis D) dont la chaine est
//     renseignee ET empruntee donne REFUSE ACQUISE G|D ; la trame se ferme quand meme. Sinon OK, la trame se ferme, et avec
//     projection chaque chaine non renseignee ajoute un avertissement (OK AVERT G, D ou G D).
// Sortie : "numero nom reponse" par appel, puis JOURNAL (refus de wait, begin, end), AVERTISSEMENTS (un par oeil),
//   SILENCIEUX (refus de acquire et release, qui n'ecrivent rien).
#include <iostream>
#include <string>

struct Chaine {
    long long index = 0;
    bool empruntee = false;
};

int main() {
    long long k = 1, n = 0;
    if (!(std::cin >> k >> n)) { k = 1; n = 0; }
    if (k < 1) k = 1;

    bool trameOuverte = false;
    Chaine chaines[2]; // 0 : gauche, 1 : droite
    long long journal = 0, avertissements = 0, silencieux = 0;

    for (long long numero = 1; numero <= n; ++numero) {
        std::string nom;
        std::cin >> nom;
        std::cout << numero << " " << nom;

        if (nom == "wait") {
            if (trameOuverte) { std::cout << " REFUSE"; ++journal; }
            else std::cout << " OK";
        } else if (nom == "begin") {
            if (trameOuverte) { std::cout << " REFUSE"; ++journal; }
            else { std::cout << " OK"; trameOuverte = true; }
        } else if (nom == "acquire" || nom == "release") {
            std::string lettre;
            std::cin >> lettre;
            Chaine& c = chaines[lettre == "G" ? 0 : 1];
            if (nom == "acquire") {
                if (c.empruntee) { std::cout << " REFUSE"; ++silencieux; }
                else { std::cout << " OK " << c.index; c.empruntee = true; }
            } else {
                if (!c.empruntee) { std::cout << " REFUSE"; ++silencieux; }
                else { std::cout << " OK"; c.empruntee = false; c.index = (c.index + 1) % k; }
            }
        } else if (nom == "end") {
            std::string premier;
            std::cin >> premier;
            bool avecProjection = (premier != "-");
            bool renseignee[2] = {false, false};
            if (avecProjection) {
                std::string second;
                std::cin >> second;
                renseignee[0] = (premier == "1");
                renseignee[1] = (second == "1");
            }

            if (!trameOuverte) {
                std::cout << " REFUSE AUCUNE";
                ++journal;
            } else {
                trameOuverte = false; // la trame se ferme dans tous les cas
                int bloque = -1;
                if (avecProjection) {
                    for (int oeil = 0; oeil < 2 && bloque < 0; ++oeil) {
                        if (renseignee[oeil] && chaines[oeil].empruntee) bloque = oeil;
                    }
                }
                if (bloque >= 0) {
                    std::cout << " REFUSE ACQUISE " << (bloque == 0 ? "G" : "D");
                    ++journal;
                } else {
                    std::cout << " OK";
                    if (avecProjection && (!renseignee[0] || !renseignee[1])) {
                        std::cout << " AVERT";
                        if (!renseignee[0]) { std::cout << " G"; ++avertissements; }
                        if (!renseignee[1]) { std::cout << " D"; ++avertissements; }
                    }
                }
            }
        }
        std::cout << "\n";
    }

    std::cout << "JOURNAL " << journal << "\n";
    std::cout << "AVERTISSEMENTS " << avertissements << "\n";
    std::cout << "SILENCIEUX " << silencieux << "\n";
    return 0;
}
