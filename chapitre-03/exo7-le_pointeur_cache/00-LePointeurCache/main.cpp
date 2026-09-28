// =============================================================================
// Exercice — LE POINTEUR CACHE (chapitre 3, exo 7)
// Curseur cache et confine a la fenetre. A chaque image, affichage de la
// position x/y (NkInput.MouseX/Y, confinee par le systeme) et du rawDelta
// (NkInput.MouseRawDeltaX/Y, non confine, lit le mouvement physique brut de
// la souris independamment de la position ecran).
// =============================================================================
#include <cstdio>

#include "NKWindow/NKMain.h"
#include "NKWindow/Core/NkWindow.h"
#include "NKWindow/Core/NkWindowConfig.h"
#include "NKWindow/Core/NkEvent.h"

using namespace nkentseu;

static void ConfigureAppData(NkAppData &d) {
        d.appName = "LePointeurCache";
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

        fenetre.ShowMouse(false);
        fenetre.ClipMouseToClient(true);

        while (enCours) {
                NkEvents().PollEvents();

                std::printf("pos = (%d, %d)   rawDelta = (%d, %d)\n",
                                        NkInput.MouseX(), NkInput.MouseY(),
                                        NkInput.MouseRawDeltaX(), NkInput.MouseRawDeltaY());
        }

        fenetre.ShowMouse(true);
        fenetre.ClipMouseToClient(false);

        return 0;
}
