// ANI-4087 chapitre 5, exercice 5 : "Quatre filtres, une mesure"
#include "NKImage/Core/NkImage.h"
#include "NKTime/NkChrono.h"
#include <cstdio>
#include <cstring>
#include <utility>

using nkentseu::NkImage;
using nkentseu::NkResizeFilter;
using nkentseu::NkChrono;

int main() {
        NkImage photo;
        if (!photo.Load("murs/mur_brique.jpg")) {
                std::printf("ECHEC chargement photo source\n");
                return 1;
        }
        std::printf("Photo source : %dx%d\n", photo.Width(), photo.Height());

        // Reduction en 2048x2048 : c'est la source commune aux quatre filtres.
        NkImage source = photo.Resize(2048, 2048, NkResizeFilter::NK_BILINEAR);
        std::printf("Source de travail : %dx%d\n", source.Width(), source.Height());

        struct FiltreTest {
                const char *nom;
                NkResizeFilter filtre;
                const char *fichier;
        };
        FiltreTest filtres[4] = {
                {"NK_NEAREST",  NkResizeFilter::NK_NEAREST,  "nearest.png"},
                {"NK_BILINEAR", NkResizeFilter::NK_BILINEAR, "bilinear.png"},
                {"NK_BICUBIC",  NkResizeFilter::NK_BICUBIC,  "bicubic.png"},
                {"NK_LANCZOS3", NkResizeFilter::NK_LANCZOS3, "lanczos3.png"},
        };

        NkImage resultats[4];
        double moyennesUs[4];

        for (int i = 0; i < 4; ++i) {
                // Appel "a froid" non chronometre : la toute premiere reduction d'un
                // filtre paie une initialisation (tables, caches) que les suivantes
                // ne paient plus -- on l'ecarte de la mesure.
                NkImage chauffe = source.Resize(256, 256, filtres[i].filtre);

                NkChrono chrono;
                chrono.Reset();
                NkImage dernier;
                for (int r = 0; r < 20; ++r) {
                        dernier = source.Resize(256, 256, filtres[i].filtre);
                }
                const double totalUs = chrono.Elapsed().ToMicroseconds();
                moyennesUs[i] = totalUs / 20.0;

                resultats[i] = std::move(dernier);
                resultats[i].SavePNG(filtres[i].fichier);
        }

        // Comparaison pixel par pixel au bilineaire (indice 1).
        const NkImage &ref = resultats[1];
        long differences[4] = {0, 0, 0, 0};
        for (int i = 0; i < 4; ++i) {
                long d = 0;
                for (int y = 0; y < 256; ++y) {
                        for (int x = 0; x < 256; ++x) {
                                auto a = resultats[i].GetPixel(x, y);
                                auto b = ref.GetPixel(x, y);
                                if (a.r != b.r || a.g != b.g || a.b != b.b || a.a != b.a)
                                        d++;
                        }
                }
                differences[i] = d;
        }

        for (int i = 0; i < 4; ++i) {
                std::printf("%s : %.2f us, differences : %ld\n",
                                        filtres[i].nom, moyennesUs[i], differences[i]);
        }

        return 0;
}
