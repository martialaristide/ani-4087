// =============================================================================
// Chapitre 6, exercice 3 : accumuler puis vider.
//
// Symptôme : la tête tourne toute seule, lentement, quand on ne touche plus à la souris.
// Cause : NkInput.MouseRawDeltaX() garde le dernier déplacement reçu et personne ne le remet à zéro ; le relire à chaque image rejoue ce dernier déplacement sans fin.
// Preuve : mesuré, souris immobile : la valeur lue reste à 1 sur douze relevés de suite et le lacet monte d'environ 39 degrés par seconde.
// Correction : s'abonner aux événements bruts (NkMouseRawEvent), ajouter chaque déplacement à un accumulateur, lire l'accumulateur une seule fois par image, puis le remettre à zéro.
//
// Ce programme calcule TROIS lacets nourris par les mêmes événements :
//   lu          : ajoute NkInput.MouseRawDeltaX() a chaque image (version fautive)
//   vide        : accumule les evenements, lit une fois par image, puis vide
//   sans vidage : accumule et lit comme "vide", mais ne remet jamais a zero
// Le lacet "vide" est celui qui piloterait la camera ; ici on ne dessine rien,
// on imprime les trois lacets une fois par seconde.
// =============================================================================
#include <cstdio>

#include "NKWindow/NKMain.h"
#include "NKWindow/Core/NkWindow.h"
#include "NKWindow/Core/NkWindowConfig.h"
#include "NKWindow/Core/NkEvent.h"
#include "NKTime/NkChrono.h"

using namespace nkentseu;

static void ConfigureAppData(NkAppData &d) {
	d.appName = "AccumulerPuisVider";
}
NK_REGISTER_ENTRY_APPDATA_UPDATER(ConfigureAppData)

// Radians par compte du capteur, gardee pour toute la mesure.
static const double kSensibilite = 0.002;
static const double kRadVersDeg = 180.0 / 3.14159265358979323846;

int nkmain(const NkEntryState &state) {
	(void)state;

	NkWindowConfig config;
	config.title  = "Accumuler puis vider";
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

	// Deux accumulateurs, nourris par les MEMES evenements bruts.
	int accumuleVide = 0;
	int accumuleSansVidage = 0;
	NkEvents().AddEventCallback<NkMouseRawEvent>(
		[&accumuleVide, &accumuleSansVidage](NkMouseRawEvent *e) {
			accumuleVide += e->GetDeltaX();
			accumuleSansVidage += e->GetDeltaX();
		});

	double lacetLu = 0.0;
	double lacetVide = 0.0;
	double lacetSansVidage = 0.0;
	double prochainReleveMs = 1000.0;

	NkChrono chrono;

	while (enCours) {
		NkEvents().PollEvents();

		// 1) lu : lecture directe de l'etat du module, a chaque image.
		lacetLu += NkInput.MouseRawDeltaX() * kSensibilite;

		// 2) vide : on LIT l'accumulation une fois par image, PUIS on la vide.
		lacetVide += accumuleVide * kSensibilite;
		accumuleVide = 0;

		// 3) sans vidage : meme lecture, mais l'accumulateur n'est jamais remis a zero.
		lacetSansVidage += accumuleSansVidage * kSensibilite;

		const double ms = static_cast<double>(chrono.Elapsed().ToMilliseconds());
		if (ms >= prochainReleveMs) {
			std::printf("t = %.1f s, lu = %.1f, vide = %.1f, sans vidage = %.1f\n",
			            ms / 1000.0,
			            lacetLu * kRadVersDeg,
			            lacetVide * kRadVersDeg,
			            lacetSansVidage * kRadVersDeg);
			std::fflush(stdout);
			prochainReleveMs += 1000.0;
		}

		NkChrono::SleepMilliseconds(2);
	}

	// Toujours rendre le pointeur avant de finir (exercice 1).
	fenetre.ClipMouseToClient(false);
	fenetre.ShowMouse(true);
	return 0;
}
