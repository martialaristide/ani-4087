// L'instant predit et sa periode : comme le simulateur du moteur, a chaque image,
//   predit = maintenant + periode      avec periode = 1e9 / cadence (nanosecondes)
// puis on confronte cet instant au DEBUT REEL de l'image suivante.
//
// Variables d'environnement :
//   MASALLE_HZ=<hz>      cadence (72 par defaut ; une valeur <= 1 donne aussi 72)
//   MASALLE_EXIT=<n>     arrete la salle seule apres n images (700 pour l'exercice)
//   MASALLE_API=<opengl|vulkan|dx11|dx12|sw>   API graphique (defaut opengl)
//   MASALLE_VSYNC=0      coupe la synchronisation verticale du renderer
//   MASALLE_LEGER=1      coupe les effets d'image (SSAO, bloom, TAA, FXAA) : diagnostic
//
// Les instants sont gardes en float64, en nanosecondes, et comptes depuis le lancement.
// Sortie standard : vingt lignes "maintenant / predit / ecart" (images 1 a 20), puis
// "hz = H, periode = P ms, reel - predit = R ms" (moyenne sur 600 images, 30 premieres ecartees).
// Sortie d'erreur : intervalle moyen, minimum et maximum de reel - predit.

#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <vector>

#include "NKPlatform/NkPlatformDetect.h"
#include "NKWindow/NKMain.h"
#include "NKWindow/Core/NkWindow.h"
#include "NKWindow/Core/NkWindowConfig.h"
#include "NKWindow/Core/NkWESystem.h"
#include "NKEvent/NkWindowEvent.h"
#include "NKEvent/NkKeyboardEvent.h"
#include "NKTime/NkChrono.h"
#include "NKLogger/NkLog.h"
#include "NKMath/NkAngle.h"
#include "NKMath/NkQuat.h"
#include "NKMath/NkMat.h"

#include "NKRHI/Core/NkDeviceFactory.h"
#include "NKRenderer/NkRenderer.h"
#include "NKRenderer/Core/NkCamera.h"
#include "NKRenderer/Mesh/NkMeshSystem.h"
#include "NKRenderer/Tools/Render3D/NkRender3D.h"

using namespace nkentseu;
using namespace nkentseu::math;
using namespace nkentseu::renderer;

static void ConfigureAppData(NkAppData &d) {
        d.appName = "LInstantPredit";
}
NK_REGISTER_ENTRY_APPDATA_UPDATER(ConfigureAppData)

namespace {
        const int kAffichees = 20; // lignes imprimees
        const int kEcartees = 30;  // images de demarrage ecartees
        const int kImages = 600;   // images gardees pour la moyenne

        // La salle des exercices precedents : 4 x 4 m, hauteur 2,5 m.
        void SubmitSalle(NkRender3D *r3d, NkMeshHandle cube) {
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
                boite({0.f, -0.05f, 0.f}, {4.2f, 0.1f, 4.2f}, {0.50f, 0.45f, 0.40f});
                boite({0.f, 2.55f, 0.f}, {4.2f, 0.1f, 4.2f}, {0.90f, 0.90f, 0.90f});
                boite({0.f, 1.25f, -2.05f}, {4.2f, 2.5f, 0.1f}, {0.55f, 0.65f, 0.85f});
                boite({0.f, 1.25f, 2.05f}, {4.2f, 2.5f, 0.1f}, {0.85f, 0.80f, 0.50f});
                boite({-2.05f, 1.25f, 0.f}, {0.1f, 2.5f, 4.2f}, {0.80f, 0.45f, 0.45f});
                boite({2.05f, 1.25f, 0.f}, {0.1f, 2.5f, 4.2f}, {0.45f, 0.75f, 0.50f});
                boite({0.f, 1.0f, -1.97f}, {0.9f, 2.0f, 0.06f}, {0.45f, 0.28f, 0.12f});
                boite({0.f, 2.45f, 0.f}, {0.6f, 0.1f, 0.6f}, {1.0f, 1.0f, 1.0f});
                boite({0.f, 1.6f, -1.92f}, {4.0f, 0.06f, 0.04f}, {0.04f, 0.04f, 0.08f});
                boite({0.f, 1.6f, 1.92f}, {4.0f, 0.06f, 0.04f}, {0.04f, 0.04f, 0.08f});
                boite({-1.92f, 1.6f, 0.f}, {0.04f, 0.06f, 4.0f}, {0.04f, 0.04f, 0.08f});
                boite({1.92f, 1.6f, 0.f}, {0.04f, 0.06f, 4.0f}, {0.04f, 0.04f, 0.08f});
        }
}

