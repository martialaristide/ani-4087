// premier-son.cpp
// ANI-4087 chapitre 5, exercice 10 : le premier son.
//
// Initialise le moteur audio NKAudio, genere un ton de 440 Hz / 1 seconde
// avec AudioGenerator::GenerateTone (aucun fichier .wav fourni par
// l'enonce, donc son fabrique plutot que charge), le joue UNE FOIS au
// demarrage sans position dans l'espace (Play() simple, aucun parametre
// spatial), affiche les 4 valeurs reelles rendues par le moteur juste
// apres l'initialisation, attend que le son ait fini de jouer (la lecture
// se fait sur un autre fil que celui-ci, piege signale par l'enonce),
// puis ferme proprement : Shutdown() d'abord, Free() sur l'echantillon
// ensuite seulement.

#include "NKAudio/NkAudio.h"
#include <cstdio>
#include <thread>
#include <chrono>

using namespace nkentseu;
using namespace nkentseu::audio;

int main() {
	// ── Initialisation : une seule fois, avant toute lecture ──────────────
	AudioEngineConfig cfg;
	cfg.sampleRate = 48000;
	if (!AudioEngine::Instance().Initialize(cfg)) {
		std::printf("echec de l'initialisation du moteur audio\n");
		return 1;
	}

	// ── Les 4 valeurs reelles annoncees par le moteur, sans retouche ──────
	std::printf("dorsal : %s\n", AudioEngine::Instance().GetBackendName());
	std::printf("frequence : %d\n", AudioEngine::Instance().GetSampleRate());
	std::printf("tampon : %d\n", AudioEngine::Instance().GetBufferSize());
	std::printf("latence : %.3f\n", AudioEngine::Instance().GetLatencyMs());

	// ── Un son fabrique, joue une fois au demarrage, SANS position dans
	//    l'espace (Play() seul, pas de parametres 3D) ─────────────────────
	AudioSample son = AudioGenerator::GenerateTone(440.f, 1.f);
	AudioEngine::Instance().Play(son);

	// Le son joue sur un fil different de celui-ci : sans attente, le
	// programme se fermerait avant que quoi que ce soit ne sorte des
	// haut-parleurs. On attend la duree du son, plus une marge.
	std::this_thread::sleep_for(std::chrono::milliseconds(1200));

	// ── Fermeture : le moteur d'abord, puis liberation de l'echantillon
	//    (jamais l'inverse : une voix qui joue encore lirait des donnees
	//    liberees) ───────────────────────────────────────────────────────
	AudioEngine::Instance().Shutdown();
	AudioLoader::Free(son);

	return 0;
}
