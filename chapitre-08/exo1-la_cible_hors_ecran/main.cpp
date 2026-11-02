// La cible hors ecran : suit la creation d'une cible, son branchement au rendu, et repere la boucle qui se fige.
// Entree : "L H F N" puis N ordres (allume NOM, cible w h, vers k, taille w h, image).
// Sortie : une ligne par evenement, puis "IMAGES n" et "ETAT BLOQUE|LIBRE".
#include <iostream>
#include <string>
#include <vector>

struct Cible {
    int largeur;
    int hauteur;
};

int main() {
    int largeurFenetre = 0, hauteurFenetre = 0, nbTampons = 1, nbOrdres = 0;
    if (!(std::cin >> largeurFenetre >> hauteurFenetre >> nbTampons >> nbOrdres)) return 0;

    bool offscreenAllume = false;               // seul "allume OFFSCREEN" compte
    std::vector<Cible> cibles;                  // la cible numero k est cibles[k - 1]
    int versCible = 0;                          // 0 = la fenetre
    int zoneLargeur = largeurFenetre;           // zone de rendu : celle de la fenetre tant que "taille" n'est pas appele
    int zoneHauteur = hauteurFenetre;
    std::vector<bool> tamponPris(nbTampons > 0 ? nbTampons : 1, false);
    int numeroImage = 0;                        // les images sont numerotees a partir de 1
    int imagesAffichees = 0;                    // nombre de lignes IMAGE
    bool fige = false;                          // apres un BLOQUE, plus rien n'agit

    for (int i = 0; i < nbOrdres; ++i) {
        std::string ordre;
        if (!(std::cin >> ordre)) break;

        if (ordre == "allume") {
            std::string nom;
            std::cin >> nom;
            if (!fige && nom == "OFFSCREEN") offscreenAllume = true;
        } else if (ordre == "cible") {
            int w = 0, h = 0;
            std::cin >> w >> h;
            if (fige) continue;
            if (!offscreenAllume) {
                std::cout << "CIBLE NULLE\n";
            } else {
                cibles.push_back({w, h});
                std::cout << "CIBLE " << cibles.size() << " " << w << " " << h << "\n";
            }
        } else if (ordre == "vers") {
            int k = 0;
            std::cin >> k;
            if (fige) continue;
            if (k == 0 || (k > 0 && k <= (int)cibles.size())) {
                versCible = k;
            } else {
                std::cout << "VERS REFUSE\n";
            }
        } else if (ordre == "taille") {
            int w = 0, h = 0;
            std::cin >> w >> h;
            if (fige) continue;
            zoneLargeur = w;
            zoneHauteur = h;
        } else if (ordre == "image") {
            if (fige) continue;
            ++numeroImage;
            const int tampon = (numeroImage - 1) % nbTampons;
            if (tamponPris[tampon]) {
                std::cout << "BLOQUE " << numeroImage << "\n";
                fige = true;
                continue;
            }
            ++imagesAffichees;
            if (versCible == 0) {
                std::cout << "IMAGE " << numeroImage << " FENETRE\n";
            } else {
                std::cout << "IMAGE " << numeroImage << " CIBLE " << versCible << "\n";
                const Cible &c = cibles[versCible - 1];
                // la zone plus large OU plus haute que la cible : le travail ne se termine jamais, le tampon reste pris
                if (zoneLargeur > c.largeur || zoneHauteur > c.hauteur) tamponPris[tampon] = true;
            }
        }
    }

    std::cout << "IMAGES " << imagesAffichees << "\n";
    std::cout << "ETAT " << (fige ? "BLOQUE" : "LIBRE") << "\n";
    return 0;
}
