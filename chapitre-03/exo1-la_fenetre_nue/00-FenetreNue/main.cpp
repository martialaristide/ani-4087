// =============================================================================
// Exercice — LA FENETRE NUE (chapitre 3, exo 1)
// Le programme de quinze lignes du chapitre, tel quel, avec le strict
// necessaire autour pour qu'il compile comme point d'entree Nkentseu.
// =============================================================================
#include "NKWindow/NKMain.h"
#include "NKWindow/Core/NkWindow.h"
#include "NKWindow/Core/NkWindowConfig.h"

using namespace nkentseu;

static void ConfigureAppData(NkAppData &d) {
        d.appName = "LaFenetreNue";
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

        while (fenetre.IsOpen()) {
                NkEvents().PollEvents();
        }

        return 0;
}
