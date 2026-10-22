// Les deux images et l'image noire : la meme salle dessinee deux fois, une par oeil,
// puis soustraction pixel par pixel des deux captures.
//
// Montage (celui de la demo stereo du moteur) :
//   un renderer "compositeur" (2D) possede la frame, cree les deux cibles et les
//   affiche cote a cote ; un renderer par oeil dessine la salle dans sa cible
//   (640 x 720) ; chaque renderer d'oeil recoit SetRenderSizeOverride(640, 720).
//
// Deux passes dans la meme execution :
//   images 1 a 30  : yeux decales de +-31,5 mm le long de la droite de la tete
//                    -> capture a l'image 30 : oeil-gauche.png, oeil-droit.png
//   images 31 a 60 : les deux cameras a la place de la tete (decalage oublie)
//                    -> capture a l'image 60 : oubli-gauche.png, oubli-droit.png
// Apres la boucle : relecture des PNG, ecart par pixel = plus grande difference des
// trois canaux, comptage des ecarts > 1, ecart maximal, ecriture des images de
// difference (quatre fois l'ecart, borne a 255 : noir = pareil).
//
// Sortie standard : les deux lignes de difference.txt.
// Sortie d'erreur : controles (images non vides, tailles).
//
// MASALLE_API=<opengl|vulkan|dx11|dx12|sw>  API graphique (defaut opengl)

#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "NKPlatform/NkPlatformDetect.h"
#include "NKWindow/NKMain.h"
#include "NKWindow/Core/NkWindow.h"
#include "NKWindow/Core/NkWindowConfig.h"
#include "NKWindow/Core/NkWESystem.h"
#include "NKEvent/NkWindowEvent.h"
#include "NKEvent/NkKeyboardEvent.h"
#include "NKLogger/NkLog.h"
#include "NKMath/NkAngle.h"
#include "NKMath/NkColor.h"
#include "NKMath/NkQuat.h"
#include "NKMath/NkMat.h"
#include "NKImage/Core/NkImage.h"

#include "NKRHI/Core/NkDeviceFactory.h"
#include "NKRenderer/NkRenderer.h"
#include "NKRenderer/Core/NkCamera.h"
#include "NKRenderer/Core/NkRenderGraph.h"
#include "NKRenderer/Core/NkTextureLibrary.h"
#include "NKRenderer/Materials/NkMaterialCollection.h"
#include "NKRenderer/Mesh/NkMeshSystem.h"
#include "NKRenderer/Tools/Offscreen/NkOffscreenTarget.h"
#include "NKRenderer/Tools/Render3D/NkRender3D.h"
#include "NKRenderer/Tools/Render2D/NkRender2D.h"
#include "NKRenderer/Tools/Overlay/NkOverlayRenderer.h"

#ifdef DrawText
#undef DrawText
#endif

using namespace nkentseu;
using namespace nkentseu::math;
using namespace nkentseu::renderer;

static void ConfigureAppData(NkAppData &d) {
        d.appName = "LesDeuxYeux";
}
NK_REGISTER_ENTRY_APPDATA_UPDATER(ConfigureAppData)

namespace {
        const uint32 kLargeurOeil = 640;
        const uint32 kHauteurOeil = 720;
        const float kIpd = 0.063f;                    // ecart entre les pupilles, en metres
        const NkVec3f kTete(0.30f, 1.70f, 1.20f);     // position de la tete
        const uint64 kImageCapture1 = 30;             // deux yeux
        const uint64 kImageCapture2 = 60;             // vue oubliee

