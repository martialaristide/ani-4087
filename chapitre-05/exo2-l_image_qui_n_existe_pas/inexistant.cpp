// ANI-4087 chapitre 5, exercice 2 : "L'image qui n'existe pas"
//
// Ce fichier regroupe les DEUX niveaux reels testes dans le moteur Nkentseu :
//
//   1) Le niveau bas : NkImage::Load() directement, sur un fichier qui n'existe
//      pas. Ce code a ete compile et execute tel quel (projet NkFormatsTest,
//      main.cpp temporairement remplace par ce test, puis restaure).
//
//   2) Le niveau haut : NkTextureLibrary::Load() + NkMaterial, utilise par le
//      renderer 3D. Ce code est un EXTRAIT REEL, deja embarque et compile dans
//      Applications/Sandbox/src/Demo/Demo3D.cpp (fonctions Demo3D_Init /
//      Demo3D_Frame / Demo3D_Shutdown). Il n'est pas autonome ici (il depend
//      de l'etat de la scene "salle" : st->, ctx.renderer, st->meshPlane...),
//      il est donc documente comme extrait et non recompile isolement.

// ───────────────────────────────────────────────────────────────────────────
// 1) NIVEAU BAS : NkImage::Load direct (reellement compile et execute)
// ───────────────────────────────────────────────────────────────────────────
#include "NKImage/Core/NkImage.h"
#include <cstdio>

int main() {
        nkentseu::NkImage img;
        bool charge = img.Load("murs/nexistepas.png");
        std::printf("charge : %d, valide : %d, largeur : %d, hauteur : %d\n",
                                charge ? 1 : 0, img.IsValid() ? 1 : 0, img.Width(), img.Height());
        return 0;
}

/*
 * ─────────────────────────────────────────────────────────────────────────
 * 2) NIVEAU HAUT : extrait reel de Demo3D.cpp (NkTextureLibrary + NkMaterial)
 * ─────────────────────────────────────────────────────────────────────────
 *
 * -- Dans Demo3DState (struct, vers la ligne 138) -----------------------
 *
 * NkMaterial *inexistantMat = nullptr;
 * NkTexHandle inexistantTex;
 *
 * -- Dans Demo3D_Init (vers la ligne 2792), apres le bloc du mur masque --
 *
 * if (texLib && ctx.renderer->GetMaterials()) {
 *         NkLoadOptions optsMurAbsent;
 *         optsMurAbsent.srgb = true;
 *         optsMurAbsent.genMipmaps = true;
 *         optsMurAbsent.useClampEdge = false;
 *         optsMurAbsent.debugName = "ANI4087_C5Exo2_MurAbsent";
 *         st->inexistantTex = texLib->Load(NkString("murs/nexistepas.png"), optsMurAbsent);
 *         const bool estTextureErreur = st->inexistantTex.id == texLib->GetError().id;
 *         logger.Info("[ANI4087][exo2] Load('{0}') -> valide={1}, texture_erreur={2}\n",
 *                                 "murs/nexistepas.png",
 *                                 st->inexistantTex.IsValid() ? "oui" : "non",
 *                                 estTextureErreur ? "oui" : "non");
 *         st->inexistantMat = NkMaterial::Create(ctx.renderer->GetMaterials(), NkMaterialType::NK_PBR_METALLIC);
 *         if (st->inexistantMat)
 *                 st->inexistantMat->SetAlbedoMap(st->inexistantTex)->SetRoughness(0.9f);
 * }
 *
 * -- Dans Demo3D_Frame (vers la ligne 5423), pose du mur -----------------
 *
 * if (st->inexistantMat) {
 *         NkDrawCall3D dc;
 *         dc.mesh = st->meshPlane;
 *         dc.transform = NkMat4f::Translate({-4.f, 1.5f, -4.f}) *
 *                                    NkMat4f::RotationX(NkAngle(90.f)) *
 *                                    NkMat4f::Scale({4.f, 1.f, 3.f});
 *         dc.aabb = {{-6.f, 0.f, -4.1f}, {-2.f, 3.f, -3.9f}};
 *         dc.material = st->inexistantMat->GetInstHandle();
 *         dc.castShadow = false;
 *         r3d->Submit(dc);
 * }
 *
 * -- Dans Demo3D_Shutdown (vers la ligne 7938) ----------------------------
 *
 * if (st && st->inexistantMat)
 *         NkMaterial::Destroy(st->inexistantMat);
 */
