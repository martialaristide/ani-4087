// La trame vide : joue une boucle de rendu image par image, soumet la fin de trame a chaque image,
// et dit ce que porte chaque soumission.
// Entree : "N T" (images de la chaine d'echange ; images de la boucle), puis, si T > 0, un mot de T signes :
//   1 : le runtime demande de rendre ; 0 : il ne faut rien rendre (fenetre reduite, casque pose).
// Regle : la chaine accepte de 1 a 4 images, sinon on affiche seulement CHAINE REFUSEE. La chaine est un anneau,
//   l'image a emprunter vaut 0 au depart. Le lacet de l'image t vaut t - 1 degres.
//   Image rendue : on emprunte l'image i, on dessine, on la rend ; la prochaine devient (i + 1) % N ;
//     la couche porte le lacet et l'index i : "t OUI 1 lacet i".
//   Image non rendue : rien n'est emprunte, l'anneau ne bouge pas, la fin de trame part avec zero couche : "t NON 0".
// Sortie finale : FINS (une par image), RENDUES, SAUTEES, PROCHAINE (index de la prochaine image a emprunter).
#include <iostream>
#include <string>

int main() {
    long long nbImagesChaine = 0, nbImages = 0;
    if (!(std::cin >> nbImagesChaine >> nbImages)) { nbImagesChaine = 0; nbImages = 0; }

    if (nbImagesChaine < 1 || nbImagesChaine > 4) {
        std::cout << "CHAINE REFUSEE\n";
        return 0;
    }

    std::string signes;
    if (nbImages > 0) std::cin >> signes;

    long long prochaine = 0;   // anneau : image a emprunter
    long long fins = 0, rendues = 0, sautees = 0;

    for (long long t = 1; t <= nbImages; ++t) {
        const char signe = (t - 1 < static_cast<long long>(signes.size())) ? signes[static_cast<size_t>(t - 1)] : '0';
        const long long lacet = t - 1; // suit le numero de l'image, pas le nombre d'images rendues

        if (signe == '1') {
            const long long i = prochaine;                 // emprunt, dessin, liberation
            prochaine = (i + 1) % nbImagesChaine;          // l'anneau n'avance que sur une image rendue
            ++rendues;
            std::cout << t << " OUI 1 " << lacet << " " << i << "\n";
        } else {
            ++sautees;                                     // rien d'emprunte, l'anneau ne bouge pas
            std::cout << t << " NON 0\n";
        }
        ++fins; // la fin de trame part a chaque image, rendue ou non
    }

    std::cout << "FINS " << fins << "\n";
    std::cout << "RENDUES " << rendues << "\n";
    std::cout << "SAUTEES " << sautees << "\n";
    std::cout << "PROCHAINE " << prochaine << "\n";
    return 0;
}
