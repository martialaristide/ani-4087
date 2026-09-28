// =============================================================================
// Exercice — LA TAILLE QUI CHANGE (chapitre 3, exo 5)
// Meme fenetre que l'exercice 1. Un rappel sur NkWindowResizeEvent affiche la
// nouvelle taille dans la console a chaque changement.
// =============================================================================
#include <cstdio>

#include "NKWindow/NKMain.h"
#include "NKWindow/Core/NkWindow.h"
#include "NKWindow/Core/NkWindowConfig.h"
#include "NKWindow/Core/NkEvent.h"

using namespace nkentseu;

static void ConfigureAppData(NkAppData &d) {
        d.appName = "LaTailleQuiChange";
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

        NkEvents().AddEventCallback<NkWindowResizeEvent>([](NkWindowResizeEvent *e) {
                std::printf("Nouvelle taille : %u x %u\n", e->GetWidth(), e->GetHeight());
        });

        while (fenetre.IsOpen()) {
                NkEvents().PollEvents();
        }

        return 0;
}