        // La meme salle pour les deux yeux : 4 x 4 m, hauteur 2,5 m, avant = -Z.
        // Une caisse proche, un pilier et une sphere a des distances differentes :
        // la difference entre les yeux depend de la distance, il faut des distances variees.
        void SubmitSalle(NkRender3D *r3d, NkMeshSystem *meshes) {
                NkMeshHandle cube = meshes->GetCube();
                NkMeshHandle sphere = meshes->GetSphere();

                auto boite = [&](NkVec3f c, NkVec3f s, NkVec3f tint) {
                        NkDrawCall3D dc;
                        dc.mesh = cube;
                        dc.transform = NkMat4f::Translate(c) * NkMat4f::Scale(s);
                        dc.aabb = {{c.x - s.x * 0.5f, c.y - s.y * 0.5f, c.z - s.z * 0.5f},
                                   {c.x + s.x * 0.5f, c.y + s.y * 0.5f, c.z + s.z * 0.5f}};
                        dc.castShadow = false;
                        dc.tint = tint;
                        dc.metallic = 0.f;
                        dc.roughness = 1.f;
                        r3d->Submit(dc);
                };

                boite({0.f, -0.05f, 0.f}, {4.2f, 0.1f, 4.2f}, {0.50f, 0.45f, 0.40f});   // sol
                boite({0.f, 2.55f, 0.f}, {4.2f, 0.1f, 4.2f}, {0.90f, 0.90f, 0.90f});    // plafond
                boite({0.f, 1.25f, -2.05f}, {4.2f, 2.5f, 0.1f}, {0.55f, 0.65f, 0.85f}); // mur avant, bleu
                boite({0.f, 1.25f, 2.05f}, {4.2f, 2.5f, 0.1f}, {0.85f, 0.80f, 0.50f});  // mur arriere, jaune
                boite({-2.05f, 1.25f, 0.f}, {0.1f, 2.5f, 4.2f}, {0.80f, 0.45f, 0.45f}); // mur gauche, rouge
                boite({2.05f, 1.25f, 0.f}, {0.1f, 2.5f, 4.2f}, {0.45f, 0.75f, 0.50f});  // mur droit, vert
                boite({0.f, 1.0f, -1.97f}, {0.9f, 2.0f, 0.06f}, {0.45f, 0.28f, 0.12f}); // porte
                boite({0.f, 2.45f, 0.f}, {0.6f, 0.1f, 0.6f}, {1.0f, 1.0f, 1.0f});       // lampe

                // Bande sombre a 1,60 m sur les quatre murs.
                boite({0.f, 1.6f, -1.92f}, {4.0f, 0.06f, 0.04f}, {0.04f, 0.04f, 0.08f});
                boite({0.f, 1.6f, 1.92f}, {4.0f, 0.06f, 0.04f}, {0.04f, 0.04f, 0.08f});
                boite({-1.92f, 1.6f, 0.f}, {0.04f, 0.06f, 4.0f}, {0.04f, 0.04f, 0.08f});
                boite({1.92f, 1.6f, 0.f}, {0.04f, 0.06f, 4.0f}, {0.04f, 0.04f, 0.08f});

                // Objets a des distances differentes de la tete (z = 1,20) :
                boite({0.45f, 1.30f, 0.60f}, {0.25f, 0.25f, 0.25f}, {0.95f, 0.30f, 0.20f}); // caisse proche (~0,6 m)
                boite({-0.90f, 1.25f, -0.60f}, {0.30f, 2.5f, 0.30f}, {0.30f, 0.30f, 0.90f}); // pilier (~1,9 m)

                NkDrawCall3D dc; // sphere
                dc.mesh = sphere;
                dc.transform = NkMat4f::Translate({0.9f, 1.0f, -1.2f}) * NkMat4f::Scale({0.5f, 0.5f, 0.5f});
                dc.aabb = {{0.65f, 0.75f, -1.45f}, {1.15f, 1.25f, -0.95f}};
                dc.castShadow = false;
                dc.tint = {1.f, 0.85f, 0.2f};
                dc.metallic = 0.f;
                dc.roughness = 0.5f;
                r3d->Submit(dc);
        }

        struct Comparaison {
                bool ok = false;
                long long n = 0;       // pixels dont l'ecart depasse 1
                long long total = 0;   // T
                int ecartMax = 0;
        };

