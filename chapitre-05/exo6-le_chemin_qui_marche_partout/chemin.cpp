// ANI-4087 chapitre 5, exercice 6 : "Le chemin qui marche partout"
//
// Ce fichier est un EXTRAIT REEL, deja embarque et compile dans le moteur
// Nkentseu (Applications/Sandbox/src/Demo/Demo3D.cpp). Il n'est pas autonome
// ici (il depend de l'etat de la scene "salle" : st->, ctx.renderer, le
// logger...), il est donc documente comme extrait et non recompile isolement.
//
// L'enonce du cours decrit NkPath::LocateResource(chemin), une fonction qui
// n'existe PAS dans ce moteur (verifie : NKFileSystem/NkPath.h ne declare que
// GetCurrentDirectory, GetExecutableDirectory, GetTempDirectory, Combine,
// IsValidPath -- aucune fonction de ce nom, et une recherche texte sur tout
// le depot ne trouve "LocateResource" nulle part avant cet exercice). On
// ecrit donc notre propre variante avec les briques reellement disponibles.

#include "NKFileSystem/NkPath.h"
#include "NKContainers/String/NkString.h"
#include <cstdio>

using nkentseu::NkPath;
using nkentseu::NkString;

/*
 * Ordre de recherche de notre LocateResource (documente ici comme le demande
 * l'enonce, puisque le vrai NkPath.h n'a pas de commentaire a lire pour une
 * fonction qui n'existe pas) :
 *
 *   1. Le chemin relatif TEL QUEL, depuis le dossier COURANT -- marche si on
 *      lance depuis la racine du projet, casse des qu'on lance d'ailleurs.
 *   2. Le meme chemin, ANCRE au dossier de l'EXECUTABLE
 *      (NkPath::GetExecutableDirectory()) -- utile si les ressources sont
 *      copiees a cote du binaire (cas d'un paquet mobile).
 *   3. Le meme chemin, en remontant jusqu'a 4 dossiers PARENTS de
 *      l'executable -- cas de CE projet : l'executable vit sous
 *      Build/Bin/<config>-<os>/<nom>/, et remonter 4 niveaux retombe
 *      exactement sur la racine du projet ou se trouve murs/.
 *
 * Si rien n'est trouve apres ces trois etapes, renvoie une chaine VIDE --
 * jamais un chemin invente -- pour que l'appelant teste le resultat (avec
 * Empty()) avant de le donner a Load().
 */
static NkString LocateResource(const NkString &cheminRelatif) {
        auto existe = [](const char *p) -> bool {
                FILE *f = std::fopen(p, "rb");
                if (!f) return false;
                std::fclose(f);
                return true;
        };

        // Etape 1 : tel quel, depuis le dossier courant.
        if (existe(cheminRelatif.CStr()))
                return cheminRelatif;

        // Etape 2 : ancre au dossier de l'executable.
        NkPath exeDir = NkPath::GetExecutableDirectory();
        NkPath candidat = exeDir / cheminRelatif.CStr();
        if (existe(candidat.CStr()))
                return candidat.ToString();

        // Etape 3 : on remonte jusqu'a 4 dossiers parents de l'executable.
        NkPath remonte = exeDir;
        for (int i = 0; i < 4; ++i) {
                remonte = remonte / "..";
                NkPath c2 = remonte / cheminRelatif.CStr();
                if (existe(c2.CStr()))
                        return c2.ToString();
        }

        return NkString(""); // rien trouve : laisse l'appelant decider
}

/*
 * -- Utilisation dans Demo3D_Init, a la place du Load() direct de l'exo4 ---
 *
 * const NkString cheminMurBrique = LocateResource(NkString("murs/mur_brique.jpg"));
 * logger.Info("[ANI4087][exo6] LocateResource('{0}') -> '{1}'\n",
 *                         "murs/mur_brique.jpg", cheminMurBrique.CStr());
 * if (!cheminMurBrique.Empty()) {
 *         st->exo4Tex = texLib->Load(cheminMurBrique, optsMurBrique);
 * } else {
 *         logger.Errorf("[ANI4087][exo6] LocateResource n'a rien trouve pour '{0}'\n",
 *                                 "murs/mur_brique.jpg");
 * }
 */
