// La stabilite mesuree : le temps de chaque image de la salle pendant UNE MINUTE, et ce qu'il dit de la stabilite.
//   Le temps d'une image est l'intervalle entre deux DEBUTS d'image (NkChrono::Now().nanoseconds, horloge qui ne recule jamais).
//   Les 30 premieres images paient le demarrage et sont ecartees : la mesure part du debut de l'image 31 et ne compte que
//   les intervalles termines au plus tard 60 s plus tard (la duree couverte reelle est imprimee sur la sortie d'erreur).
//   Ecart type = racine de la moyenne des carres des ecarts a la moyenne. Budget = un quatre-vingt-dixieme de seconde.
//   MASALLE_VSYNC=0 : cfg.vsync = false, et la ligne s'appelle "sans attente" ; sinon "ecran".
//   (NK_VK_MAILBOX ne vaut que sous Vulkan : le moteur n'est pas compile avec Vulkan ici, on mesure sous OpenGL.)
// Sortie standard (une ligne) :
//   <ecran|sans attente> : N images, moyenne M ms, ecart type E ms, max X ms, hors budget H
// Sortie d'erreur : diagnostics (duree couverte, percentiles, plus longs intervalles, histogramme, temps par phase).
// MASALLE_API=<opengl|vulkan|dx11|dx12|sw> choisit l'API (defaut opengl). Echap quitte (mesure abandonnee).

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
        d.appName = "LaStabiliteMesuree";
}
NK_REGISTER_ENTRY_APPDATA_UPDATER(ConfigureAppData)

namespace {
        const int kEcartees = 30;            // images ecartees au depart
        const double kDureeMs = 60000.0;     // une minute exactement
        const double kBudgetMs = 1000.0 / 90.0; // budget de 90 hertz

        double NowMs() { return (double)NkChrono::Now().nanoseconds * 1e-6; }

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
                boite({0.9f, 1.0f, -1.2f}, {0.4f, 0.4f, 0.4f}, {1.0f, 0.85f, 0.2f});
        }
}

