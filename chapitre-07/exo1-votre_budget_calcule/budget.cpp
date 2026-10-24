// Votre budget calcule : le budget d'une image pour trois cadences, puis le temps reel entre
// deux debuts d'image de la salle.
//
// Au demarrage (avant la fenetre) : budget = 1000 / cadence, en millisecondes.
// Mesure : l'instant de DEBUT de chaque image (NkChrono::Now().nanoseconds, horloge qui ne
// recule jamais). Le temps d'une image est l'intervalle entre deux debuts successifs. On ecarte
// les 30 premieres images (le demarrage) et on garde 1000 intervalles.
//
// Variables d'environnement :
//   MASALLE_EXIT=<n>         arrete la salle seule apres n images (1040 pour l'exercice)
//   NK_VK_MAILBOX=1          mesure sans l'attente de l'ecran (Vulkan seulement)
//   MASALLE_API=<opengl|vulkan|dx11|dx12|sw>   API graphique (defaut opengl)
//   MASALLE_VSYNC=0          coupe la synchronisation verticale (cfg.vsync = false), hors Vulkan
//
// Sortie standard : les trois budgets, puis la ligne de mesure ("ecran" ou "sans attente").
// Sortie d'erreur : l'API, le nombre d'images, les premiers intervalles.

#include <algorithm>
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
        d.appName = "VotreBudgetCalcule";
}
NK_REGISTER_ENTRY_APPDATA_UPDATER(ConfigureAppData)

namespace {
        const int kEcartees = 30;     // images de demarrage ecartees
        const int kIntervalles = 1000; // intervalles gardes

        double BudgetMs(double hertz) { return 1000.0 / hertz; }

        // La salle de l'exercice 6 du chapitre 6 : 4 x 4 m, hauteur 2,5 m.
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
                boite({0.f, -0.05f, 0.f}, {4.2f, 0.1f, 4.2f}, {0.50f, 0.45f, 0.40f});   // sol
                boite({0.f, 2.55f, 0.f}, {4.2f, 0.1f, 4.2f}, {0.90f, 0.90f, 0.90f});    // plafond
                boite({0.f, 1.25f, -2.05f}, {4.2f, 2.5f, 0.1f}, {0.55f, 0.65f, 0.85f}); // mur avant
                boite({0.f, 1.25f, 2.05f}, {4.2f, 2.5f, 0.1f}, {0.85f, 0.80f, 0.50f});  // mur arriere
                boite({-2.05f, 1.25f, 0.f}, {0.1f, 2.5f, 4.2f}, {0.80f, 0.45f, 0.45f}); // mur gauche
                boite({2.05f, 1.25f, 0.f}, {0.1f, 2.5f, 4.2f}, {0.45f, 0.75f, 0.50f});  // mur droit
                boite({0.f, 1.0f, -1.97f}, {0.9f, 2.0f, 0.06f}, {0.45f, 0.28f, 0.12f}); // porte
                boite({0.f, 2.45f, 0.f}, {0.6f, 0.1f, 0.6f}, {1.0f, 1.0f, 1.0f});       // lampe
                boite({0.f, 1.6f, -1.92f}, {4.0f, 0.06f, 0.04f}, {0.04f, 0.04f, 0.08f}); // bandes sombres
                boite({0.f, 1.6f, 1.92f}, {4.0f, 0.06f, 0.04f}, {0.04f, 0.04f, 0.08f});
                boite({-1.92f, 1.6f, 0.f}, {0.04f, 0.06f, 4.0f}, {0.04f, 0.04f, 0.08f});
                boite({1.92f, 1.6f, 0.f}, {0.04f, 0.06f, 4.0f}, {0.04f, 0.04f, 0.08f});
        }
}

