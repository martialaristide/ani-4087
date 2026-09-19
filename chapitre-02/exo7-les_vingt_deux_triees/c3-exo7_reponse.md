# Exercice 7 — Les vingt-trois triees

## Liste (23 dependances de NKXRDemo, tiree du vrai fichier NKXRDemo.jenga du depot Rihen-Universe/Nkentseu)

NKXR, NKRenderer, NKRHI, NKSL, NKGLSlang, NKSPIRVCross,
NKSerialization, NKReflection, NKFileSystem, NKFont, NKImage, NKGlad,
NKEvent, NKWindow, NKMath, NKTime, NKLogger, NKStream,
NKContainers, NKMemory, NKCore, NKPlatform, NKThreading

## Tas A — le nom seul dit le role (18)

NKXR, NKRenderer, NKGLSlang, NKSerialization, NKReflection, NKFileSystem,
NKFont, NKImage, NKEvent, NKWindow, NKMath, NKTime, NKLogger, NKStream,
NKContainers, NKMemory, NKPlatform, NKThreading

## Tas B — petite idee, sans certitude (2)

NKSL, NKCore

## Tas C — aucune idee (3)

NKRHI, NKSPIRVCross, NKGlad

## Une phrase par module du Tas C, apres lecture du vrai en-tete/source

**NKRHI** — d'apres son ROADMAP.md, c'est l'interface unique (`NkIDevice`)
qui unifie six moteurs graphiques differents (Vulkan, OpenGL, DirectX 11,
DirectX 12, Metal et un rasteriseur logiciel) derriere le meme jeu d'appels,
pour que le reste du moteur n'ait jamais a savoir laquelle est reellement
utilisee.

**NKSPIRVCross** — c'est un simple reconditionnement, pour le systeme de
build Jenga, du vrai outil SPIRV-Cross du Khronos Group : il convertit du
bytecode de shader compile (SPIR-V) vers du code source lisible dans
d'autres langages de shader (GLSL, HLSL, Metal Shading Language).

**NKGlad** — reconditionnement, toujours pour Jenga, de la bibliotheque
glad (David Herberth) : le code genere qui va chercher, au demarrage du
programme, les adresses reelles des fonctions OpenGL/EGL/GLES/Vulkan
fournies par le pilote graphique.