        // Recharge deux PNG, compare pixel par pixel, ecrit l'image de difference.
        Comparaison Comparer(const char *fichierG, const char *fichierD, const char *fichierDiff) {
                Comparaison res;
                NkImage a, b;
                if (!a.Load(fichierG, 4) || !b.Load(fichierD, 4)) {
                        std::fprintf(stderr, "comparaison : lecture impossible (%s, %s)\n", fichierG, fichierD);
                        return res;
                }
                const int w = a.Width(), h = a.Height();
                if (w != b.Width() || h != b.Height()) {
                        std::fprintf(stderr, "comparaison : tailles differentes (%dx%d contre %dx%d)\n", w, h,
                                     b.Width(), b.Height());
                        return res;
                }
                NkImage diff = NkImage::Create((uint32)w, (uint32)h, 4);
                if (!diff.IsValid()) {
                        std::fprintf(stderr, "comparaison : image de difference non creee\n");
                        return res;
                }

                // Controle "deux images blanches" : pixels qui different du pixel du coin haut-gauche.
                const NkColor coinA = a.GetPixel(0, 0), coinB = b.GetPixel(0, 0);
                long long nonFondA = 0, nonFondB = 0;

                for (int y = 0; y < h; ++y) {
                        for (int x = 0; x < w; ++x) {
                                const NkColor ca = a.GetPixel(x, y);
                                const NkColor cb = b.GetPixel(x, y);
                                int e = std::abs((int)ca.r - (int)cb.r);
                                const int eg = std::abs((int)ca.g - (int)cb.g);
                                const int eb = std::abs((int)ca.b - (int)cb.b);
                                if (eg > e) e = eg;
                                if (eb > e) e = eb;
                                if (e > 1) ++res.n;
                                if (e > res.ecartMax) res.ecartMax = e;
                                int v = 4 * e;
                                if (v > 255) v = 255;
                                diff.SetPixel(x, y, NkColor((uint8)v, (uint8)v, (uint8)v, 255));
                                if (ca.r != coinA.r || ca.g != coinA.g || ca.b != coinA.b) ++nonFondA;
                                if (cb.r != coinB.r || cb.g != coinB.g || cb.b != coinB.b) ++nonFondB;
                        }
                }
                res.total = (long long)w * (long long)h;
                std::fprintf(stderr, "controle %s / %s : %dx%d, pixels differents du coin : %lld et %lld sur %lld\n",
                             fichierG, fichierD, w, h, nonFondA, nonFondB, res.total);
                res.ok = diff.SavePNG(fichierDiff);
                if (!res.ok) std::fprintf(stderr, "comparaison : ecriture de %s impossible\n", fichierDiff);
                return res;
        }
}

