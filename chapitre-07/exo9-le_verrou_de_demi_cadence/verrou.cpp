// Le verrou de demi-cadence : la salle est alourdie (K copies decalees de 5 m le long de x), puis on mesure 1000 intervalles
// entre deux DEBUTS d'image, sans verrou ou avec verrou (MASALLE_VERROU=0 ou 1).
//   Verrou : a la fin de chaque image (apres EndFrame), on attend que 2 periodes d'ecran se soient ecoulees depuis le DEBUT de
//   cette image. Si le travail a deja depasse ce delai, on n'attend pas (et on ne rattrape rien : l'image suivante commence tard).
//   Precision de l'attente : NkChrono::SleepMilliseconds ne descend pas sous la milliseconde et, sous Windows, elle peut
//   dormir plus que demande (granularite de l'ordonnanceur). On dort donc par pas de 1 ms tant qu'il reste plus de
//   kMargeSpinMs, puis on boucle en attente active sur l'horloge pour les derniers instants : l'erreur de sortie est mesuree
//   et imprimee (diagnostic sur la sortie d'erreur).
//   Sans NkChrono::BeginPreciseTiming() (appelee ici, appariee a EndPreciseTiming), SleepMilliseconds dort ~15,5 ms quoi qu on
//   demande (commentaire de NkChrono.h) : le verrou serait alors quantifie, pas cadence. Marge d attente active : 3 ms.
//   Periode de l'ecran : 1000 / 60 ms (60 Hz lus dans les parametres d'affichage de Windows), modifiable par MASALLE_PERIODE_MS.
//   Les 30 premieres images sont ecartees. Ecart type = racine de la moyenne des carres des ecarts a la moyenne.
// Variables : MASALLE_COPIES=K (defaut 1), MASALLE_VERROU=0|1 (defaut 0), MASALLE_N=nombre d'intervalles (defaut 1000).
// Sortie standard : "<sans verrou|avec verrou> : moyenne M ms, ecart type E ms, max X ms"
// Sortie d'erreur : copies, periode, diagnostics. Echap quitte (mesure abandonnee).

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
        d.appName = "LeVerrouDeDemiCadence";
}
NK_REGISTER_ENTRY_APPDATA_UPDATER(ConfigureAppData)

namespace {
        const int kEcartees = 30;         // images ecartees au depart
        const double kMargeSpinMs = 3.0;  // sous cette marge, on n'appelle plus Sleep : attente active

        double NowMs() { return (double)NkChrono::Now().nanoseconds * 1e-6; }

