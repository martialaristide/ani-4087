// Le mauvais espace : dit ou le compositeur croit que l'oeil etait quand l'espace declare dans la couche n'est pas
// celui des poses.
// Entree : "ax ay az ak" (ancre de l'espace LOCAL, exprimee dans STAGE), puis N, puis N lignes "D C x y z k" :
//   D : espace dans lequel la pose a ete demandee (S : STAGE, L : LOCAL) ; C : espace declare dans la couche ;
//   x y z : position en mm (y monte) ; k : lacet en quarts de tour.
// Regle : (x, z) tourne de k quarts de tour : 0 : (x,z) ; 1 : (z,-x) ; 2 : (-x,-z) ; 3 : (-z,x) ; y ne tourne pas.
//   Composer l'ancre A avec une pose p : position A + (p tournee de A.k), lacet (A.k + p.k) % 4 (LOCAL -> STAGE).
//   Vraie pose dans STAGE : p si D vaut S, ancre composee avec p si D vaut L.
//   Pose crue par le compositeur : p si C vaut S, ancre composee avec p si C vaut L.
//   ECART = pose crue - vraie pose, composante par composante ; dk = (k cru - k vrai + 4) % 4.
// Sortie : "i CRU x y z k" puis "i ECART dx dy dz dk" par pose, puis JUSTES j et FAUSSES f (juste : quatre ecarts nuls).
#include <iostream>

struct Pose {
    long long x = 0, y = 0, z = 0;
    int k = 0;
};

static int lacet(long long k) { return static_cast<int>(((k % 4) + 4) % 4); }

// Tourne (x, z) de k quarts de tour ; y ne tourne pas.
static void tourner(long long x, long long z, int k, long long& rx, long long& rz) {
    switch (k) {
        case 0: rx = x;  rz = z;  break;
        case 1: rx = z;  rz = -x; break;
        case 2: rx = -x; rz = -z; break;
        default: rx = -z; rz = x; break;
    }
}

// Ancre composee avec p : passage de LOCAL a STAGE.
static Pose composer(const Pose& ancre, const Pose& p) {
    long long rx = 0, rz = 0;
    tourner(p.x, p.z, ancre.k, rx, rz); // on tourne la position AVANT d'ajouter l'ancre
    Pose r;
    r.x = ancre.x + rx;
    r.y = ancre.y + p.y;
    r.z = ancre.z + rz;
    r.k = (ancre.k + p.k) % 4;
    return r;
}

int main() {
    Pose ancre;
    long long ak = 0;
    std::cin >> ancre.x >> ancre.y >> ancre.z >> ak;
    ancre.k = lacet(ak);

    long long n = 0;
    if (!(std::cin >> n)) n = 0;

    long long justes = 0, fausses = 0;

    for (long long i = 1; i <= n; ++i) {
        char demande = 'S', declare = 'S';
        Pose p;
        long long k = 0;
        std::cin >> demande >> declare >> p.x >> p.y >> p.z >> k;
        p.k = lacet(k);

        const Pose vraie = (demande == 'L') ? composer(ancre, p) : p;  // ou etait vraiment l'oeil, dans STAGE
        const Pose crue = (declare == 'L') ? composer(ancre, p) : p;   // ou le compositeur le croit

        const long long dx = crue.x - vraie.x, dy = crue.y - vraie.y, dz = crue.z - vraie.z;
        const int dk = (crue.k - vraie.k + 4) % 4;

        std::cout << i << " CRU " << crue.x << " " << crue.y << " " << crue.z << " " << crue.k << "\n";
        std::cout << i << " ECART " << dx << " " << dy << " " << dz << " " << dk << "\n";

        if (dx == 0 && dy == 0 && dz == 0 && dk == 0) ++justes;
        else ++fausses;
    }

    std::cout << "JUSTES " << justes << "\n";
    std::cout << "FAUSSES " << fausses << "\n";
    return 0;
}
