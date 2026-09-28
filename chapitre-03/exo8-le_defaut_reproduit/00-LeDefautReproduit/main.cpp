// =============================================================================
// Exercice — LE DEFAUT REPRODUIT (chapitre 3, exo 8)
// Affiche rawDeltaX a chaque image, sans aucune accumulation manuelle : on
// lit directement NkInput.MouseRawDeltaX() a chaque tour de boucle. Objectif :
// observer ce que fait cette valeur une fois la souris immobilisee.
// =============================================================================
#include <cstdio>

#include "NKWindow/NKMain.h"
#include "NKWindow/Core/NkWindow.h"
#include "NKWindow/Core/NkWindowConfig.h"
#include "NKWindow/Core/NkEvent.h"

using namespace nkentseu;

static void ConfigureAppData(NkAppData &d) {
        d.appName = "LeDefautReproduit";
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
        NkEvents().AddEventCallback<NkWindowCloseEvent>([&enCours](NkWindowCloseEvent *e) {
                (void)e;
                enCours = false;
        });
        NkEvents().AddEventCallback<NkKeyPressEvent>([&enCours](NkKeyPressEvent *e) {
                if (e->GetKey() == NkKey::NK_ESCAPE) {
                        enCours = false;
                }
        });

        while (enCours) {
                NkEvents().PollEvents();

                std::printf("rawDeltaX = %d\n", NkInput.MouseRawDeltaX());
        }

        return 0;
}
