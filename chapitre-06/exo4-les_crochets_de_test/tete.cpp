// =============================================================================
// Chapitre 6, exercice 4 : les crochets de test.
//
// La tete est rangee dans une structure : lacet et tangage en radians,
// position en metres. PoserTete la pose directement, sans passer par la
// souris. Quand la variable d'environnement MASALLE_TEST vaut 1, nkmain lance
// des epreuves AVANT d'ouvrir la fenetre, les imprime, puis rend 0 si tout
// passe et 1 sinon.
//
// Convention de la salle : repere de la main droite, avant = -Z, haut = +Y,
// donc la droite = +X. Un lacet positif tourne autour de +Y, dans le sens
// inverse des aiguilles d'une montre vu d'en haut : il fait passer le regard
// de -Z vers -X (vers la gauche).
// =============================================================================
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "NKWindow/NKMain.h"
#include "NKWindow/Core/NkWindow.h"
#include "NKWindow/Core/NkWindowConfig.h"
#include "NKWindow/Core/NkEvent.h"

using namespace nkentseu;

namespace {

const float kPi = 3.14159265358979323846f;

float EnRadians(float degres) {
	return degres * kPi / 180.0f;
}

struct Vec3 {
	float x, y, z;
};

struct Tete {
	float lacet = 0.0f;    // radians
	float tangage = 0.0f;  // radians
	Vec3 position = {0.0f, 0.0f, 0.0f};  // metres

	// Direction du regard.
	Vec3 Regard() const {
		Vec3 r;
		r.x = -std::sin(lacet) * std::cos(tangage);
		r.y = std::sin(tangage);
		r.z = -std::cos(lacet) * std::cos(tangage);
		return r;
	}

	// Pose les trois valeurs sans passer par la souris.
	void PoserTete(float lacetRad, float tangageRad, const Vec3 &pos) {
		lacet = lacetRad;
		tangage = tangageRad;
		position = pos;
	}
};

// Une epreuve : la tete est posee avec ces angles (en degres), et le regard
// attendu est ECRIT EN DUR. Il vient du raisonnement sur la convention de la
// salle, jamais de la formule de Regard().
struct Epreuve {
	float lacetDeg;
	float tangageDeg;
	Vec3 attendu;
};

const float kRac2Sur2 = 0.70710678f;

const Epreuve kEpreuves[] = {
	// lacet 90 : le regard quitte -Z et va vers -X (la gauche).
	{90.0f, 0.0f, {-1.0f, 0.0f, 0.0f}},
	// lacet 0, tangage 0 : on regarde l'avant, -Z.
	{0.0f, 0.0f, {0.0f, 0.0f, -1.0f}},
	// lacet 180 : on regarde vers l'arriere, +Z.
	{180.0f, 0.0f, {0.0f, 0.0f, 1.0f}},
	// tangage 45, lacet 0 : moitie vers le haut (+Y), moitie vers l'avant (-Z).
	{0.0f, 45.0f, {0.0f, kRac2Sur2, -kRac2Sur2}},
	// lacet 90 et tangage 45 : moitie haut, moitie gauche (-X).
	{90.0f, 45.0f, {-kRac2Sur2, kRac2Sur2, 0.0f}},
};

// cos(90 deg) calcule en float ne vaut pas exactement zero : tolerance.
const float kTolerance = 1e-4f;

bool Proche(float a, float b) {
	return std::fabs(a - b) < kTolerance;
}

// Evite d'imprimer -0.0000.
float Propre(float v) {
	return std::fabs(v) < 5e-5f ? 0.0f : v;
}

int LancerEpreuves() {
	int echecs = 0;
	const Vec3 positionTest = {1.0f, 1.6f, -2.0f};
	for (const Epreuve &e : kEpreuves) {
		Tete tete;
		tete.PoserTete(EnRadians(e.lacetDeg), EnRadians(e.tangageDeg), positionTest);
		const Vec3 obtenu = tete.Regard();

		const bool direction = Proche(obtenu.x, e.attendu.x) &&
		                       Proche(obtenu.y, e.attendu.y) &&
		                       Proche(obtenu.z, e.attendu.z);
		const bool position = Proche(tete.position.x, positionTest.x) &&
		                      Proche(tete.position.y, positionTest.y) &&
		                      Proche(tete.position.z, positionTest.z);
		const bool ok = direction && position;
		if (!ok) {
			++echecs;
		}
		std::printf("lacet %g, tangage %g : attendu (%.4f, %.4f, %.4f), obtenu (%.4f, %.4f, %.4f), %s\n",
		            e.lacetDeg, e.tangageDeg,
		            Propre(e.attendu.x), Propre(e.attendu.y), Propre(e.attendu.z),
		            Propre(obtenu.x), Propre(obtenu.y), Propre(obtenu.z),
		            ok ? "OK" : "ECHEC");
	}
	std::fflush(stdout);
	return echecs == 0 ? 0 : 1;
}

}  // namespace

static void ConfigureAppData(NkAppData &d) {
	d.appName = "LesCrochetsDeTest";
}
NK_REGISTER_ENTRY_APPDATA_UPDATER(ConfigureAppData)

int nkmain(const NkEntryState &state) {
	(void)state;

	// Le test tourne avant l'ouverture de la fenetre, sans souris.
	const char *test = std::getenv("MASALLE_TEST");
	if (test != nullptr && std::strcmp(test, "1") == 0) {
		return LancerEpreuves();
	}

	NkWindowConfig config;
	config.title  = "Les crochets de test";
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
	}
	return 0;
}
