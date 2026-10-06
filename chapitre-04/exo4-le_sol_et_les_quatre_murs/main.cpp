#include <iostream>
#include <string>
#include <vector>

// Emprise au sol d'un mur (rectangle axis-aligned, bornes incluses).
struct Mur {
    std::string nom;
    int xmin;
    int xmax;
    int zmin;
    int zmax;
};

// Carre d'angle a verifier : meme forme qu'un mur, sans le nom.
struct Carre {
    int xmin;
    int xmax;
    int zmin;
    int zmax;
};

// Un angle est bouche si au moins UN mur contient le carre tout entier.
// Deux murs qui se partagent le carre ne le bouchent pas (regle 4 de l'enonce) :
// c'est pour ca qu'on teste chaque mur seul, jamais une union de plusieurs murs.
bool contientCarre(const Mur& m, const Carre& c) {
    return m.xmin <= c.xmin && m.xmax >= c.xmax
        && m.zmin <= c.zmin && m.zmax >= c.zmax;
}

bool angleBouche(const std::vector<Mur>& murs, const Carre& c) {
    for (const Mur& m : murs) {
        if (contientCarre(m, c)) {
            return true;
        }
    }
    return false;
}

int main() {
    int L = 0;
    int e = 0;
    std::cin >> L >> e;

    const int h = L / 2;

    int n = 0;
    std::cin >> n;

    std::vector<Mur> murs;
    murs.reserve(static_cast<std::size_t>(n));

    for (int i = 0; i < n; ++i) {
        std::string nom;
        int cx = 0;
        int cz = 0;
        int sx = 0;
        int sz = 0;
        std::cin >> nom >> cx >> cz >> sx >> sz;

        Mur m;
        m.nom = nom;
        m.xmin = cx - sx / 2;
        m.xmax = cx + sx / 2;
        m.zmin = cz - sz / 2;
        m.zmax = cz + sz / 2;
        murs.push_back(m);

        std::cout << m.nom << " " << m.xmin << " " << m.xmax
                  << " " << m.zmin << " " << m.zmax << "\n";
    }

    // Les quatre angles, toujours dans l'ordre d'affichage impose.
    const Carre fondGauche  {-h - e, -h,     -h - e, -h};
    const Carre fondDroit   { h,      h + e, -h - e, -h};
    const Carre entreeGauche{-h - e, -h,      h,      h + e};
    const Carre entreeDroit { h,      h + e,  h,      h + e};

    const bool okFondGauche   = angleBouche(murs, fondGauche);
    const bool okFondDroit    = angleBouche(murs, fondDroit);
    const bool okEntreeGauche = angleBouche(murs, entreeGauche);
    const bool okEntreeDroit  = angleBouche(murs, entreeDroit);

    std::cout << "FOND_GAUCHE " << (okFondGauche ? "BOUCHE" : "TROU") << "\n";
    std::cout << "FOND_DROIT " << (okFondDroit ? "BOUCHE" : "TROU") << "\n";
    std::cout << "ENTREE_GAUCHE " << (okEntreeGauche ? "BOUCHE" : "TROU") << "\n";
    std::cout << "ENTREE_DROIT " << (okEntreeDroit ? "BOUCHE" : "TROU") << "\n";

    const int trous = (okFondGauche ? 0 : 1) + (okFondDroit ? 0 : 1)
                     + (okEntreeGauche ? 0 : 1) + (okEntreeDroit ? 0 : 1);

    std::cout << "TROUS " << trous << "\n";

    return 0;
}