        // Une copie de la salle (13 boites), decalee de dx metres le long de x.
        void SubmitSalle(NkRender3D *r3d, NkMeshHandle cube, float dx) {
                auto boite = [&](NkVec3f c, NkVec3f s, NkVec3f tint) {
                        c.x += dx;
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

        int copies = 1;
        if (const char *v = std::getenv("MASALLE_COPIES")) copies = std::max(1, std::atoi(v));
        bool verrou = false;
        if (const char *v = std::getenv("MASALLE_VERROU")) verrou = (v[0] == '1');
        int nVoulu = 1000;
        if (const char *v = std::getenv("MASALLE_N")) nVoulu = std::max(10, std::atoi(v));
        double periodeMs = 1000.0 / 60.0;
        if (const char *v = std::getenv("MASALLE_PERIODE_MS")) periodeMs = std::atof(v);
        const double cibleMs = 2.0 * periodeMs; // deux periodes d'ecran
        const bool minuterieAvant = NkChrono::IsPreciseTimingActive(); // deja active avant notre appel ?
        NkChrono::BeginPreciseTiming(); // sans elle, SleepMilliseconds dort ~15,5 ms quoi qu on demande (NkChrono.h)

        NkWindowConfig winCfg;
        winCfg.title = "Le verrou de demi-cadence (ne touchez a rien ; Echap = quitter)";
        winCfg.width = 1280;
        winCfg.height = 720;
        winCfg.centered = true;
        winCfg.resizable = false;

        NkWindow window(winCfg);
        if (!window.IsValid()) {
                logger.Error("[Verrou] Creation fenetre KO");
                return 1;
        }

        NkDeviceInitInfo devInfo{};
        devInfo.surface = window.GetSurfaceDesc();
        devInfo.width = (uint32)window.GetSize().width;
        devInfo.height = (uint32)window.GetSize().height;
        devInfo.api = NkGraphicsApi::NK_GFX_API_OPENGL;
        NkIDevice *device = NkDeviceFactory::CreateForApi(devInfo.api, devInfo);
        if (!device || !device->IsValid()) {
                logger.Error("[Verrou] Creation device KO");
                window.Close();
                return 2;
        }

        NkRendererConfig cfg = NkRendererConfig::ForGame(devInfo.api, devInfo.width, devInfo.height);
        NkRenderer *renderer = NkRenderer::Create(device, cfg);
        if (!renderer || !renderer->Initialize()) {
                logger.Error("[Verrou] Init renderer KO");
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

        std::vector<double> dts;        // intervalles retenus entre deux debuts d'image (ms)
        std::vector<double> travail;    // duree du travail (debut -> fin d'EndFrame), image par image (ms)
        std::vector<double> erreurSortie; // avec verrou : instant de sortie de l'attente moins la cible (ms, >= 0)
        double debutPrec = 0.0;
        double travailPrec = 0.0, erreurPrec = 0.0;
        int frame = 0, depassements = 0, appelsSleep = 0;
        bool fini = false;

        while (running && window.IsOpen() && !fini) {
                const double t0 = NowMs(); // debut de cette image
                ++frame;
                if (frame > 1 && frame - 1 > kEcartees) {
                        dts.push_back(t0 - debutPrec); // intervalle entre le debut de l'image precedente et celui-ci
                        travail.push_back(travailPrec);
                        if (verrou) erreurSortie.push_back(erreurPrec);
                        if ((int)dts.size() >= nVoulu) {
                                fini = true;
                                break;
                        }
                }
                debutPrec = t0;

                events.PollEvents();
                if (!running) break;

                if (renderer->BeginFrame()) {
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
                        for (int k = 0; k < copies; ++k) SubmitSalle(r3d, meshCube, 5.0f * (float)k);
                        renderer->Present();
                        renderer->EndFrame();
                }
                travailPrec = NowMs() - t0;

                // ── Le verrou : attendre jusqu'a (debut de l'image + 2 periodes) ──────────
                erreurPrec = 0.0;
                if (verrou) {
                        const double cible = t0 + cibleMs;
                        if (NowMs() > cible) {
                                ++depassements; // travail plus long que la cible : rien a attendre
                        } else {
                                while (cible - NowMs() > kMargeSpinMs) { // gros de l'attente : dormir 1 ms a la fois
                                        NkChrono::SleepMilliseconds(1);
                                        ++appelsSleep;
                                }
                                while (NowMs() < cible) { /* attente active sur les derniers instants */ }
                                erreurPrec = NowMs() - cible;
                        }
                }
        }

        NkChrono::EndPreciseTiming();
        NkRenderer::Destroy(renderer);
        NkDeviceFactory::Destroy(device);
        window.Close();

        if (!fini || dts.empty()) {
                std::fprintf(stderr, "mesure abandonnee : %d intervalles retenus sur %d\n", (int)dts.size(), nVoulu);
                return 4;
        }
        const int n = (int)dts.size();
        double somme = 0.0, mx = 0.0;
        for (double d : dts) {
                somme += d;
                if (d > mx) mx = d;
        }
        const double moy = somme / n;
        double carres = 0.0;
        for (double d : dts) carres += (d - moy) * (d - moy);
        const double ecart = std::sqrt(carres / n);
        std::printf("%s : moyenne %.3f ms, ecart type %.3f ms, max %.3f ms\n", verrou ? "avec verrou" : "sans verrou", moy, ecart, mx);
        std::fflush(stdout);

        // Diagnostics (sortie d'erreur)
        std::fprintf(stderr, "minuterie fine deja active avant BeginPreciseTiming : %s\n", minuterieAvant ? "oui" : "non");
        std::fprintf(stderr, "copies de la salle : %d ; periode ecran %.4f ms ; cible du verrou %.4f ms ; %d intervalles\n", copies,
                     periodeMs, cibleMs, n);
        std::vector<double> tri = dts;
        std::sort(tri.begin(), tri.end());
        std::fprintf(stderr, "min %.3f ms ; mediane %.3f ms ; p95 %.3f ms ; p99 %.3f ms ; max %.3f ms\n", tri.front(), tri[n / 2],
                     tri[(size_t)(0.95 * (n - 1))], tri[(size_t)(0.99 * (n - 1))], tri.back());
        double sTrav = 0.0, mxTrav = 0.0;
        int auDessusPeriode = 0, auDessusCible = 0;
        for (double d : travail) {
                sTrav += d;
                if (d > mxTrav) mxTrav = d;
                if (d > periodeMs) ++auDessusPeriode;
                if (d > cibleMs) ++auDessusCible;
        }
        std::fprintf(stderr, "travail (debut -> fin d'image) : moyenne %.3f ms, max %.3f ms ; > 1 periode : %d ; > 2 periodes : %d\n",
                     sTrav / n, mxTrav, auDessusPeriode, auDessusCible);
        int hors = 0;
        for (double d : dts) if (d > cibleMs + 1.0) ++hors;
        std::fprintf(stderr, "intervalles > cible + 1 ms : %d\n", hors);
        if (verrou) {
                double sErr = 0.0, mxErr = 0.0;
                for (double e : erreurSortie) { sErr += e; if (e > mxErr) mxErr = e; }
                std::fprintf(stderr, "verrou : images sans attente (travail deja > cible) %d ; appels a Sleep %d ; erreur de sortie moyenne %.4f ms, max %.4f ms\n",
                             depassements, appelsSleep, sErr / n, mxErr);
        }
        {
                const int kBins = 21;
                std::vector<int> h(kBins, 0);
                for (double d : dts) ++h[std::min(kBins - 1, (int)(d / 5.0))];
                std::fprintf(stderr, "histogramme (tranches de 5 ms) :");
                for (int b = 0; b < kBins; ++b)
                        if (h[b] > 0) std::fprintf(stderr, " [%d-%d)%d", b * 5, (b + 1) * 5, h[b]);
                std::fprintf(stderr, "\n");
        }
        return 0;
}
