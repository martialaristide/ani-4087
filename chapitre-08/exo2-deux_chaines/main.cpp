// Deux chaines : cree des chaines d'echange a partir de leur description, dit lesquelles sont refusees, bilan par oeil.
// Entree : N, puis N lignes "largeur hauteur images oeil nom" (nom vaut "-" quand la chaine n'a pas de nom).
// Sortie : une ligne par chaine (CHAINE ... ou REFUSEE ...), puis ACCEPTEES, EMPLACEMENTS, OCCUPES, GAUCHE, DROIT, SANS NOM.
#include <iostream>
#include <string>

int main() {
    int n = 0;
    if (!(std::cin >> n)) n = 0;

    long long acceptees = 0, occupes = 0, gauche = 0, droit = 0, sansNom = 0;
    const long long emplacementsParChaine = 4; // tableau fixe : 4 emplacements, quel que soit le nombre d'images

    for (int i = 0; i < n; ++i) {
        long long largeur = 0, hauteur = 0, images = 0, oeil = 0;
        std::string nom;
        if (!(std::cin >> largeur >> hauteur >> images >> oeil >> nom)) break;

        const bool sansNomRecu = (nom == "-");
        const std::string affiche = sansNomRecu ? "SANS-NOM" : nom;

        if (largeur == 0 || hauteur == 0) {
            // le controle de la taille passe en premier
            std::cout << "REFUSEE " << affiche << " TAILLE\n";
        } else if (images == 0 || images > 4) {
            std::cout << "REFUSEE " << affiche << " IMAGES\n";
        } else {
            std::cout << "CHAINE " << affiche << " " << largeur << " " << hauteur << " " << images << " "
                      << (oeil == 0 ? "GAUCHE" : "DROIT") << "\n";
            ++acceptees; // seules les chaines acceptees entrent dans le bilan
            occupes += images;
            if (oeil == 0) ++gauche; else ++droit;
            if (sansNomRecu) ++sansNom;
        }
    }

    std::cout << "ACCEPTEES " << acceptees << "\n";
    std::cout << "EMPLACEMENTS " << emplacementsParChaine * acceptees << "\n";
    std::cout << "OCCUPES " << occupes << "\n";
    std::cout << "GAUCHE " << gauche << "\n";
    std::cout << "DROIT " << droit << "\n";
    std::cout << "SANS NOM " << sansNom << "\n";
    return 0;
}
