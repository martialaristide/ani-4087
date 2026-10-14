// sdf.cpp
// ANI-4087 chapitre 5, exercice 8 : le SDF mesure.
//
// Refait le panneau "Sortie" de deux facons -- couverture ordinaire et SDF
// (distance signee) -- a partir de la MEME police embarquee Inter 32px, et
// compare les deux a l'identique (meme taille d'image 8x, meme lecture
// bilineaire de l'atlas), pour isoler l'effet de la SEULE methode.
//
// CONSTAT IMPORTANT (verifie, pas suppose) : NkFontAtlas::Build() avec
// sdfMode=true construit bien un atlas (texWidth/texHeight corrects, voir
// atlas.txt), mais son CONTENU est casse dans ce build du moteur. Preuve :
//   - a une position ou l'atlas ORDINAIRE vaut 255 (plein coeur d'un trait
//     du glyphe 'o', verifie par balayage), l'atlas SDF vaut 0 au MEME pixel
//     (169,121), alors que 0 ne devrait jamais apparaitre en plein milieu
//     d'un trait epais pour une distance signee correcte.
//   - l'histogramme de l'atlas SDF entier (512x512) a 98.5% de ses pixels
//     sous la valeur 50, et seulement 3 pixels (sur 262144) au-dessus de
//     200 : aucun plateau "profondement a l'interieur", ce qui est
//     impossible pour un vrai SDF avec spread=6 sur une police remplissant
//     une bonne partie de ses traits.
//   - cause trouvee en lisant le moteur : dans NkFontAtlas.cpp, l'appel
//     `nkfont::NkMakeSDFFromBitmap(dst, drawW, drawH, sdfDst, sdfSpread)`
//     passe `dst = texPixels + r.y*atlasW + r.x` (un pointeur dont la ligne
//     suivante est a +atlasW), mais NkMakeSDFFromBitmap (NkFontParser.cpp)
//     indexe ses propres buffers avec `alpha[py*w+px]` ou `w` est le
//     parametre recu, c'est-a-dire `drawW` (largeur LOCALE du glyphe), pas
//     `atlasW`. Des que atlasW != drawW (le cas normal dans un atlas
//     empaquete), la fonction lit/ecrit avec le mauvais pas de ligne :
//     c'est un bug de stride reel du moteur, pas une mauvaise lecture de
//     ma part de la convention documentee.
//
// En consequence : j'appelle quand meme atlas.sdfMode=true / sdfSpread=6 /
// Build() comme demande par l'enonce (pour mesurer et rendre dans atlas.txt
// la taille REELLE de cet atlas), mais pour fabriquer l'image "sdf" qui sert
// a la vraie comparaison, je recalcule moi-meme le champ de distance signee
// -- avec le MEME algorithme que celui du moteur (recherche de voisinage,
// seuil a 127.5, meme convention de spread), simplement avec le bon pas de
// ligne -- a partir de l'atlas de couverture ORDINAIRE, dont le contenu est
// lui verifie correct.

#include "NKFont/NkFont.h"
#include "NKFont/Embedded/NkFontEmbedded.h"
#include "NKImage/Core/NkImage.h"
#include <cstdio>
#include <cmath>
#include <vector>

using namespace nkentseu;

// ── Regle du nuanceur SDF du moteur (kNkFontFragSDF), recopiee sur CPU ──────
static float32 smoothstep(float32 a, float32 b, float32 x) {
	float32 t = (x - a) / (b - a);
	if (t < 0.f) t = 0.f;
	if (t > 1.f) t = 1.f;
	return t * t * (3.f - 2.f * t);
}

// ── Lecture bilineaire de l'atlas (4 voisins) ───────────────────────────────
static uint8 SampleBilinear(const uint8 *pixels, int32 texW, int32 texH, float32 px, float32 py) {
	const float32 fx = px - 0.5f;
	const float32 fy = py - 0.5f;
	int32 x0 = (int32)std::floor(fx);
	int32 y0 = (int32)std::floor(fy);
	const float32 tx = fx - (float32)x0;
	const float32 ty = fy - (float32)y0;
	int32 x1 = x0 + 1, y1 = y0 + 1;
	auto clampi = [](int32 v, int32 lo, int32 hi) { return v < lo ? lo : (v > hi ? hi : v); };
	x0 = clampi(x0, 0, texW - 1);
	x1 = clampi(x1, 0, texW - 1);
	y0 = clampi(y0, 0, texH - 1);
	y1 = clampi(y1, 0, texH - 1);
	const float32 v00 = (float32)pixels[y0 * texW + x0];
	const float32 v10 = (float32)pixels[y0 * texW + x1];
	const float32 v01 = (float32)pixels[y1 * texW + x0];
	const float32 v11 = (float32)pixels[y1 * texW + x1];
	const float32 vx0 = v00 + (v10 - v00) * tx;
	const float32 vx1 = v01 + (v11 - v01) * tx;
	return (uint8)(vx0 + (vx1 - vx0) * ty + 0.5f);
}

