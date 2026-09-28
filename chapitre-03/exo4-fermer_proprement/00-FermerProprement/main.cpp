// =============================================================================
// Exercice — FERMER PROPREMENT (chapitre 3, exo 4)
// La boucle ne teste plus fenetre.IsOpen() mais un booleen local, mis a false
// par deux rappels independants : la fermeture demandee par le systeme (croix,
// Alt+F4...) et la touche Echap. Les deux chemins convergent au meme endroit.
// =============================================================================
#include "NKWindow/NKMain.h"
#include "NKWindow/Core/NkWindow.h"
#include "NKWindow/Core/NkWindowConfig.h"
#include "NKWindow/Core/NkEvent.h"

using namespace nkentseu;

static void ConfigureAppData(NkAppData &d) {
        d.appName = "FermerProprement";
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

        // Rappel 1 : fermeture demandee par le systeme (clic sur la croix, Alt+F4...)
        NkEvents().AddEventCallback<NkWindowCloseEvent>([&enCours](NkWindowCloseEvent *e) {
                (void)e;
                enCours = false;
        });

        // Rappel 2 : touche Echap -> meme sortie que le clic sur la croix
        NkEvents().AddEventCallback<NkKeyPressEvent>([&enCours](NkKeyPressEvent *e) {
                if (e->GetKey() == NkKey::NK_ESCAPE) {
                        enCours = false;
                }
        });

        while (enCours) {
                NkEvents().PollEvents();
        }

        return 0;
}
