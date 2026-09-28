// =============================================================================
// Exercice — LE DEFAUT CORRIGE (chapitre 3, exo 9)
// Correction du defaut de l'exercice 8 : au lieu d'interroger un etat
// (NkInput.MouseRawDeltaX(), qui ne se remet jamais a zero), on accumule
// soi-meme les evenements NkMouseRawEvent recus pendant l'image, puis on
// "consomme" ce total a chaque tour de boucle (on le lit et on le remet a
// zero). Les deux series sont affichees cote a cote pour comparaison directe.
// =============================================================================
#include <cstdio>

#include "NKWindow/NKMain.h"
#include "NKWindow/Core/NkWindow.h"
#include "NKWindow/Core/NkWindowConfig.h"
#include "NKWindow/Core/NkEvent.h"

using namespace nkentseu;

static void ConfigureAppData(NkAppData &d) {
        d.appName = "LeDefautCorrige";
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

        // Accumulateur : chaque NkMouseRawEvent recu pendant l'image ajoute son
        // delta au total en attente.
        int32 accumulateurX = 0;
        NkEvents().AddEventCallback<NkMouseRawEvent>([&accumulateurX](NkMouseRawEvent *e) {
                accumulateurX += e->GetDeltaX();
        });

        while (enCours) {
                NkEvents().PollEvents();

                // Consommation : on prend le total accumule pendant cette image,
                // puis on remet l'accumulateur a zero pour l'image suivante.
                int32 deltaXConsomme = accumulateurX;
                accumulateurX = 0;

                std::printf("etat (defectueux) = %-5d   accumulateur (corrige) = %d\n",
                                        NkInput.MouseRawDeltaX(), deltaXConsomme);
        }

        return 0;
}
