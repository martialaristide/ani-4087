// =============================================================================
// Chapitre 6, exercice 1 : le pointeur capture.
//
// Tout le code qui touche au pointeur de la souris est ici, et nulle part
// ailleurs. Le programme appelant n'utilise que deux fonctions :
//
//     void PointeurInstaller(nkentseu::NkWindow &fenetre);
//     void PointeurRendre();
//
// Etat voulu :
//   - pointeur CACHE, CAPTURE, et CONFINE a la fenetre
//   - touche F1 : rend le pointeur (visible, libre)
//   - touche F2 : le reprend
//   - a la fin du programme, le pointeur est TOUJOURS rendu
// =============================================================================
#include "NKWindow/Core/NkWindow.h"
#include "NKWindow/Core/NkWESystem.h"
#include "NKEvent/NkKeyboardEvent.h"

using namespace nkentseu;

namespace {

NkWindow *gFenetre = nullptr;
bool gCapture = false;  // l'etat que NOUS croyons avoir demande a la fenetre

// Met le pointeur dans l'etat demande. Ne touche la fenetre que si l'etat
// change : ShowMouse compte les appels (compteur Win32), donc deux appels a
// false exigeraient deux appels a true.
void Appliquer(bool capture) {
	if (gFenetre == nullptr || capture == gCapture) {
		return;
	}
	gFenetre->ShowMouse(!capture);          // cache quand on capture
	gFenetre->CaptureMouse(capture);        // la fenetre garde la souris
	gFenetre->ClipMouseToClient(capture);   // ... et la confine a sa zone cliente
	gCapture = capture;
}

}  // namespace

void PointeurInstaller(NkWindow &fenetre) {
	gFenetre = &fenetre;
	gCapture = false;
	Appliquer(true);

	NkEvents().AddEventCallback<NkKeyPressEvent>([](NkKeyPressEvent *e) {
		if (e->GetKey() == NkKey::NK_F1) {
			Appliquer(false);   // rendre
		} else if (e->GetKey() == NkKey::NK_F2) {
			Appliquer(true);    // reprendre
		}
	});
}

// A appeler apres la boucle, quelle que soit la raison de la sortie
// (Echap, croix de la fenetre, erreur). Sans effet si deja rendu.
void PointeurRendre() {
	Appliquer(false);
}