int nkmain(const NkEntryState &state) {
        (void)state;

        // ── 1) Fenetre, device (OpenGL par defaut : DX12 ne compile pas les shaders ici) ──
        NkWindowConfig winCfg;
        winCfg.title = "Les deux yeux (Echap = quitter)";
        winCfg.width = 1280;
        winCfg.height = 720;
        winCfg.centered = true;
        winCfg.resizable = false;

        NkWindow window(winCfg);
        if (!window.IsValid()) {
                logger.Error("[DeuxYeux] Creation fenetre KO");
                return 1;
        }

        NkDeviceInitInfo devInfo{};
        devInfo.surface = window.GetSurfaceDesc();
        devInfo.width = (uint32)window.GetSize().width;
        devInfo.height = (uint32)window.GetSize().height;

        NkGraphicsApi apiChoisie = NkGraphicsApi::NK_GFX_API_OPENGL;
        if (const char *a = std::getenv("MASALLE_API")) {
                if (!std::strcmp(a, "vulkan")) apiChoisie = NkGraphicsApi::NK_GFX_API_VULKAN;
                else if (!std::strcmp(a, "dx11")) apiChoisie = NkGraphicsApi::NK_GFX_API_DX11;
                else if (!std::strcmp(a, "dx12")) apiChoisie = NkGraphicsApi::NK_GFX_API_DX12;
                else if (!std::strcmp(a, "sw")) apiChoisie = NkGraphicsApi::NK_GFX_API_SOFTWARE;
        }
        devInfo.api = apiChoisie;
        NkIDevice *device = NkDeviceFactory::CreateForApi(apiChoisie, devInfo);
        if (!device || !device->IsValid()) {
                logger.Error("[DeuxYeux] Creation device KO");
                window.Close();
                return 2;
        }

        const uint32 W = devInfo.width;
        const uint32 H = devInfo.height;

        // ── 2) Le compositeur : possede la frame, cree les deux cibles ──────────────
        NkRendererConfig cfgMain = NkRendererConfig::For2D(devInfo.api, W, H);
        cfgMain.Enable(NK_SS_OFFSCREEN);
        NkRenderer *rMain = NkRenderer::Create(device, cfgMain);
        if (!rMain) {
                logger.Error("[DeuxYeux] Init compositeur KO");
                NkDeviceFactory::Destroy(device);
                window.Close();
                return 3;
        }

        // ── 3) Deux cibles (640 x 720) et deux renderers de salle, un par oeil ──────
        NkOffscreenTarget *cible[2] = {nullptr, nullptr};
        NkRenderer *rOeil[2] = {nullptr, nullptr};
        for (int e = 0; e < 2; ++e) {
                NkOffscreenDesc od;
                od.width = kLargeurOeil;
                od.height = kHauteurOeil;
                od.hasDepth = true;
                od.hdr = false;
                od.colorFmt = ::nkentseu::NkGPUFormat::NK_RGBA8_UNORM;
                od.readable = true;
                od.readback = true;
                od.name = (e == 0) ? "DeuxYeux_Gauche" : "DeuxYeux_Droit";
                cible[e] = rMain->CreateOffscreen(od);
                if (!cible[e]) {
                        logger.Errorf("[DeuxYeux] Cible %d KO\n", e);
                        return 4;
                }

                NkRendererConfig cfgEye = NkRendererConfig::ForGame(devInfo.api, kLargeurOeil, kHauteurOeil);
                cfgEye.Disable(NK_SS_RENDER2D);
                cfgEye.Disable(NK_SS_TEXT);
                cfgEye.Disable(NK_SS_OVERLAY);
                cfgEye.Enable(NK_SS_OFFSCREEN);
                // Pas d'effet qui garde une memoire d'une image a l'autre : la difference
                // doit venir de la position des yeux, pas d'un historique.
                cfgEye.postProcess.ssao = false;
                cfgEye.postProcess.bloom = false;
                cfgEye.postProcess.taa = false;
                cfgEye.postProcess.fxaa = false;
                rOeil[e] = NkRenderer::Create(device, cfgEye);
                if (!rOeil[e]) {
                        logger.Errorf("[DeuxYeux] Renderer oeil %d KO\n", e);
                        return 5;
                }
                // Chaque renderer a la taille de SA cible.
                rOeil[e]->SetRenderSizeOverride(kLargeurOeil, kHauteurOeil);
                rOeil[e]->SetFinalColorTarget(rMain->GetTextures()->GetRHIHandle(cible[e]->GetColorResult()));
        }

        bool running = true;
        NkEventSystem &events = NkEvents();
        events.AddEventCallback<NkWindowCloseEvent>([&](NkWindowCloseEvent *) { running = false; });
        events.AddEventCallback<NkKeyPressEvent>([&](NkKeyPressEvent *e) {
                if (e->GetKey() == NkKey::NK_ESCAPE) running = false;
        });

        // ── 4) La tete : droite, regard vers -Z. La droite de la tete = q * (1,0,0). ──
        const NkQuatf teteQ = NkQuatf::Identity();
        const NkVec3f droiteTete = teteQ * NkVec3f(1.0f, 0.0f, 0.0f);
        const NkVec3f regard = teteQ * NkVec3f(0.0f, 0.0f, -1.0f);
        const NkVec3f haut = teteQ * NkVec3f(0.0f, 1.0f, 0.0f);
        const float demiEcart = kIpd * 0.5f;

        // ── 5) Boucle : 60 images rendues ──────────────────────────────────────────
        uint64 frame = 0;
        uint64 tentatives = 0;
        bool capture1 = false, capture2 = false;
        bool okCapture[4] = {false, false, false, false};

        while (running && window.IsOpen() && frame < kImageCapture2 && tentatives < 5000) {
                events.PollEvents();
                if (!running) break;
                ++tentatives;
                if (!rMain->BeginFrame()) continue;
                ++frame;
                NkICommandBuffer *cmd = rMain->GetCmd();

                const bool oubli = frame > kImageCapture1; // 2e passe : yeux non decales
                for (int e = 0; e < 2; ++e) {
                        NkRenderer *r = rOeil[e];
                        NkRender3D *r3d = r->GetRender3D();
                        // Ce que BeginFrame ferait si ce renderer possedait la frame.
                        // Oublier ResetFrame : le 2e oeil heriterait de ce que le 1er a laisse.
                        r->FlushGraphRebuilds();
                        r3d->ResetFrame();
                        if (auto *mc = r->GetMaterialCollection()) mc->Upload();

                        const float cote = (e == 0) ? -demiEcart : +demiEcart; // gauche = -, droit = +
                        const float decalage = oubli ? 0.0f : cote;
                        NkCamera3DData cd;
                        cd.position = NkVec3f(kTete.x + droiteTete.x * decalage, kTete.y + droiteTete.y * decalage,
                                              kTete.z + droiteTete.z * decalage);
                        cd.target = NkVec3f(cd.position.x + regard.x, cd.position.y + regard.y, cd.position.z + regard.z);
                        cd.up = haut;
                        cd.fovY = 60.f;
                        cd.aspect = (float32)kLargeurOeil / (float32)kHauteurOeil;
                        cd.nearPlane = 0.05f;
                        cd.farPlane = 50.f;

                        NkSceneContext sctx;
                        sctx.camera = NkCamera3D(cd);
                        sctx.time = 0.f;
                        sctx.ambientIntensity = 2.0f;

                        NkLightDesc sun;
                        sun.type = NkLightType::NK_DIRECTIONAL;
                        sun.direction = {-0.4f, -1.f, -0.3f};
                        sun.color = {1.f, 0.95f, 0.85f};
                        sun.intensity = 2.f;
                        sun.castShadow = false;
                        sctx.lights.PushBack(sun);
                        NkLightDesc fill;
                        fill.type = NkLightType::NK_DIRECTIONAL;
                        fill.direction = {0.3f, 1.f, 0.2f};
                        fill.color = {1.f, 1.f, 1.f};
                        fill.intensity = 1.5f;
                        fill.castShadow = false;
                        sctx.lights.PushBack(fill);

                        r3d->BeginScene(sctx);
                        SubmitSalle(r3d, r->GetMeshSystem()); // la MEME salle pour les deux yeux
                        // Le graphe de l'oeil s'execute sur le tampon de commandes de l'ecran.
                        if (auto *graph = r->GetRenderGraph()) graph->Execute(cmd);
                }

                // Les deux cibles cote a cote dans la fenetre.
                NkOverlayRenderer *overlay = rMain->GetOverlay();
                NkRender2D *r2d = rMain->GetRender2D();
                if (overlay && r2d) {
                        overlay->BeginOverlay(cmd, W, H);
                        r2d->DrawImage(cible[0]->GetColorResult(), {0.f, 0.f, 640.f, 720.f});
                        r2d->DrawImage(cible[1]->GetColorResult(), {640.f, 0.f, 640.f, 720.f});
                        overlay->EndOverlay();
                }
                rMain->Present();
                rMain->EndFrame();

                // Les deux yeux sont captures a la MEME image.
                if (frame == kImageCapture1) {
                        device->WaitIdle();
                        okCapture[0] = cible[0]->Capture("oeil-gauche.png");
                        okCapture[1] = cible[1]->Capture("oeil-droit.png");
                        capture1 = true;
                } else if (frame == kImageCapture2) {
                        device->WaitIdle();
                        okCapture[2] = cible[0]->Capture("oubli-gauche.png");
                        okCapture[3] = cible[1]->Capture("oubli-droit.png");
                        capture2 = true;
                }
        }

        // ── 6) Fermeture propre ────────────────────────────────────────────────────
        device->WaitIdle();
        for (int e = 0; e < 2; ++e) {
                NkRenderer::Destroy(rOeil[e]);
                rMain->DestroyOffscreen(cible[e]);
        }
        NkRenderer::Destroy(rMain);
        NkDeviceFactory::Destroy(device);
        window.Close();

        // ── 7) Soustractions, apres la boucle ──────────────────────────────────────
        std::fprintf(stderr, "captures : %d %d %d %d (images rendues : %llu)\n", okCapture[0], okCapture[1],
                     okCapture[2], okCapture[3], (unsigned long long)frame);
        if (!capture1 || !capture2) {
                std::fprintf(stderr, "captures incompletes : pas de comparaison\n");
                return 6;
        }
        const Comparaison deux = Comparer("oeil-gauche.png", "oeil-droit.png", "difference.png");
        const Comparaison oubli = Comparer("oubli-gauche.png", "oubli-droit.png", "difference-vue-oubliee.png");
        if (!deux.ok || !oubli.ok) return 7;

        std::printf("deux yeux : %lld pixels sur %lld, ecart max %d\n", deux.n, deux.total, deux.ecartMax);
        std::printf("vue oubliee : %lld pixels sur %lld, ecart max %d\n", oubli.n, oubli.total, oubli.ecartMax);
        std::fflush(stdout);
        return 0;
}
