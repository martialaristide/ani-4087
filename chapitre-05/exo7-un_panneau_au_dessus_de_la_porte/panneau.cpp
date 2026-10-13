// panneau.cpp
// ANI-4087 chapitre 5, exercice 7 : un panneau au-dessus de la porte.
//
// Ce programme construit l'image du mot "Sortie" en rasterisant
// manuellement, lettre par lettre, les glyphes d'une police embarquee
// dans le moteur (NkEmbeddedFontId::Inter, 32px), lus dans l'atlas de
// polices NkFontAtlas. Aucune lettre n'est recopiee "a l'oeil" : chaque
// pixel de sortie est calcule a partir des coordonnees UV du glyphe dans
// l'atlas (u0,v0,u1,v1), jamais du rectangle brut de l'atlas (qui peut
// etre plus grand que la lettre elle-meme, avec du padding autour).

#include "NKFont/NkFont.h"
#include "NKFont/Embedded/NkFontEmbedded.h"
#include "NKImage/Core/NkImage.h"
#include <cstdio>

using namespace nkentseu;

int main() {
	// --- Etape 1 : construire l'atlas avec la police embarquee -------------
	NkFontAtlas atlas;
	NkFont *police = NkFontEmbedded::AddToAtlas(atlas, NkEmbeddedFontId::Inter, 32.f);
	if (!police) {
		std::fprintf(stderr, "echec : AddToAtlas a renvoye nullptr (police Inter indisponible)\n");
		return 1;
	}
	if (!atlas.Build()) {
		std::fprintf(stderr, "echec : atlas.Build() a renvoye false\n");
		return 1;
	}

	// --- Etape 2 : recuperer les pixels bruts de l'atlas (alpha 8 bits) ----
	uint8 *atlasPixels = nullptr;
	int32 atlasW = 0, atlasH = 0;
	atlas.GetTexDataAsAlpha8(&atlasPixels, &atlasW, &atlasH);
	if (!atlasPixels || atlasW <= 0 || atlasH <= 0) {
		std::fprintf(stderr, "echec : GetTexDataAsAlpha8 n'a rien rendu\n");
		return 1;
	}
	std::printf("atlas : %d x %d\n", atlasW, atlasH);

	// --- Etape 3 : mesurer l'image de sortie --------------------------------
	const char *mot = "Sortie";
	const int32 nbLettres = 6;

	float32 largeurImg = 0.f;
	for (int32 i = 0; i < nbLettres; ++i) {
		const NkFontGlyph *g = police->FindGlyph((NkFontCodepoint)(unsigned char)mot[i]);
		if (!g) {
			std::fprintf(stderr, "echec : FindGlyph('%c') introuvable\n", mot[i]);
			return 1;
		}
		largeurImg += g->advanceX;
	}
	const float32 hauteurImgF = police->ascent - police->descent;
	const int32 largeur = (int32)(largeurImg + 0.5f);
	const int32 hauteur = (int32)(hauteurImgF + 0.5f);
	if (largeur <= 0 || hauteur <= 0) {
		std::fprintf(stderr, "echec : dimensions d'image invalides (%d x %d)\n", largeur, hauteur);
		return 1;
	}
	std::printf("image : %d x %d\n", largeur, hauteur);

	// --- Etape 4 : creer l'image de fond -------------------------------------
	// Fond bleu nuit, texte blanc casse : panneau de signalisation lisible.
	const math::NkColor couleurFond(0x10, 0x20, 0x3C, 0xFF);
	const math::NkColor couleurTexte(0xF2, 0xF2, 0xF0, 0xFF);
	const uint32 couleurFondPacked = 0x10203CFFu;

	NkImage panneau = NkImage::Create((uint32)largeur, (uint32)hauteur, 4, couleurFondPacked);
	if (!panneau.IsValid()) {
		std::fprintf(stderr, "echec : NkImage::Create a renvoye une image invalide\n");
		return 1;
	}

	// --- Etape 5 : poser chaque lettre, pixel par pixel ----------------------
	// La ligne de base est a `ascent` pixels du haut de l'image.
	const float32 baseline = police->ascent;
	float32 curseur = 0.f;

	for (int32 i = 0; i < nbLettres; ++i) {
		const NkFontGlyph *g = police->FindGlyph((NkFontCodepoint)(unsigned char)mot[i]);

		const float32 rectXDeb = curseur + g->x0;
		const float32 rectYDeb = baseline + g->y0;
		const float32 rectXFin = curseur + g->x1;
		const float32 rectYFin = baseline + g->y1;
		const float32 rectLargeur = rectXFin - rectXDeb;
		const float32 rectHauteur = rectYFin - rectYDeb;

		if (rectLargeur > 0.f && rectHauteur > 0.f) {
			const int32 pxDeb = (int32)rectXDeb;
			const int32 pyDeb = (int32)rectYDeb;
			const int32 pxFin = (int32)(rectXFin + 0.999f);
			const int32 pyFin = (int32)(rectYFin + 0.999f);

			for (int32 py = pyDeb; py < pyFin; ++py) {
				if (py < 0 || py >= hauteur) continue;
				for (int32 px = pxDeb; px < pxFin; ++px) {
					if (px < 0 || px >= largeur) continue;

					// Fraction de la position du pixel DANS LE RECTANGLE DE
					// LA LETTRE (pas dans le rectangle brut de l'atlas).
					const float32 fx = ((float32)px + 0.5f - rectXDeb) / rectLargeur;
					const float32 fy = ((float32)py + 0.5f - rectYDeb) / rectHauteur;

					const float32 u = g->u0 + fx * (g->u1 - g->u0);
					const float32 v = g->v0 + fy * (g->v1 - g->v0);

					int32 ax = (int32)(u * (float32)atlasW);
					int32 ay = (int32)(v * (float32)atlasH);
					if (ax < 0) ax = 0;
					if (ax >= atlasW) ax = atlasW - 1;
					if (ay < 0) ay = 0;
					if (ay >= atlasH) ay = atlasH - 1;

					const uint8 couverture = atlasPixels[ay * atlasW + ax];
					const float32 t = (float32)couverture / 255.f;

					math::NkColor pixelFinal(
						(uint8)(couleurFond.r + t * ((float32)couleurTexte.r - (float32)couleurFond.r)),
						(uint8)(couleurFond.g + t * ((float32)couleurTexte.g - (float32)couleurFond.g)),
						(uint8)(couleurFond.b + t * ((float32)couleurTexte.b - (float32)couleurFond.b)),
						255);
					panneau.SetPixel(px, py, pixelFinal);
				}
			}
		}

		curseur += g->advanceX;
	}

	if (!panneau.SavePNG("panneau.png")) {
		std::fprintf(stderr, "echec : SavePNG(\"panneau.png\") a renvoye false\n");
		return 1;
	}

	std::printf("panneau.png ecrit : %d x %d\n", largeur, hauteur);
	return 0;
}
