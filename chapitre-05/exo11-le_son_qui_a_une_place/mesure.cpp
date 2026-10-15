// mesure.cpp
// ANI-4087 chapitre 5, exercice 11 : le son qui a une place.
//
// Mesure hors ligne (NULL_OUTPUT, aucune sortie materielle) de ce que le
// moteur envoie a chaque oreille pour 3 positions de la pluie (gauche,
// devant, droite, toutes a 2 m de l'auditeur), chacune avec puis sans le
// mode positional -- 6 mesures au total. L'auditeur est a l'origine et
// regarde vers -Z : avant (0,0,-1), haut (0,1,0), comme demande.
//
// Pour chaque mesure, on rend exactement 48000 trames avec RenderToBuffer,
// par blocs de 256 (le dernier bloc est plus court : 48000 = 187*256 + 128,
// 48000 n'etant pas un multiple exact de 256 -- rendu tel quel, pas
// arrondi), on accumule la somme des carres de chaque canal, puis on
// prend la racine carree de la moyenne (RMS) pour l'oreille gauche et
// pour la droite.
//
// Piege de l'enonce respecte : la source ne bouge jamais pendant qu'elle
// joue -- chaque mesure fait son propre Play() frais, puis Stop() avant
// la mesure suivante, jamais de deplacement a chaud.

#include "NKAudio/NkAudio.h"
#include <cstdio>
#include <cmath>
#include <vector>

using namespace nkentseu;
using namespace nkentseu::audio;

struct MesurePosition {
	const char *nom;
	float32 x, y, z;
};

// RMS gauche/droite pour une position et un mode positional donnes.
static void Mesurer(AudioSample &pluie, const MesurePosition &pos, bool positional,
                     float32 *outRmsG, float32 *outRmsD) {
	VoiceParams p;
	p.looping = true;
	p.source3d.position[0] = pos.x;
	p.source3d.position[1] = pos.y;
	p.source3d.position[2] = pos.z;
	p.source3d.positional  = positional;
	p.source3d.minDistance = 0.5f;
	p.source3d.maxDistance = 6.0f;

	AudioHandle voix = AudioEngine::Instance().Play(pluie, p);

	const int32 TOTAL_FRAMES = 48000;
	const int32 BLOC = 256;
	std::vector<float32> buffer((size_t)BLOC * 2);

	double sommeCarresG = 0.0, sommeCarresD = 0.0;
	int32 fait = 0;
	while (fait < TOTAL_FRAMES) {
		int32 ceBloc = (TOTAL_FRAMES - fait < BLOC) ? (TOTAL_FRAMES - fait) : BLOC;
		AudioEngine::Instance().RenderToBuffer(buffer.data(), ceBloc, 2);
		for (int32 i = 0; i < ceBloc; ++i) {
			float32 g = buffer[(size_t)i * 2 + 0];
			float32 d = buffer[(size_t)i * 2 + 1];
			sommeCarresG += (double)g * (double)g;
			sommeCarresD += (double)d * (double)d;
		}
		fait += ceBloc;
	}

	AudioEngine::Instance().Stop(voix);

	*outRmsG = (float32)std::sqrt(sommeCarresG / (double)TOTAL_FRAMES);
	*outRmsD = (float32)std::sqrt(sommeCarresD / (double)TOTAL_FRAMES);
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

	AudioSample pluie = AudioGenerator::GenerateNoise(4.f, WaveformType::NOISE_PINK);

	const MesurePosition positions[3] = {
		{"gauche", -2.f, 0.f,  0.f},
		{"devant",  0.f, 0.f, -2.f},
		{"droite",  2.f, 0.f,  0.f},
	};

	FILE *f = std::fopen("mesures.txt", "w");
	if (!f) {
		std::fprintf(stderr, "echec : impossible d'ecrire mesures.txt\n");
		return 1;
	}

	for (int32 i = 0; i < 3; ++i) {
		for (int32 posFlag = 1; posFlag >= 0; --posFlag) {
			float32 rmsG = 0.f, rmsD = 0.f;
			Mesurer(pluie, positions[i], posFlag != 0, &rmsG, &rmsD);
			std::printf("%s %d : G %.6f D %.6f\n", positions[i].nom, posFlag, rmsG, rmsD);
			std::fprintf(f, "%s %d : G %.6f D %.6f\n", positions[i].nom, posFlag, rmsG, rmsD);
		}
	}

	std::fclose(f);

	AudioEngine::Instance().Shutdown();
	AudioLoader::Free(pluie);

	return 0;
}
