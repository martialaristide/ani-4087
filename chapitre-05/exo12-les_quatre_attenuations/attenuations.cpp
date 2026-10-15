// attenuations.cpp
// ANI-4087 chapitre 5, exercice 12 : les quatre attenuations.
//
// Mesure hors ligne (NULL_OUTPUT) du gain reel des 4 modeles
// d'attenuation (NONE, INVERSE, LINEAR, EXPONENTIAL) a 5 distances
// (0.5, 1, 2, 4, 8 m), pour rolloffFactor = 1 puis 2. L'auditeur est a
// l'origine, regarde vers -Z (avant (0,0,-1), haut (0,1,0)). La source
// est un sinus doux (440 Hz, 2 s, amplitude 0.1) place devant
// l'auditeur en (0, 0, -d), joue en boucle.
//
// Pour chaque mesure : 48000 trames rendues par blocs de 256 avec
// RenderToBuffer, RMS calcule sur les DEUX oreilles ENSEMBLE (toutes les
// valeurs L et R confondues dans une seule moyenne des carres), comme
// demande -- pas une RMS separee par oreille.
//
// Le gain de chaque mesure est cette RMS divisee par la RMS du modele
// NONE, a la MEME distance et au MEME rolloffFactor (donc NONE/NONE = 1.000
// partout, par construction).
//
// Piege respecte : la source ne bouge jamais pendant qu'elle joue -- un
// Play() frais et un Stop() par mesure, jamais de changement a chaud des
// reglages d'une voix en cours de lecture.

#include "NKAudio/NkAudio.h"
#include <cstdio>
#include <cmath>
#include <vector>

using namespace nkentseu;
using namespace nkentseu::audio;

// RMS combinee (L et R ensemble) pour un modele/distance/rolloff donnes.
static double MesurerRMS(AudioSample &son, AttenuationModel modele, float32 distance, float32 rolloff) {
	VoiceParams p;
	p.looping = true;
	p.source3d.positional       = true;
	p.source3d.minDistance      = 1.0f;
	p.source3d.maxDistance      = 100.0f;
	p.source3d.rolloffFactor    = rolloff;
	p.source3d.attenuationModel = modele;
	p.source3d.position[0] = 0.0f;
	p.source3d.position[1] = 0.0f;
	p.source3d.position[2] = -distance;

	AudioHandle voix = AudioEngine::Instance().Play(son, p);

	const int32 TOTAL_FRAMES = 48000;
	const int32 BLOC = 256;
	std::vector<float32> buffer((size_t)BLOC * 2);

	double sommeCarres = 0.0;
	int32 fait = 0;
	while (fait < TOTAL_FRAMES) {
		int32 ceBloc = (TOTAL_FRAMES - fait < BLOC) ? (TOTAL_FRAMES - fait) : BLOC;
		AudioEngine::Instance().RenderToBuffer(buffer.data(), ceBloc, 2);
		for (int32 i = 0; i < ceBloc * 2; ++i) {
			double v = (double)buffer[i];
			sommeCarres += v * v;
		}
		fait += ceBloc;
	}

	AudioEngine::Instance().Stop(voix);

	return std::sqrt(sommeCarres / (double)(TOTAL_FRAMES * 2));
}

int main() {
	AudioEngineConfig cfg;
	cfg.sampleRate = 48000;
	cfg.backend = AudioBackendType::NULL_OUTPUT;
	if (!AudioEngine::Instance().Initialize(cfg)) {
		std::fprintf(stderr, "echec : initialisation audio (NULL_OUTPUT)\n");
		return 1;
	}

	AudioEngine::Instance().SetListenerPosition(0.f, 0.f, 0.f);
	AudioEngine::Instance().SetListenerOrientation(0.f, 0.f, -1.f, 0.f, 1.f, 0.f);

	AudioSample son = AudioGenerator::GenerateTone(440.f, 2.f, WaveformType::SINE, 48000, 0.1f);

	struct Modele { const char *nom; AttenuationModel valeur; };
	const Modele modeles[4] = {
		{"NONE", AttenuationModel::NONE},
		{"INVERSE", AttenuationModel::INVERSE},
		{"LINEAR", AttenuationModel::LINEAR},
		{"EXPONENTIAL", AttenuationModel::EXPONENTIAL},
	};

	struct Distance { const char *label; float32 valeur; };
	const Distance distances[5] = {
		{"0,5 m", 0.5f},
		{"1 m",   1.0f},
		{"2 m",   2.0f},
		{"4 m",   4.0f},
		{"8 m",   8.0f},
	};

	const float32 rolloffs[2] = {1.0f, 2.0f};

	FILE *f = std::fopen("mesures.txt", "w");
	if (!f) {
		std::fprintf(stderr, "echec : impossible d'ecrire mesures.txt\n");
		return 1;
	}

	// Pour le graphe (rolloff=1 uniquement) : on imprime aussi un CSV brut
	// a cote, plus simple a relire depuis Python que de reparser mesures.txt.
	FILE *csv = std::fopen("mesures_rolloff1.csv", "w");
	std::fprintf(csv, "modele,distance,gain\n");

	for (int32 im = 0; im < 4; ++im) {
		for (int32 ir = 0; ir < 2; ++ir) {
			float32 rolloff = rolloffs[ir];

			// RMS de NONE a cette distance/rolloff (reference du gain).
			double rmsNone[5];
			for (int32 id = 0; id < 5; ++id) {
				rmsNone[id] = MesurerRMS(son, AttenuationModel::NONE, distances[id].valeur, rolloff);
			}

			for (int32 id = 0; id < 5; ++id) {
				double rms = MesurerRMS(son, modeles[im].valeur, distances[id].valeur, rolloff);
				double gain = rms / rmsNone[id];
				std::printf("%s, rolloff %d, %s : %.3f\n", modeles[im].nom, (int)rolloff, distances[id].label, gain);
				std::fprintf(f, "%s, rolloff %d, %s : %.3f\n", modeles[im].nom, (int)rolloff, distances[id].label, gain);
				if (ir == 0) {
					std::fprintf(csv, "%s,%.1f,%.3f\n", modeles[im].nom, distances[id].valeur, gain);
				}
			}
		}
	}

	std::fclose(f);
	std::fclose(csv);

	AudioEngine::Instance().Shutdown();
	AudioLoader::Free(son);

	return 0;
}
