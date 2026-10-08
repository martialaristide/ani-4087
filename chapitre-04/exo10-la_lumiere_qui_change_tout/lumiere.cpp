#include <cmath>
#include <iostream>
#include <string>
#include <vector>

namespace {

struct Vec3 {
    double x;
    double y;
    double z;
};

struct Face {
    std::string nom;
    Vec3 normale;
};

// Les cinq faces et leurs normales interieures, dans l'ordre impose par l'enonce.
const std::vector<Face>& faces() {
    static const std::vector<Face> f = {
        {"SOL", {0.0, 1.0, 0.0}},
        {"FOND", {0.0, 0.0, 1.0}},
        {"ENTREE", {0.0, 0.0, -1.0}},
        {"GAUCHE", {1.0, 0.0, 0.0}},
        {"DROIT", {-1.0, 0.0, 0.0}},
    };
    return f;
}

double produitScalaire(const Vec3& a, const Vec3& b) {
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

double longueur(const Vec3& v) {
    return std::sqrt(produitScalaire(v, v));
}

} // namespace

int main() {
    double ambiante = 0.0;
    int n = 0;
    std::cin >> ambiante >> n;

    for (int i = 0; i < n; ++i) {
        std::string nom;
        Vec3 direction{};
        double intensite = 0.0;
        std::cin >> nom >> direction.x >> direction.y >> direction.z >> intensite;

        const double longueurDirection = longueur(direction);

        // Lumiere la plus forte et la plus faible rencontrees, pour le CONTRASTE.
        long long plusEclairee = 0;
        long long plusSombre = 0;

        for (std::size_t j = 0; j < faces().size(); ++j) {
            const Face& face = faces()[j];

            // c est le cosinus (non normalise par la normale, deja unitaire) entre
            // la normale interieure et la direction du soleil, signe inverse : une
            // face "face au soleil" (normale opposee a la direction d'arrivee) doit
            // donner un c positif. On divise par la longueur de la direction pour
            // ne garder que son orientation, jamais sa magnitude.
            double c = 0.0;
            if (longueurDirection > 0.0) {
                c = -produitScalaire(face.normale, direction) / longueurDirection;
            }

            // Une face qui ne recoit pas directement le soleil (c negatif) ne
            // garde que la lumiere ambiante : sans ce plancher, elle serait noire.
            const double contribution = intensite * std::max(0.0, c);
            const long long valeurLumiere = std::llround(ambiante + contribution);

            if (j == 0 || valeurLumiere > plusEclairee) plusEclairee = valeurLumiere;
            if (j == 0 || valeurLumiere < plusSombre) plusSombre = valeurLumiere;

            std::cout << nom << " " << face.nom << " " << valeurLumiere << "\n";
        }

        std::cout << nom << " CONTRASTE " << (plusEclairee - plusSombre) << "\n";
    }

    return 0;
}
