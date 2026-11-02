// Compter les emprunts : compte les emprunts reussis, les emprunts refuses et les liberations d'une boucle de rendu,
// et trouve le chemin qui oublie de rendre l'image.
// Entree : une ligne "n N" (nombre d'images de la chaine de 1 a 4, nombre de trames), puis N mots, un par trame :
//   normal : emprunte, dessine, rend l'image                       -> "t OK"
//   sortie : emprunte, puis sort aussitot SANS rendre l'image      -> "t FUITE"   (le chemin qui fuit)
//   double : emprunte, tente un second emprunt (refuse), puis rend -> "t DOUBLE"
// Une trame dont l'emprunt est refuse (une image est encore empruntee) s'arrete la : "t BLOQUEE", et ne rend rien.
// Sortie : une ligne par trame, puis REUSSIES a, REFUSEES b, LIBERATIONS c, ECART (a - c), PREMIERE FUITE t|AUCUNE.
#include <iostream>
#include <string>

int main() {
    long long n = 1, nbTrames = 0;
    if (!(std::cin >> n >> nbTrames)) nbTrames = 0;

    bool empruntee = false;      // une image est-elle empruntee ?
    long long reussies = 0, refusees = 0, liberations = 0;
    long long premiereFuite = 0; // 0 = aucune

    for (long long t = 1; t <= nbTrames; ++t) {
        std::string chemin;
        if (!(std::cin >> chemin)) break;

        // chaque trame commence par un emprunt
        if (empruntee) {
            ++refusees; // un emprunt refuse se compte, mais ne coute rien a la chaine
            std::cout << t << " BLOQUEE\n"; // la trame n'a rien emprunte : elle ne rend rien
            continue;
        }
        empruntee = true;
        ++reussies;

        if (chemin == "sortie") {
            // sortie anticipee : l'image reste empruntee, aucune liberation
            std::cout << t << " FUITE\n";
            if (premiereFuite == 0) premiereFuite = t;
        } else if (chemin == "double") {
            ++refusees; // le second emprunt est refuse (une image est deja empruntee)
            empruntee = false;
            ++liberations;
            std::cout << t << " DOUBLE\n";
        } else {
            // normal : dessine puis rend l'image
            empruntee = false;
            ++liberations;
            std::cout << t << " OK\n";
        }
    }

    std::cout << "REUSSIES " << reussies << "\n";
    std::cout << "REFUSEES " << refusees << "\n";
    std::cout << "LIBERATIONS " << liberations << "\n";
    std::cout << "ECART " << reussies - liberations << "\n";
    if (premiereFuite == 0) {
        std::cout << "PREMIERE FUITE AUCUNE\n";
    } else {
        std::cout << "PREMIERE FUITE " << premiereFuite << "\n";
    }
    (void)n;
    return 0;
}

// ── Ma boucle de rendu reelle, instrumentee (extrait du programme de mesure de la salle) ─────────────────────────
// Dans ma boucle, j'appelle "emprunt" l'appel BeginFrame() (il rend faux s'il est refuse) et "liberation" l'appel
// EndFrame() : ce n'est pas le AcquireImage/ReleaseImage d'une chaine de casque, mais l'equivalent au niveau de la trame.
//
//     const bool emprunte = renderer->BeginFrame();                      // l'emprunt
//     if (emprunte) ++empruntsReussis; else ++empruntsRefuses;
//     if (emprunte) {
//         ... scene, soumission ...
//         renderer->Present();
//         renderer->EndFrame();                                          // la liberation
//         ++liberations;
//     }
//
// Le chemin qui fuirait (c'est la trame "sortie" ci-dessus) : une sortie anticipee entre l'emprunt et la liberation.
//
//     if (emprunte) {
//         if (!sceneValide) continue;      // FUITE : EndFrame() n'est jamais appele pour cette trame
//         ...
//         renderer->EndFrame();
//         ++liberations;
//     }
//
// Apres cette sortie, empruntsReussis - liberations vaut 1 pour toujours, et chaque trame suivante est refusee.
