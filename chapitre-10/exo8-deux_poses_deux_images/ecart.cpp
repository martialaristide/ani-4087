// Deux poses, deux images : mesure, image par image, l'ecart entre la pose qui a servi au rendu et la pose relue a la
// fin de trame.
// Entree : "N d" (nombre d'images ; temps en ms entre le rendu et la fin de trame), puis N lignes "x1 z1 a1 x2 z2 a2" :
//   pose du rendu puis pose de la fin de trame, vues de dessus (x, z en mm ; lacet a en milliemes de degre), entiers signes.
// Sortie : "t POSITION p ANGLE a" par image (p : distance entre les deux positions arrondie a l'entier le plus proche ;
//   a : ecart de lacet sans son signe), puis MOYENNE POSITION et MOYENNE ANGLE (somme des ecarts / N, arrondie a l'entier
//   inferieur), puis VITESSE v (somme des ecarts de lacet / (N * d), entier inferieur : milliemes de degre par ms =
//   degres par seconde). Si N ou d vaut zero : seulement AUCUNE apres les lignes d'images.
#include <iostream>

static long long absolu(long long v) { return v < 0 ? -v : v; }

// Racine carree de s arrondie a l'entier le plus proche, en entiers (pas de flottants).
static long long racineArrondie(long long s) {
    long long r = 0;
    while ((r + 1) * (r + 1) <= s) ++r;      // r = partie entiere de la racine
    return (s > r * r + r) ? r + 1 : r;      // (r + 1/2)^2 = r^2 + r + 1/4
}

int main() {
    long long n = 0, d = 0;
    if (!(std::cin >> n >> d)) { n = 0; d = 0; }

    long long sommePosition = 0, sommeAngle = 0;

    for (long long t = 1; t <= n; ++t) {
        long long x1 = 0, z1 = 0, a1 = 0, x2 = 0, z2 = 0, a2 = 0;
        std::cin >> x1 >> z1 >> a1 >> x2 >> z2 >> a2;

        const long long dx = x2 - x1, dz = z2 - z1;
        const long long p = racineArrondie(dx * dx + dz * dz);
        const long long a = absolu(a2 - a1); // une tete qui tourne a gauche bouge autant qu'a droite

        sommePosition += p;
        sommeAngle += a;
        std::cout << t << " POSITION " << p << " ANGLE " << a << "\n";
    }

    if (n <= 0 || d <= 0) {
        std::cout << "AUCUNE\n";
        return 0;
    }

    std::cout << "MOYENNE POSITION " << sommePosition / n << "\n";   // sommes positives : division entiere = entier inferieur
    std::cout << "MOYENNE ANGLE " << sommeAngle / n << "\n";
    std::cout << "VITESSE " << sommeAngle / (n * d) << "\n";
    return 0;
}