int nkmain(const NkEntryState &state) {
        (void)state;
        const double t0 = (double)NkChrono::Now().nanoseconds; // le lancement

        // ── La cadence et la periode, comme le simulateur du moteur ───────────────
        double hz = 72.0;
        if (const char *e = std::getenv("MASALLE_HZ"); e && e[0]) {
                const double v = std::atof(e);
                if (v > 1.0) hz = v; // displayHz > 1 ? displayHz : 72
        }
        const double periodeNs = 1e9 / hz;

        int imagesMax = 0;
        if (const char *e = std::getenv("MASALLE_EXIT"); e && e[0]) imagesMax = std::atoi(e);
        const char *vs = std::getenv("MASALLE_VSYNC");
        const bool vsyncCoupee = (vs && vs[0] == '0');
        const char *lg = std::getenv("MASALLE_LEGER");
        const bool leger = (lg && lg[0] == '1');

        // ── Fenetre, device, renderer ─────────────────────────────────────────────
        NkWindowConfig winCfg;
        winCfg.title = "L'instant predit (Echap = quitter)";
        winCfg.width = 1280;
        winCfg.height = 720;
        winCfg.centered = true;
        winCfg.resizable = false;

        NkWindow window(winCfg);
        if (!window.IsValid()) {
                logger.Error("[Instant] Creation fenetre KO");
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
                logger.Error("[Instant] Creation device KO");
                window.Close();
                return 2;
        }

        NkRendererConfig cfg = NkRendererConfig::ForGame(devInfo.api, devInfo.width, devInfo.height);
        if (vsyncCoupee) cfg.vsync = false;
        if (leger) {
                cfg.postProcess.ssao = false;
                cfg.postProcess.bloom = false;
                cfg.postProcess.taa = false;
                cfg.postProcess.fxaa = false;
        }
        NkRenderer *renderer = NkRenderer::Create(device, cfg);
        if (!renderer || !renderer->Initialize()) {
                logger.Error("[Instant] Init renderer KO");
                NkRenderer::Destroy(renderer);
                NkDeviceFactory::Destroy(device);
                window.Close();
                return 3;
        }
        NkRender3D *r3d = renderer->GetRender3D();
        NkMeshHandle meshCube = renderer->GetMeshSystem()->GetCube();
        const float aspect = (float)devInfo.width / (float)devInfo.height;

        bool running = true;
        NkEventSystem &events = NkEvents();
        events.AddEventCallback<NkWindowCloseEvent>([&](NkWindowCloseEvent *) { running = false; });
        events.AddEventCallback<NkKeyPressEvent>([&](NkKeyPressEvent *e) {
                if (e->GetKey() == NkKey::NK_ESCAPE) running = false;
        });

        // ── Boucle : au DEBUT de chaque image, maintenant et predit = maintenant + periode ──
        std::vector<double> maintenant, predit; // nanosecondes, float64
        maintenant.reserve(imagesMax > 0 ? (size_t)imagesMax + 4 : 4096);
        predit.reserve(imagesMax > 0 ? (size_t)imagesMax + 4 : 4096);
        int frame = 0;

        while (running && window.IsOpen()) {
                events.PollEvents();
                if (!running) break;
                const double now = (double)NkChrono::Now().nanoseconds;
                maintenant.push_back(now);
                predit.push_back(now + periodeNs);
                ++frame;

                if (renderer->BeginFrame()) {
                        const float lacet = (float)frame * 0.2f;
                        const NkQuatf q = NkQuatf::RotateY(NkAngle(lacet));
                        const NkVec3f oeil(0.0f, 1.6f, 0.0f);
                        NkCamera3DData camData;
                        camData.position = oeil;
                        camData.target = oeil + q * NkVec3f(0.0f, 0.0f, -1.0f);
                        camData.up = NkVec3f(0.0f, 1.0f, 0.0f);
                        camData.fovY = 60.f;
                        camData.aspect = aspect;
                        camData.nearPlane = 0.05f;
                        camData.farPlane = 50.f;

                        NkSceneContext sctx;
                        sctx.camera = NkCamera3D(camData);
                        sctx.time = 0.f;
                        sctx.ambientIntensity = 2.0f;
                        NkLightDesc sun;
                        sun.type = NkLightType::NK_DIRECTIONAL;
                        sun.direction = {-0.4f, -1.f, -0.3f};
                        sun.color = {1.f, 0.95f, 0.85f};
                        sun.intensity = 2.f;
                        sun.castShadow = false;
                        sctx.lights.PushBack(sun);

                        r3d->BeginScene(sctx);
                        SubmitSalle(r3d, meshCube);
                        renderer->Present();
                        renderer->EndFrame();
                }

                if (imagesMax > 0 && frame >= imagesMax) break;
        }

        NkRenderer::Destroy(renderer);
        NkDeviceFactory::Destroy(device);
        window.Close();

        // ── Sorties (apres la boucle, pour ne pas ralentir les images) ────────────
        std::fprintf(stderr, "images : %d ; leger : %s ; synchro coupee : %s\n", frame, leger ? "oui" : "non",
                     vsyncCoupee ? "oui" : "non");
        if ((int)maintenant.size() < kEcartees + kImages + 1) {
                std::fprintf(stderr, "pas assez d'images : %d, il en faut %d (MASALLE_EXIT=700)\n",
                             (int)maintenant.size(), kEcartees + kImages + 1);
                return 4;
        }

        // Les vingt premieres images : instants comptes depuis le lancement, en millisecondes.
        for (int i = 0; i < kAffichees; ++i)
                std::printf("maintenant = %.3f ms, predit = %.3f ms, ecart = %.3f ms\n", (maintenant[i] - t0) * 1e-6,
                            (predit[i] - t0) * 1e-6, (predit[i] - maintenant[i]) * 1e-6);

        // reel - predit : le debut REEL de l'image suivante moins l'instant predit a l'image d'avant.
        double somme = 0.0, mn = 1e300, mx = -1e300;
        for (int k = kEcartees; k < kEcartees + kImages; ++k) {
                const double r = (maintenant[k + 1] - predit[k]) * 1e-6;
                somme += r;
                if (r < mn) mn = r;
                if (r > mx) mx = r;
        }
        const double moyenne = somme / (double)kImages;
        std::printf("hz = %g, periode = %.3f ms, reel - predit = %.3f ms\n", hz, periodeNs * 1e-6, moyenne);
        std::fflush(stdout);
        std::fprintf(stderr, "intervalle moyen entre deux debuts d'image : %.3f ms ; reel - predit min %.3f ms, max %.3f ms\n",
                     moyenne + periodeNs * 1e-6, mn, mx);
        return 0;
}