int nkmain(const NkEntryState &state) {
        (void)state;

        // ── 0) Les trois budgets, avant d'ouvrir la fenetre ────────────────────────
        const double cadences[3] = {72.0, 90.0, 120.0};
        for (int i = 0; i < 3; ++i)
                std::printf("budget %d Hz : %.3f ms\n", (int)cadences[i], BudgetMs(cadences[i]));
        std::fflush(stdout);

        int imagesMax = 0;
        if (const char *e = std::getenv("MASALLE_EXIT"); e && e[0]) imagesMax = std::atoi(e);
        const char *mb = std::getenv("NK_VK_MAILBOX");
        const char *vs = std::getenv("MASALLE_VSYNC");
        const bool vsyncCoupee = (vs && vs[0] == '0');
        const bool sansAttente = (mb && mb[0] == '1') || vsyncCoupee;

        // ── 1) Fenetre, device, renderer ───────────────────────────────────────────
        NkWindowConfig winCfg;
        winCfg.title = "Votre budget calcule (Echap = quitter)";
        winCfg.width = 1280;
        winCfg.height = 720;
        winCfg.centered = true;
        winCfg.resizable = false;

        NkWindow window(winCfg);
        if (!window.IsValid()) {
                logger.Error("[Budget] Creation fenetre KO");
                return 1;
        }

        NkDeviceInitInfo devInfo{};
        devInfo.surface = window.GetSurfaceDesc();
        devInfo.width = (uint32)window.GetSize().width;
        devInfo.height = (uint32)window.GetSize().height;

        NkGraphicsApi apiChoisie = NkGraphicsApi::NK_GFX_API_OPENGL;
        const char *nomApi = "opengl";
        if (const char *a = std::getenv("MASALLE_API")) {
                if (!std::strcmp(a, "vulkan")) { apiChoisie = NkGraphicsApi::NK_GFX_API_VULKAN; nomApi = "vulkan"; }
                else if (!std::strcmp(a, "dx11")) { apiChoisie = NkGraphicsApi::NK_GFX_API_DX11; nomApi = "dx11"; }
                else if (!std::strcmp(a, "dx12")) { apiChoisie = NkGraphicsApi::NK_GFX_API_DX12; nomApi = "dx12"; }
                else if (!std::strcmp(a, "sw")) { apiChoisie = NkGraphicsApi::NK_GFX_API_SOFTWARE; nomApi = "sw"; }
        }
        devInfo.api = apiChoisie;
        NkIDevice *device = NkDeviceFactory::CreateForApi(apiChoisie, devInfo);
        if (!device || !device->IsValid()) {
                logger.Error("[Budget] Creation device KO");
                window.Close();
                return 2;
        }

        NkRendererConfig cfg = NkRendererConfig::ForGame(devInfo.api, devInfo.width, devInfo.height);
        if (vsyncCoupee) cfg.vsync = false;
        NkRenderer *renderer = NkRenderer::Create(device, cfg);
        if (!renderer || !renderer->Initialize()) {
                logger.Error("[Budget] Init renderer KO");
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

        // ── 2) Boucle : l'instant de debut de chaque image est note ───────────────
        std::vector<double> debuts; // en nanosecondes
        debuts.reserve(imagesMax > 0 ? (size_t)imagesMax + 4 : 4096);
        int frame = 0;

        while (running && window.IsOpen()) {
                events.PollEvents();
                if (!running) break;
                debuts.push_back((double)NkChrono::Now().nanoseconds); // DEBUT de l'image
                ++frame;

                if (renderer->BeginFrame()) {
                        // La tete tourne tres lentement : une image qui change, sans souris.
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

        // ── 3) Les intervalles : entre deux debuts d'image successifs ─────────────
        std::fprintf(stderr, "api demandee : %s ; images : %d ; sans attente : %s\n", nomApi, frame,
                     sansAttente ? "oui" : "non");
        std::fprintf(stderr, "synchro verticale coupee : %s\n", vsyncCoupee ? "oui" : "non");
        const int besoin = kEcartees + kIntervalles + 1; // 1031 debuts donnent 1000 intervalles
        if ((int)debuts.size() < besoin) {
                std::fprintf(stderr, "pas assez d'images : %d debuts, il en faut %d (lancez avec MASALLE_EXIT=1040)\n",
                             (int)debuts.size(), besoin);
                return 4;
        }
        std::vector<double> ms;
        ms.reserve(kIntervalles);
        for (int k = kEcartees; k < kEcartees + kIntervalles; ++k)
                ms.push_back((debuts[k + 1] - debuts[k]) * 1e-6);

        std::fprintf(stderr, "premiers intervalles (ms) : %.3f %.3f %.3f ; intervalles gardes : %d\n",
                     (debuts[1] - debuts[0]) * 1e-6, (debuts[2] - debuts[1]) * 1e-6,
                     (debuts[3] - debuts[2]) * 1e-6, (int)ms.size());

        double somme = 0.0, maxi = 0.0;
        for (double v : ms) {
                somme += v;
                if (v > maxi) maxi = v;
        }
        const double moyenne = somme / (double)ms.size();
        std::vector<double> tri = ms;
        std::sort(tri.begin(), tri.end());
        const size_t n = tri.size();
        const double mediane = (n % 2 == 1) ? tri[n / 2] : 0.5 * (tri[n / 2 - 1] + tri[n / 2]);

        { double mn = tri[0]; int s72 = 0, s90 = 0, s120 = 0; for (double v : ms) { if (v > 13.889) ++s72; if (v > 11.111) ++s90; if (v > 8.333) ++s120; } std::fprintf(stderr, "min %.3f ms ; images au-dela de 13.889 ms : %d, de 11.111 ms : %d, de 8.333 ms : %d (sur %d)\n", mn, s72, s90, s120, (int)ms.size()); }
        std::printf("%s : moyenne %.3f ms, mediane %.3f ms, max %.3f ms\n", sansAttente ? "sans attente" : "ecran",
                    moyenne, mediane, maxi);
        std::fflush(stdout);
        return 0;
}
