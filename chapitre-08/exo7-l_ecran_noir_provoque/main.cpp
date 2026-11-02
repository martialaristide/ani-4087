// L'ecran noir provoque : suit la cible, la chaine et les images d'un oeil, et dit ce que le compositeur affiche
// apres chaque redimensionnement.
// Entree : N, puis N ordres : cible w h, chaine, enregistre, image.
// Sortie : des lignes CIBLE / IMAGE au fil des ordres, puis AFFICHEES a, NOIRES b et PERIMEES c.
#include <iostream>
#include <string>

int main() {
    long long nbOrdres = 0;
    if (!(std::cin >> nbOrdres)) nbOrdres = 0;

    long long prochainePoignee = 101;  // la premiere cible recoit 101, la suivante 102, etc.
    long long poigneeCourante = 0;     // 0 avant la premiere cible
    long long rangee = 0;              // la valeur rangee a l'index 0 de la chaine (0 au depart)
    long long numeroImage = 0;         // les images sont numerotees a partir de 1
    long long affichees = 0, noires = 0, perimees = 0;

    for (long long i = 0; i < nbOrdres; ++i) {
        std::string ordre;
        if (!(std::cin >> ordre)) break;

        if (ordre == "cible") {
            long long w = 0, h = 0;
            std::cin >> w >> h;
            poigneeCourante = prochainePoignee++;
            std::cout << "CIBLE " << poigneeCourante << " " << w << " " << h << "\n";
        } else if (ordre == "chaine") {
            rangee = 0; // la chaine neuve est vide : l'index 0 vaut 0
        } else if (ordre == "enregistre") {
            rangee = poigneeCourante; // la poignee de la cible COURANTE, pas la premiere
        } else if (ordre == "image") {
            ++numeroImage;
            if (rangee == 0) {
                std::cout << "IMAGE " << numeroImage << " NOIR\n";
                ++noires;
            } else if (rangee == poigneeCourante) {
                std::cout << "IMAGE " << numeroImage << " AFFICHE " << rangee << "\n";
                ++affichees;
            } else {
                // la chaine garde la poignee d'une ancienne cible
                std::cout << "IMAGE " << numeroImage << " PERIMEE " << rangee << "\n";
                ++perimees;
            }
        }
    }

    std::cout << "AFFICHEES " << affichees << "\n";
    std::cout << "NOIRES " << noires << "\n";
    std::cout << "PERIMEES " << perimees << "\n";
    return 0;
}
