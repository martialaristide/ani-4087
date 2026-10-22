// =============================================================================
// Chapitre 6, exercice 2 : la derive provoquee.
//
// VERSION FAUTIVE, ET C'EST VOULU. A chaque image on ajoute au lacet et au
// tangage le deplacement brut que garde le module d'entrees
// (NkInput.MouseRawDeltaX/Y), multiplie par une sensibilite. On n'accumule
// rien soi-meme et on ne remet rien a zero : le module ne vide jamais cette
// valeur, donc la tete continue de tourner quand la souris s'arrete.
//
// La camera regarderait dans la direction
//   (-sin(lacet)*cos(tangage) ; sin(tangage) ; -cos(lacet)*cos(tangage))
// ici on ne dessine rien : on mesure seulement le lacet.
// =============================================================================
#include <cmath>
#include <cstdint>
#include <cstdio>

#include "NKWindow/NKMain.h"
#include "NKWindow/Core/NkWindow.h"
#include "NKWindow/Core/NkWindowConfig.h"
#include "NKWindow/Core/NkEvent.h"
#include "NKTime/NkChrono.h"

using namespace nkentseu;

static void ConfigureAppData(NkAppData &d) {
	d.appName = "LaDeriveProvoquee";
}
NK_REGISTER_ENTRY_APPDATA_UPDATER(ConfigureAppData)

// Radians par compte du capteur, gardee pour toute la mesure.
static const double kSensibilite = 0.002;
static const double kRadVersDeg = 180.0 / 3.14159265358979323846;

int nkmain(const NkEntryState &state) {
	(void)state;

	NkWindowConfig config;
	config.title  = "La derive provoquee";
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

	// Pointeur cache et confine : la souris reste dans la fenetre.
	fenetre.ShowMouse(false);
	fenetre.ClipMouseToClient(true);

	double lacet = 0.0;    // radians
	double tangage = 0.0;  // radians
	std::uint64_t images = 0;
	double prochainReleveMs = 1000.0;

	NkChrono chrono;

	while (enCours) {
		NkEvents().PollEvents();

		// Le defaut voulu : lecture directe, a chaque image, sans accumuler
		// et sans remettre a zero.
		const int brutX = NkInput.MouseRawDeltaX();
		const int brutY = NkInput.MouseRawDeltaY();
		lacet += brutX * kSensibilite;
		tangage += brutY * kSensibilite;
		++images;

		const double ms = static_cast<double>(chrono.Elapsed().ToMilliseconds());
		if (ms >= prochainReleveMs) {
			std::printf("t = %.1f s, images = %llu, brut = %d, lacet = %.1f deg\n",
			            ms / 1000.0,
			            static_cast<unsigned long long>(images),
			            brutX,
			            lacet * kRadVersDeg);
			std::fflush(stdout);
			prochainReleveMs += 1000.0;
		}

		NkChrono::SleepMilliseconds(2);
	}

	// Toujours rendre le pointeur avant de finir (exercice 1).
	fenetre.ClipMouseToClient(false);
	fenetre.ShowMouse(true);
	(void)tangage;
	return 0;
}
