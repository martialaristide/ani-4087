// =============================================================================
// formats.cpp — ANI-4087, chapitre 5, exercice 1 : "Treize formats, une image"
//
// Charge une vraie photographie (un mur), la fait ecrire par le moteur dans
// cinq formats, puis RELIT chacun des cinq fichiers produits pour mesurer :
//   - sa taille reelle sur le disque (octets)
//   - ses dimensions relues (Width() / Height())
// Piege a eviter : un fichier ecrit n'est pas forcement un fichier relisible.
// C'est la relecture qui fait foi, jamais le nom du fichier qu'on vient
// d'ecrire ni la taille en memoire de l'image source.
// =============================================================================
#include "NKImage/Core/NkImage.h"

#include <cstdio>
#include <functional>

using namespace nkentseu;

namespace {

struct Format {
        const char *nom;
        const char *fichier;
        std::function<bool(const NkImage &)> ecrire;
};

// Taille reelle sur le disque, ou -1 si le fichier n'existe pas / ne s'ouvre
// pas. On ne se fie jamais a une taille en memoire pour ce chiffre : le
// chapitre le rappelle, disque et memoire ne mesurent pas la meme chose.
long TailleFichier(const char *chemin) {
        FILE *f = std::fopen(chemin, "rb");
        if (!f) return -1;
        if (std::fseek(f, 0, SEEK_END) != 0) {
                std::fclose(f);
                return -1;
        }
        const long taille = std::ftell(f);
        std::fclose(f);
        return taille;
}

} // namespace

int main() {
        // Chemin relatif : resolu depuis le repertoire d'ou l'executable est
        // lance (piege du chapitre 6). On lance donc ce programme avec
        // mur3.jpeg a cote de lui.
        NkImage photo;
        if (!photo.Load("mur3.jpeg")) {
                std::printf("[ERREUR] impossible de charger mur3.jpeg\n");
                return 1;
        }
        std::printf("[OK] photo chargee : %d x %d\n", photo.Width(), photo.Height());

        // Cinq formats choisis pour couvrir les trois familles possibles :
        // compresse avec perte (JPEG), compresse sans perte (PNG, QOI), et
        // non compresse (BMP, TGA).
        const Format formats[] = {
                {"PNG", "mur_png.png", [](const NkImage &im) { return im.SavePNG("mur_png.png"); }},
                {"JPEG", "mur_jpeg.jpg", [](const NkImage &im) { return im.SaveJPEG("mur_jpeg.jpg", 90); }},
                {"BMP", "mur_bmp.bmp", [](const NkImage &im) { return im.SaveBMP("mur_bmp.bmp"); }},
                {"TGA", "mur_tga.tga", [](const NkImage &im) { return im.SaveTGA("mur_tga.tga"); }},
                {"QOI", "mur_qoi.qoi", [](const NkImage &im) { return im.SaveQOI("mur_qoi.qoi"); }},
        };

        FILE *out = std::fopen("formats.txt", "w");
        if (!out) {
                std::printf("[ERREUR] impossible d'ouvrir formats.txt en ecriture\n");
                return 1;
        }

        for (const Format &fmt : formats) {
                if (!fmt.ecrire(photo)) {
                        std::printf("  %-4s : ECHEC a l'ecriture\n", fmt.nom);
                        std::fprintf(out, "%s : illisible\n", fmt.nom);
                        continue;
                }

                // La relecture, pas l'ecriture, decide du resultat.
                NkImage relue;
                const bool relit = relue.Load(fmt.fichier);
                const long taille = TailleFichier(fmt.fichier);

                if (!relit || taille < 0) {
                        std::printf("  %-4s : ecrit mais illisible a la relecture\n", fmt.nom);
                        std::fprintf(out, "%s : %ld octets, illisible\n", fmt.nom, taille < 0 ? 0 : taille);
                        continue;
                }

                if (relue.Width() != photo.Width() || relue.Height() != photo.Height()) {
                        std::printf("  %-4s : ATTENTION dimensions relues %d x %d != photo d'origine %d x %d\n",
                                                fmt.nom, relue.Width(), relue.Height(), photo.Width(), photo.Height());
                }

                std::printf("  %-4s : %ld octets, %d x %d\n", fmt.nom, taille, relue.Width(), relue.Height());
                std::fprintf(out, "%s : %ld octets, %d x %d\n", fmt.nom, taille, relue.Width(), relue.Height());
        }

        std::fclose(out);
        return 0;
}