// ── Mon propre champ de distance signee, calcule depuis l'atlas de
// COUVERTURE ordinaire (qui, lui, est correct) -- meme algorithme que
// nkfont::NkMakeSDFFromBitmap (seuil 127.5, recherche de voisinage dans un
// rayon spread), mais applique avec le VRAI pas de ligne (atlasW), donc sans
// le bug de stride du moteur.
static std::vector<uint8> ConstruireSDFDepuisCouverture(const uint8 *couverture, int32 w, int32 h, int32 spread) {
	std::vector<uint8> sortie((size_t)w * (size_t)h, 0);
	const float32 threshold = 127.5f;
	for (int32 py = 0; py < h; ++py) {
		for (int32 px = 0; px < w; ++px) {
			const bool interieur = (float32)couverture[py * w + px] >= threshold;
			float32 minDistSq = (float32)(spread * spread + 1);
			int32 x0 = px - spread, x1 = px + spread;
			int32 y0 = py - spread, y1 = py + spread;
			if (x0 < 0) x0 = 0;
			if (x1 >= w) x1 = w - 1;
			if (y0 < 0) y0 = 0;
			if (y1 >= h) y1 = h - 1;
			for (int32 qy = y0; qy <= y1; ++qy) {
				for (int32 qx = x0; qx <= x1; ++qx) {
					const bool qInterieur = (float32)couverture[qy * w + qx] >= threshold;
					if (qInterieur == interieur) continue;
					const float32 dx = (float32)(qx - px), dy = (float32)(qy - py);
					const float32 dsq = dx * dx + dy * dy;
					if (dsq < minDistSq) minDistSq = dsq;
				}
			}
			float32 dist = (minDistSq > (float32)(spread * spread)) ? (float32)spread : std::sqrt(minDistSq);
			float32 signedDist = interieur ? dist : -dist;
			float32 norm = (signedDist / (float32)spread + 1.f) * 0.5f;
			if (norm < 0.f) norm = 0.f;
			if (norm > 1.f) norm = 1.f;
			sortie[(size_t)py * w + px] = (uint8)(norm * 255.f + 0.5f);
		}
	}
	return sortie;
}

static NkImage FabriquerPanneau(NkFont *police, const uint8 *pixels, int32 atlasW, int32 atlasH,
								 int32 largeurPx, int32 hauteurPx, float32 echelle, bool sdf, float32 s) {
	NkImage img = NkImage::Create((uint32)largeurPx, (uint32)hauteurPx, 4, 0x10203CFFu);
	if (!img.IsValid()) return img;
	const char *mot = "Sortie";
	const float32 baseline = police->ascent * echelle;
	float32 curseur = 0.f;
	for (int32 i = 0; i < 6; ++i) {
		const NkFontGlyph *g = police->FindGlyph((NkFontCodepoint)(unsigned char)mot[i]);
		const float32 rectXDeb = curseur + g->x0 * echelle, rectYDeb = baseline + g->y0 * echelle;
		const float32 rectXFin = curseur + g->x1 * echelle, rectYFin = baseline + g->y1 * echelle;
		const float32 rectLargeur = rectXFin - rectXDeb, rectHauteur = rectYFin - rectYDeb;
		if (rectLargeur > 0.f && rectHauteur > 0.f) {
			const int32 pxDeb = (int32)rectXDeb, pyDeb = (int32)rectYDeb;
			const int32 pxFin = (int32)(rectXFin + 0.999f), pyFin = (int32)(rectYFin + 0.999f);
			for (int32 py = pyDeb; py < pyFin; ++py) {
				if (py < 0 || py >= hauteurPx) continue;
				for (int32 px = pxDeb; px < pxFin; ++px) {
					if (px < 0 || px >= largeurPx) continue;
					const float32 fx = ((float32)px + 0.5f - rectXDeb) / rectLargeur;
					const float32 fy = ((float32)py + 0.5f - rectYDeb) / rectHauteur;
					const float32 u = g->u0 + fx * (g->u1 - g->u0), v = g->v0 + fy * (g->v1 - g->v0);
					const uint8 valeur = SampleBilinear(pixels, atlasW, atlasH, u * (float32)atlasW, v * (float32)atlasH);
					float32 t;
					if (sdf) {
						const float32 dist = (float32)valeur / 255.f;
						t = smoothstep(0.5f - s, 0.5f + s, dist);
					} else {
						t = (float32)valeur / 255.f;
					}
					math::NkColor pf((uint8)(0x10 + t * (0xF2 - 0x10)), (uint8)(0x20 + t * (0xF2 - 0x20)),
									  (uint8)(0x3C + t * (0xF0 - 0x3C)), 255);
					img.SetPixel(px, py, pf);
				}
			}
		}
		curseur += g->advanceX * echelle;
	}
	return img;
}

