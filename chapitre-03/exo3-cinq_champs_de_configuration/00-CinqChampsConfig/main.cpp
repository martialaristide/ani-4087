// =============================================================================
// Exercice — CINQ CHAMPS DE CONFIGURATION (chapitre 3, exo 3)
// Meme fenetre que les exercices precedents. Un seul champ de NkWindowConfig
// est modifie a la fois (voir la ligne marquee CHAMP TESTE ICI), reconstruit
// et observe, avant de passer au champ suivant.
// =============================================================================
#include "NKWindow/NKMain.h"
#include "NKWindow/Core/NkWindow.h"
#include "NKWindow/Core/NkWindowConfig.h"

using namespace nkentseu;

static void ConfigureAppData(NkAppData &d) {
        d.appName = "CinqChampsConfig";
}
NK_REGISTER_ENTRY_APPDATA_UPDATER(ConfigureAppData)

int nkmain(const NkEntryState &state) {
        (void)state;

        NkWindowConfig config;
        config.title  = "Ma salle";
        config.width  = 1280;
        config.height = 720;

        // ===== CHAMP TESTE ICI (une seule ligne a la fois) =====================
        config.alwaysOnTop = true;
        // =========================================================================

        NkWindow fenetre(config);
        if (!fenetre.IsValid()) {
                return 1;
        }

        while (fenetre.IsOpen()) {
                NkEvents().PollEvents();
        }

        return 0;
}
