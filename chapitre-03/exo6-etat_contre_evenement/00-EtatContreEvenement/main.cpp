// =============================================================================
// Exercice — ETAT CONTRE EVENEMENT (chapitre 3, exo 6)
// Deux compteurs : l'un s'incremente a chaque image ou Espace est tenue
// (poll d'etat via NkInput.IsKeyDown), l'autre a chaque NkKeyPressEvent recu
// sur Espace (evenement discret). Affichage des compteurs des le relachement
// de la touche (pas seulement a la fermeture), et fermeture geree par un
// booleen (meme mecanisme que l'exercice 4) pour eviter tout blocage sur la
// croix.
// =============================================================================
#include <cstdio>

#include "NKWindow/NKMain.h"
#include "NKWindow/Core/NkWindow.h"
#include "NKWindow/Core/NkWindowConfig.h"
#include "NKWindow/Core/NkEvent.h"

using namespace nkentseu;

static void ConfigureAppData(NkAppData &d) {
        d.appName = "EtatContreEvenement";
}
NK_REGISTER_ENTRY_APPDATA_UPDATER(ConfigureAppData)

int nkmain(const NkEntryState &state) {
        (void)state;

        NkWindowConfig config;
        config.title  = "Ma salle";
        config.width  = 1280;
        config.height = 720;

        NkWindow fenetre(config);
        if (!fenetre.IsValid()) {
                return 1;
        }

        bool enCours = true;
        uint64 imagesEspaceTenu = 0;
        uint64 evenementsPressionEspace = 0;

        // Fermeture propre : croix -> enCours = false (meme mecanisme que l'exo 4)
        NkEvents().AddEventCallback<NkWindowCloseEvent>([&enCours](NkWindowCloseEvent *e) {
                (void)e;
                enCours = false;
        });

        NkEvents().AddEventCallback<NkKeyPressEvent>([&evenementsPressionEspace](NkKeyPressEvent *e) {
                if (e->GetKey() == NkKey::NK_SPACE) {
                        evenementsPressionEspace++;
                }
        });

        // Affiche les compteurs des que Espace est relachee, sans attendre la
        // fermeture de la fenetre.
        NkEvents().AddEventCallback<NkKeyReleaseEvent>(
                [&imagesEspaceTenu, &evenementsPressionEspace](NkKeyReleaseEvent *e) {
                        if (e->GetKey() == NkKey::NK_SPACE) {
                                std::printf("Images ou Espace etait tenue (IsKeyDown) : %llu\n",
                                                        (unsigned long long)imagesEspaceTenu);
                                std::printf("NkKeyPressEvent recus sur Espace          : %llu\n",
                                                        (unsigned long long)evenementsPressionEspace);
                                std::fflush(stdout);
                        }
                });

        while (enCours) {
                NkEvents().PollEvents();

                if (NkInput.IsKeyDown(NkKey::NK_SPACE)) {
                        imagesEspaceTenu++;
                }
        }

        return 0;
}