int main() {
	NkFontAtlas atlasOrd32;
	NkFont *policeOrd32 = NkFontEmbedded::AddToAtlas(atlasOrd32, NkEmbeddedFontId::Inter, 32.f);
	if (!policeOrd32 || !atlasOrd32.Build()) { std::fprintf(stderr, "echec ordinaire 32\n"); return 1; }

	NkFontAtlas atlasSdf32;
	atlasSdf32.sdfMode = true;
	atlasSdf32.sdfSpread = 6;
	NkFont *policeSdf32 = NkFontEmbedded::AddToAtlas(atlasSdf32, NkEmbeddedFontId::Inter, 32.f);
	if (!policeSdf32 || !atlasSdf32.Build()) { std::fprintf(stderr, "echec sdf 32\n"); return 1; }

	NkFontAtlas atlasOrd128;
	NkFont *policeOrd128 = NkFontEmbedded::AddToAtlas(atlasOrd128, NkEmbeddedFontId::Inter, 128.f);
	if (!policeOrd128 || !atlasOrd128.Build()) { std::fprintf(stderr, "echec ordinaire 128\n"); return 1; }

	std::printf("ordinaire 32 : %d x %d\n", atlasOrd32.texWidth, atlasOrd32.texHeight);
	std::printf("sdf 32 : %d x %d\n", atlasSdf32.texWidth, atlasSdf32.texHeight);
	std::printf("ordinaire 128 : %d x %d\n", atlasOrd128.texWidth, atlasOrd128.texHeight);

	uint8 *pixOrd = nullptr; int32 wOrd = 0, hOrd = 0;
	atlasOrd32.GetTexDataAsAlpha8(&pixOrd, &wOrd, &hOrd);

	// Mon propre SDF, calcule a partir de l'atlas de couverture (correct).
	std::vector<uint8> monSdf = ConstruireSDFDepuisCouverture(pixOrd, wOrd, hOrd, 6);

	const float32 echelle = 8.f;
	const char *mot = "Sortie";
	float32 largeurF = 0.f;
	for (int32 i = 0; i < 6; ++i)
		largeurF += policeOrd32->FindGlyph((NkFontCodepoint)(unsigned char)mot[i])->advanceX;
	const int32 largeurPx = (int32)(largeurF * echelle + 0.5f);
	const int32 hauteurPx = (int32)((policeOrd32->ascent - policeOrd32->descent) * echelle + 0.5f);
	std::printf("image (commune) : %d x %d\n", largeurPx, hauteurPx);

	NkImage imgOrd = FabriquerPanneau(policeOrd32, pixOrd, wOrd, hOrd, largeurPx, hauteurPx, echelle, false, 0.f);
	imgOrd.SavePNG("ordinaire.png");
	std::printf("ordinaire.png ecrit\n");

	const float32 sValeurs[3] = {0.02f, 0.05f, 0.10f};
	const char *sFichiers[3] = {"sdf_s002.png", "sdf_s005.png", "sdf_s010.png"};
	for (int32 i = 0; i < 3; ++i) {
		NkImage imgSdf = FabriquerPanneau(policeOrd32, monSdf.data(), wOrd, hOrd, largeurPx, hauteurPx, echelle, true, sValeurs[i]);
		imgSdf.SavePNG(sFichiers[i]);
		std::printf("%s ecrit (s=%.3f)\n", sFichiers[i], sValeurs[i]);
	}
	return 0;
}