int nkmain(const NkEntryState &state) {
        (void)state;

        bool sansAttente = false;
        if (const char *v = std::getenv("MASALLE_VSYNC")) sansAttente = (v[0] == '0');
        const char *mailbox = std::getenv("NK_VK_MAILBOX");

        NkWindowConfig winCfg;
        winCfg.title = "La stabilite mesuree (une minute, ne touchez a rien ; Echap = quitter)";
        winCfg.width = 1280;
        winCfg.height = 720;
        winCfg.centered = true;
        winCfg.resizable = false;

        NkWindow window(winCfg);
        if (!window.IsValid()) {
                logger.Error("[Stabilite] Creation fenetre KO");
                return 1;
        }

        NkDeviceInitInfo devInfo{};
        devInfo.surface = window.GetSurfaceDesc();
        devInfo.width = (uint32)window.GetSize().width;
        devInfo.height = (uint32)window.GetSize().height;

        NkGraphicsApi apiChoisie = NkGraphicsApi::NK_GFX_API_OPENGL;
        const char *apiNom = "opengl";
        if (const char *a = std::getenv("MASALLE_API")) {
                if (!std::strcmp(a, "vulkan")) { apiChoisie = NkGraphicsApi::NK_GFX_API_VULKAN; apiNom = "vulkan"; }
                else if (!std::strcmp(a, "dx11")) { apiChoisie = NkGraphicsApi::NK_GFX_API_DX11; apiNom = "dx11"; }
                else if (!std::strcmp(a, "dx12")) { apiChoisie = NkGraphicsApi::NK_GFX_API_DX12; apiNom = "dx12"; }
                else if (!std::strcmp(a, "sw")) { apiChoisie = NkGraphicsApi::NK_GFX_API_SOFTWARE; apiNom = "sw"; }
        }
        devInfo.api = apiChoisie;
        NkIDevice *device = NkDeviceFactory::CreateForApi(apiChoisie, devInfo);
        if (!device || !device->IsValid()) {
                logger.Error("[Stabilite] Creation device KO");
                window.Close();
                return 2;
        }

        NkRendererConfig cfg = NkRendererConfig::ForGame(devInfo.api, devInfo.width, devInfo.height);
        if (sansAttente) cfg.vsync = false;
        NkRenderer *renderer = NkRenderer::Create(device, cfg);
        if (!renderer || !renderer->Initialize()) {
                logger.Error("[Stabilite] Init renderer KO");
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

        // ── Boucle : une minute apres 30 images ecartees ──────────────────────────
        std::vector<double> dts, fins; // intervalles retenus (ms), et leur fin (ms depuis le debut de la mesure)
        std::vector<double> ecartes;   // intervalles des images ecartees (ms), pour memoire
        double phaseSomme[5] = {0, 0, 0, 0, 0}, phasePrec[5] = {0, 0, 0, 0, 0};
        double debutPrec = 0.0, tMesure0 = 0.0;
        int frame = 0;
        bool fini = false;

        while (running && window.IsOpen() && !fini) {
                const double t0 = NowMs(); // debut de cette image
                ++frame;
                if (frame > 1) {
                        const double dt = t0 - debutPrec; // duree de l'image precedente
                        if (frame - 1 < kEcartees + 1) {
                                ecartes.push_back(dt); // images 1 a 30
                        } else {
                                // l'image precedente est l'image 31 ou une suivante : elle fait partie de la minute
                                if (frame - 1 == kEcartees + 1) tMesure0 = debutPrec;
                                if (t0 - tMesure0 > kDureeMs) {
                                        fini = true; // cet intervalle depasserait la minute : on s'arrete sans le compter
                                        break;
                                }
                                dts.push_back(dt);
                                fins.push_back(t0 - tMesure0);
                                for (int p = 0; p < 5; ++p) phaseSomme[p] += phasePrec[p];
                        }
                }
                debutPrec = t0;

                events.PollEvents();
                if (!running) break;
                const double t1 = NowMs();
                double t2 = t1, t3 = t1, t4 = t1, t5 = t1;

                if (renderer->BeginFrame()) {
                        t2 = NowMs();
                        const NkVec3f oeil(0.0f, 1.6f, 0.0f);
                        NkCamera3DData camData;
                        camData.position = oeil;
                        camData.target = oeil + NkVec3f(0.0f, 0.0f, -1.0f);
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
                        t3 = NowMs();
                        renderer->Present();
                        t4 = NowMs();
                        renderer->EndFrame();
                        t5 = NowMs();
                }
                phasePrec[0] = t1 - t0; // evenements
                phasePrec[1] = t2 - t1; // BeginFrame
                phasePrec[2] = t3 - t2; // scene : BeginScene + soumission
                phasePrec[3] = t4 - t3; // Present
                phasePrec[4] = t5 - t4; // EndFrame
        }

        NkRenderer::Destroy(renderer);
        NkDeviceFactory::Destroy(device);
        window.Close();

        // ── Sorties ───────────────────────────────────────────────────────────────
        if (!fini || dts.empty()) {
                std::fprintf(stderr, "mesure abandonnee : %d intervalles retenus, minute non terminee (Echap ou fermeture)\n", (int)dts.size());
                return 4;
        }
        const int n = (int)dts.size();
        double somme = 0.0, mx = 0.0;
        int horsBudget = 0;
        for (double d : dts) {
                somme += d;
                if (d > mx) mx = d;
                if (d > kBudgetMs) ++horsBudget;
        }
        const double moy = somme / n;
        double carres = 0.0;
        for (double d : dts) carres += (d - moy) * (d - moy);
        const double ecart = std::sqrt(carres / n);
        std::printf("%s : %d images, moyenne %.3f ms, ecart type %.3f ms, max %.3f ms, hors budget %d\n",
                    sansAttente ? "sans attente" : "ecran", n, moy, ecart, mx, horsBudget);
        std::fflush(stdout);

        // Diagnostics
        std::fprintf(stderr, "api demandee : %s ; vsync %s ; NK_VK_MAILBOX %s (sans effet hors Vulkan)\n", apiNom,
                     sansAttente ? "false" : "defaut", mailbox ? mailbox : "(absente)");
        std::fprintf(stderr, "duree couverte : %.3f s sur %d intervalles ; budget 90 Hz = %.3f ms\n", somme * 1e-3, n, kBudgetMs);
        {
                double mxEcarte = 0.0, premier = ecartes.empty() ? 0.0 : ecartes[0];
                for (double d : ecartes) if (d > mxEcarte) mxEcarte = d;
                std::fprintf(stderr, "images ecartees : %d ; premiere image %.3f ms ; plus longue des %d ecartees %.3f ms\n",
                             (int)ecartes.size(), premier, (int)ecartes.size(), mxEcarte);
        }
        std::vector<double> tri = dts;
        std::sort(tri.begin(), tri.end());
        std::fprintf(stderr, "min %.3f ms ; mediane %.3f ms ; p95 %.3f ms ; p99 %.3f ms ; max %.3f ms\n", tri.front(), tri[n / 2],
                     tri[(size_t)(0.95 * (n - 1))], tri[(size_t)(0.99 * (n - 1))], tri.back());
        int plus100 = 0, plus1000 = 0;
        for (double d : dts) {
                if (d > 100.0) ++plus100;
                if (d > 1000.0) ++plus1000;
        }
        std::fprintf(stderr, "intervalles > 100 ms : %d ; > 1000 ms : %d\n", plus100, plus1000);
        {
                std::vector<int> idx(n);
                for (int i = 0; i < n; ++i) idx[i] = i;
                std::partial_sort(idx.begin(), idx.begin() + std::min(5, n), idx.end(), [&](int a, int b) { return dts[a] > dts[b]; });
                std::fprintf(stderr, "les 5 plus longs :");
                for (int i = 0; i < std::min(5, n); ++i)
                        std::fprintf(stderr, " %.1f ms (a %.2f s)%s", dts[idx[i]], fins[idx[i]] * 1e-3, i + 1 < std::min(5, n) ? "," : "");
                std::fprintf(stderr, "\n");
        }
        {
                const int kBins = 21; // 0-5, 5-10, ..., 95-100, puis >= 100
                std::vector<int> h(kBins, 0);
                for (double d : dts) ++h[std::min(kBins - 1, (int)(d / 5.0))];
                std::fprintf(stderr, "histogramme (tranches de 5 ms) :");
                for (int b = 0; b < kBins; ++b)
                        if (h[b] > 0) std::fprintf(stderr, " [%d-%s)%d", b * 5, b == kBins - 1 ? "+" : std::to_string((b + 1) * 5).c_str(), h[b]);
                std::fprintf(stderr, "\n");
        }
        std::fprintf(stderr, "temps moyen par phase (ms) : evenements %.3f, BeginFrame %.3f, scene %.3f, Present %.3f, EndFrame %.3f ; somme %.3f\n",
                     phaseSomme[0] / n, phaseSomme[1] / n, phaseSomme[2] / n, phaseSomme[3] / n, phaseSomme[4] / n,
                     (phaseSomme[0] + phaseSomme[1] + phaseSomme[2] + phaseSomme[3] + phaseSomme[4]) / n);
        return 0;
}
