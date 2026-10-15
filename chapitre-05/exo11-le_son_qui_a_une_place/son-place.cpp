// son-place.cpp
// ANI-4087 chapitre 5, exercice 11 : le son qui a une place.
//
// Ouvre une fenetre (la "salle"), place une pluie (bruit rose) en boucle a
// la position de la fenetre de l'enonce (-1.98, 1.40, -0.90), avec
// attenuation 3D, puis fait suivre l'auditeur (SetListenerPosition /
// SetListenerOrientation) a travers 3 positions successives dans la piece :
// pres de la fenetre, au centre, dans le coin oppose. Une 4e phase
// recommence la position "centre" SANS positional, pour comparer.
//
// Aucune piece/fenetre 3D de cours precedent n'existait dans ce depot pour
// cet exercice : je definis moi-meme une piece simple, documentee ici et
// dans ecoute.txt. Piece rectangulaire d'environ 4m x 2.5m x 4m.
// minDistance = 0.5 (pas d'attenuation ecrasante tout pres de la source) ;
// maxDistance = 6.0 (le coin le plus eloigne de la fenetre dans cette piece
// est a environ 4.65 m ; 6 m laisse un vrai gradient d'attenuation sur
// toute la piece sans tomber a zero avant le coin oppose).
//
// Les 4 phases s'enchainent automatiquement (8 s chacune, annoncees dans
// la console) pendant une seule execution : plus simple et plus fiable a
// ecouter au casque que de relancer le programme 3 ou 4 fois a la main.

#include "NKWindow/NKMain.h"
#include "NKWindow/Core/NkWindow.h"
#include "NKWindow/Core/NkWindowConfig.h"
#include "NKWindow/Core/NkEvent.h"
#include "NKAudio/NkAudio.h"
#include <cstdio>
#include <cmath>
#include <chrono>

using namespace nkentseu;
using namespace nkentseu::audio;

static void ConfigureAppData(NkAppData &d) {
	d.appName = "SonPlace";
}
NK_REGISTER_ENTRY_APPDATA_UPDATER(ConfigureAppData)

// Position de la fenetre (source de pluie), donnee par l'enonce.
static const float32 FENETRE_X = -1.98f, FENETRE_Y = 1.40f, FENETRE_Z = -0.90f;

static void RegarderFenetre(float32 px, float32 py, float32 pz, float32 *outFwd) {
	float32 dx = FENETRE_X - px, dy = FENETRE_Y - py, dz = FENETRE_Z - pz;
	float32 len = std::sqrt(dx * dx + dy * dy + dz * dz);
	if (len < 1e-6f) { outFwd[0] = 0.f; outFwd[1] = 0.f; outFwd[2] = -1.f; return; }
	outFwd[0] = dx / len; outFwd[1] = dy / len; outFwd[2] = dz / len;
}

int nkmain(const NkEntryState &state) {
	(void)state;

	NkWindowConfig config;
	config.title  = "Son Place";
	config.width  = 960;
	config.height = 540;

	NkWindow fenetre(config);
	if (!fenetre.IsValid()) {
		std::fprintf(stderr, "echec : fenetre invalide\n");
		return 1;
	}

	bool enCours = true;
	NkEvents().AddEventCallback<NkWindowCloseEvent>([&enCours](NkWindowCloseEvent *e) {
		(void)e;
		enCours = false;
	});
	NkEvents().AddEventCallback<NkKeyPressEvent>([&enCours](NkKeyPressEvent *e) {
		if (e->GetKey() == NkKey::NK_ESCAPE) enCours = false;
	});

	AudioEngineConfig cfg;
	cfg.sampleRate = 48000;
	if (!AudioEngine::Instance().Initialize(cfg)) {
		std::fprintf(stderr, "echec : initialisation audio\n");
		return 1;
	}

	AudioSample pluie = AudioGenerator::GenerateNoise(4.f, WaveformType::NOISE_PINK);

	struct Phase {
		const char *nom;
		float32 x, y, z;
		bool positional;
	};
	const Phase phases[4] = {
		{"Position 1 : pres de la fenetre",      -1.50f, 1.40f, -0.70f, true},
		{"Position 2 : au centre",                 0.00f, 1.40f,  0.00f, true},
		{"Position 3 : coin oppose",               1.80f, 1.40f,  1.80f, true},
		{"Sans positional (position du centre)",   0.00f, 1.40f,  0.00f, false},
	};

	AudioHandle voix = AUDIO_HANDLE_INVALID;
	int32 phaseActuelle = -1;
	auto horlogeDebut = std::chrono::steady_clock::now();
	const int32 DUREE_PHASE_MS = 8000;

	while (enCours) {
		NkEvents().PollEvents();

		auto maintenant = std::chrono::steady_clock::now();
		auto ecouleMs = std::chrono::duration_cast<std::chrono::milliseconds>(maintenant - horlogeDebut).count();
		int32 phaseVoulue = (int32)(ecouleMs / DUREE_PHASE_MS);

		if (phaseVoulue >= 4) {
			enCours = false;
			break;
		}

		if (phaseVoulue != phaseActuelle) {
			phaseActuelle = phaseVoulue;
			const Phase &ph = phases[phaseActuelle];

			if (voix != AUDIO_HANDLE_INVALID) {
				AudioEngine::Instance().Stop(voix);
			}

			VoiceParams p;
			p.looping = true;
			p.source3d.position[0] = FENETRE_X;
			p.source3d.position[1] = FENETRE_Y;
			p.source3d.position[2] = FENETRE_Z;
			p.source3d.positional  = ph.positional;
			p.source3d.minDistance = 0.5f;
			p.source3d.maxDistance = 6.0f;
			voix = AudioEngine::Instance().Play(pluie, p);

			float32 regard[3];
			RegarderFenetre(ph.x, ph.y, ph.z, regard);
			AudioEngine::Instance().SetListenerPosition(ph.x, ph.y, ph.z);
			AudioEngine::Instance().SetListenerOrientation(regard[0], regard[1], regard[2], 0.f, 1.f, 0.f);

			std::printf("=== %s (positional=%d) : ecoutez au casque pendant %d s ===\n",
			            ph.nom, (int)ph.positional, DUREE_PHASE_MS / 1000);
		}
	}

	if (voix != AUDIO_HANDLE_INVALID) {
		AudioEngine::Instance().Stop(voix);
	}
	AudioEngine::Instance().Shutdown();
	AudioLoader::Free(pluie);

	return 0;
}
