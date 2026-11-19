// Les six symptomes : relie chaque faute de fin de trame a ce qu'on verrait sur un casque, et dit lesquelles le
// simulateur signale.
// Entree : N, puis N lignes "fin g d instant pose espace" (une application par ligne) :
//   fin : TOUJOURS, SI_RENDU ou JAMAIS (quand la boucle appelle EndFrame) ; g, d : 1 si la chaine de l'oeil (gauche,
//   droit) est dans la projection, 0 sinon ; instant : P (predit) ou N (courant) ; pose : R (du rendu) ou F (relue) ;
//   espace : S (le bon) ou L (autre). Chaque application tourne dans une session ou la fenetre est reduite au moins une fois.
// Symptomes sur un casque, dans cet ordre : JAMAIS -> FIGE ; SI_RENDU -> NE_REPART_PAS ; g = 0 -> OEIL_NOIR_G ;
//   d = 0 -> OEIL_NOIR_D ; instant N -> TREMBLE ; pose F -> DE_TRAVERS ; espace L -> DECALE.
// Sortie par ligne i : "i CASQUE" + symptomes (ou AUCUN), puis "i SIMULATEUR DIT|MUET". Le simulateur dit quelque chose
//   si fin n'est pas TOUJOURS ou si g ou d vaut 0. A la fin : MUETTES m = applications ayant au moins un symptome sur un
//   casque et dont le simulateur ne dit rien.
#include <iostream>
#include <string>

int main() {
    long long n = 0;
    if (!(std::cin >> n)) n = 0;

    long long muettes = 0;

    for (long long i = 1; i <= n; ++i) {
        std::string fin, instant, pose, espace;
        int g = 1, d = 1;
        std::cin >> fin >> g >> d >> instant >> pose >> espace;

        std::string symptomes;
        const auto ajouter = [&symptomes](const char* nom) {
            symptomes += " ";
            symptomes += nom;
        };
        if (fin == "JAMAIS") ajouter("FIGE");
        if (fin == "SI_RENDU") ajouter("NE_REPART_PAS");
        if (g == 0) ajouter("OEIL_NOIR_G");
        if (d == 0) ajouter("OEIL_NOIR_D");
        if (instant == "N") ajouter("TREMBLE");
        if (pose == "F") ajouter("DE_TRAVERS");
        if (espace == "L") ajouter("DECALE");

        const bool simulateurDit = (fin != "TOUJOURS") || g == 0 || d == 0;

        std::cout << i << " CASQUE" << (symptomes.empty() ? " AUCUN" : symptomes) << "\n";
        std::cout << i << " SIMULATEUR " << (simulateurDit ? "DIT" : "MUET") << "\n";

        if (!symptomes.empty() && !simulateurDit) ++muettes;
    }

    std::cout << "MUETTES " << muettes << "\n";
    return 0;
}
