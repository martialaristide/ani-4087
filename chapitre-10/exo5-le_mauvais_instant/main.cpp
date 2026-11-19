// Le mauvais instant : calcule l'erreur laissee a l'ecran quand la couche porte la pose relue au moment de soumettre
// au lieu de celle du rendu.
// Entree : "w T" (vitesse de la tete en degres par seconde ; nombre d'images), puis T lignes "P S A M" :
//   P : instant d'affichage predit par WaitFrame ; S : instant ou la boucle soumet la fin de trame ;
//   A : instant ou l'image est vraiment affichee ; M : R (pose du rendu) ou F (pose relue a l'instant S).
// Modele : la tete tourne a vitesse constante, son lacet a l'instant x vaut w * x milliemes de degre.
//   On dessine avec la pose predite : lacet du rendu = w * P. Pose soumise = w * P (R) ou w * S (F).
//   CASQUE = pose soumise - lacet du rendu (le compositeur du casque tourne l'image de cet ecart).
//   SIMULATEUR = w * A - w * P (le simulateur ne tourne rien : il montre l'image du rendu, quelle que soit la pose soumise).
// Sortie : "t CASQUE c SIMULATEUR s" par image, puis FAUSSES (images ou CASQUE != 0), PIRE CASQUE, PIRE SIMULATEUR
//   (plus grands ecarts, sans leur signe). Calcul sur 64 bits : on forme les differences avant de multiplier par w.
#include <iostream>

static long long absolu(long long v) { return v < 0 ? -v : v; }

int main() {
    long long w = 0, nbImages = 0;
    if (!(std::cin >> w >> nbImages)) { w = 0; nbImages = 0; }

    long long fausses = 0, pireCasque = 0, pireSimulateur = 0;

    for (long long t = 1; t <= nbImages; ++t) {
        long long p = 0, s = 0, a = 0;
        char mode = 'R';
        std::cin >> p >> s >> a >> mode;

        // w * S - w * P et w * A - w * P, sans former w * instant (qui deborde quand les instants sont grands)
        const long long casque = (mode == 'F') ? w * (s - p) : 0;
        const long long simulateur = w * (a - p);

        if (casque != 0) ++fausses;
        if (absolu(casque) > pireCasque) pireCasque = absolu(casque);
        if (absolu(simulateur) > pireSimulateur) pireSimulateur = absolu(simulateur);

        std::cout << t << " CASQUE " << casque << " SIMULATEUR " << simulateur << "\n";
    }

    std::cout << "FAUSSES " << fausses << "\n";
    std::cout << "PIRE CASQUE " << pireCasque << "\n";
    std::cout << "PIRE SIMULATEUR " << pireSimulateur << "\n";
    return 0;
}
