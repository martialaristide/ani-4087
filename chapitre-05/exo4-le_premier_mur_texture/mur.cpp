// ANI-4087 chapitre 5, exercice 4 : "Le premier mur texture"
//
// Ce fichier est un EXTRAIT REEL, deja embarque et compile dans le moteur
// Nkentseu (Applications/Sandbox/src/Demo/Demo3D.cpp, fonctions Demo3D_Init /
// Demo3D_Frame / Demo3D_Shutdown). Il n'est pas autonome ici (il depend de
// l'etat de la scene "salle" : st->, ctx.renderer, st->meshPlane...), il est
// donc documente comme extrait et non recompile isolement.
//
// La photo utilisee est une vraie photo de mur prise au telephone
// (murs/mur_brique.jpg, 1080x809, deja utilisee a l'exercice 1 de ce
// chapitre). Le chemin est relatif au dossier d'ou le programme est lance :
// lance depuis ailleurs, ce meme code ne trouverait plus le fichier (c'est
// exactement le piege signale par l'enonce).

/*
 * -- Dans Demo3DState (struct) --------------------------------------------
 *
 * NkMaterial *exo4Mat = nullptr;
 * NkTexHandle exo4Tex;
 *
 * -- Dans Demo3D_Init, AVANT la boucle de frame ----------------------------
 *
 * // Charge UNE SEULE FOIS ici. Recharger a chaque frame gaspillerait le GPU
 * // et, en plus, le moteur ne journalise la ligne de dimensions qu'au
 * // PREMIER chargement (NK_TEX_CACHE=0 force un nouveau chargement si on
 * // veut revoir cette ligne).
 * if (texLib && ctx.renderer->GetMaterials()) {
 *         NkLoadOptions optsMurBrique;
 *         optsMurBrique.srgb = true;
 *         optsMurBrique.genMipmaps = true;
 *         optsMurBrique.useClampEdge = false;
 *         optsMurBrique.debugName = "ANI4087_C5Exo4_MurBrique";
 *         st->exo4Tex = texLib->Load(NkString("murs/mur_brique.jpg"), optsMurBrique);
 *         const bool exo4TexOk = st->exo4Tex.id != texLib->GetError().id;
 *         logger.Info("[ANI4087][exo4] Load('{0}') -> valide={1}, ok={2}\n",
 *                                 "murs/mur_brique.jpg",
 *                                 st->exo4Tex.IsValid() ? "oui" : "non",
 *                                 exo4TexOk ? "oui" : "non");
 *         st->exo4Mat = NkMaterial::Create(ctx.renderer->GetMaterials(), NkMaterialType::NK_PBR_METALLIC);
 *         if (st->exo4Mat) {
 *                 st->exo4Mat->SetAlbedoMap(st->exo4Tex)->SetRoughness(0.85f);
 *                 st->exo4Mat->SetTriplanarTileSize(1.0f); // repete tous les metres au lieu d'etirer
 *         }
 * }
 *
 * -- Variante de "Pave" qui recoit un materiau explicite -------------------
 * // Les blocs de pose de mur deja presents dans la scene (sol, cloisons,
 * // panneau masque...) dupliquent chacun la construction d'un NkDrawCall3D ;
 * // cette fonction factorise ce geste pour le cas "un pave avec SON materiau".
 * static void PaveAvecMateriau(renderer::NkRender3D *r3d, NkMeshHandle mesh,
 *                                                          const NkMat4f &transform, const NkAABB &aabb,
 *                                                          NkMaterial *mat, bool castShadow) {
 *         if (!r3d || !mat) return;
 *         NkDrawCall3D dc;
 *         dc.mesh = mesh;
 *         dc.transform = transform;
 *         dc.aabb = aabb;
 *         dc.material = mat->GetInstHandle();
 *         dc.castShadow = castShadow;
 *         r3d->Submit(dc);
 * }
 *
 * -- Dans Demo3D_Frame, pose d'UN SEUL mur avec ce materiau -----------------
 * if (st->exo4Mat) {
 *         PaveAvecMateriau(r3d, st->meshPlane,
 *                                          NkMat4f::Translate({4.f, 1.5f, -4.f}) *
 *                                                  NkMat4f::RotationX(NkAngle(90.f)) *
 *                                                  NkMat4f::Scale({4.f, 1.f, 3.f}),
 *                                          NkAABB{{2.f, 0.f, -4.1f}, {6.f, 3.f, -3.9f}},
 *                                          st->exo4Mat, false);
 * }
 *
 * -- Dans Demo3D_Shutdown ----------------------------------------------------
 *
 * if (st && st->exo4Mat)
 *         NkMaterial::Destroy(st->exo4Mat);
 */
