// La manette qui s'eteint : fait tourner une saisie avec le test de active (AVEC) et une saisie sans (SANS) sur les memes
// images, et compte leurs ecarts.
// Entree : T (nombre d'images) puis, si T > 0, un mot de T signes :
//   1 : manette allumee, gachette appuyee ; 0 : manette allumee, gachette relachee ; x : manette eteinte.
// Modele : active = 0 pour x, 1 sinon ; current = 1 pour 1, 0 sinon ; changed = (current != current de l'image d'avant),
// calcule meme pendant la coupure ; avant l'image 1, current vaut 0. La main est toujours assez pres de l'objet.
//   AVEC : si active vaut 0, rien ne change ; sinon prise si current && changed, lacher si !current && changed.
//   SANS : pareil sans le test de active. Un lacher sur une image inactive est une CHUTE : l'objet est par terre et plus
//          aucune prise ne reussit ensuite dans cette version. Un objet deja lache ne peut pas tomber.
// Sortie : "t AVEC SANS" (TENU ou LIBRE) par image, puis PRISES AVEC, PRISES SANS, CHUTES SANS, COINCES AVEC, DIFFERENCES.
#include <iostream>
#include <string>

int main() {
    long long nbImages = 0;
    if (!(std::cin >> nbImages)) nbImages = 0;

    std::string signes;
    if (nbImages > 0) std::cin >> signes;

    int courantAvant = 0;         // avant l'image 1, current vaut 0
    bool tenuAvec = false;        // l'objet est-il tenu dans la version AVEC ?
    bool tenuSans = false;        // ... dans la version SANS ?
    bool tombeSans = false;       // l'objet est-il par terre dans la version SANS ?
    long long prisesAvec = 0, prisesSans = 0, chutesSans = 0, coincesAvec = 0, differences = 0;

    for (long long t = 1; t <= nbImages; ++t) {
        const char signe = (t - 1 < static_cast<long long>(signes.size())) ? signes[static_cast<size_t>(t - 1)] : 'x';
        const int active = (signe == 'x') ? 0 : 1;
        const int current = (signe == '1') ? 1 : 0;
        const int changed = (current != courantAvant) ? 1 : 0; // calcule meme pendant la coupure
        courantAvant = current;

        // version AVEC : on teste active
        if (active == 1) {
            if (current == 1 && changed == 1) {
                tenuAvec = true;
                ++prisesAvec; // une prise faite alors que l'objet est deja tenu compte aussi
            } else if (current == 0 && changed == 1) {
                tenuAvec = false;
            }
        }

        // version SANS : on ne teste pas active
        if (!tombeSans) {
            if (current == 1 && changed == 1) {
                tenuSans = true;
                ++prisesSans;
            } else if (current == 0 && changed == 1) {
                if (active == 0 && tenuSans) {
                    // un lacher sur une image ou active vaut 0 : l'objet tombe par terre, definitivement
                    tombeSans = true;
                    ++chutesSans;
                }
                tenuSans = false;
            }
        }

        if (signe == '0' && tenuAvec) ++coincesAvec; // main ouverte, objet encore tenu
        if (tenuAvec != tenuSans) ++differences;

        std::cout << t << " " << (tenuAvec ? "TENU" : "LIBRE") << " " << (tenuSans ? "TENU" : "LIBRE") << "\n";
    }

    std::cout << "PRISES AVEC " << prisesAvec << "\n";
    std::cout << "PRISES SANS " << prisesSans << "\n";
    std::cout << "CHUTES SANS " << chutesSans << "\n";
    std::cout << "COINCES AVEC " << coincesAvec << "\n";
    std::cout << "DIFFERENCES " << differences << "\n";
    return 0;
}
