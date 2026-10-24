// Mesurer une vitesse : la vitesse angulaire de la tete entre deux poses horodatees.
//   orientation = RotationY(lacet) * RotationX(tangage) (deux matrices), puis conversion en
//   quaternion normalise. La vitesse se calcule comme dans le module de realite virtuelle du moteur.
//
// Deux series de vingt vitesses successives (une ligne par image) :
//   serie programme : MASALLE_TOUR=<deg/s>, le lacet suit l'HORLOGE (lacet = vitesse * t), pas la souris.
//                     Les images 61 a 80 sont relevees, puis la salle s'arrete.
//   serie souris    : la souris est lue UNE fois par image (comptes accumules puis vides) ;
//                     lacet -= total_x * 0,0022 rad, tangage -= total_y * 0,0022 rad.
//                     Appuyer sur F1 en bougeant : les 20 images suivantes sont relevees.
//
// Sortie standard : la ligne "serie programme" ou "serie souris", puis vingt lignes
//   "image N : dt = D ms, vitesse = (x, y, z) deg/s".
// Sortie d'erreur : statistiques de la norme, dt min/max, comptes souris par image.
// Echap quitte. MASALLE_API=<opengl|vulkan|dx11|dx12|sw> choisit l'API (defaut opengl).

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
#include "NKEvent/NkMouseEvent.h"
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
        d.appName = "MesurerUneVitesse";
}
NK_REGISTER_ENTRY_APPDATA_UPDATER(ConfigureAppData)

namespace {
        const float kPi = 3.14159265358979f;
        const float kSensibilite = 0.0022f; // radian par compte du capteur
        const float kRadVersDeg = 180.0f / kPi;
        const int kReleves = 20;

        float gDeltaX = 0.0f; // accumulateurs des mouvements bruts
        float gDeltaY = 0.0f;

        // Vitesse angulaire entre deux poses horodatees (module de realite virtuelle du moteur).
        // Direction = axe de la rotation, longueur = vitesse en radians par seconde.
        NkVec3f VitesseAngulaire(const NkQuatf &from, const NkQuatf &to, float32 dtSeconds) {
                if (dtSeconds <= 0.f) {
                        return NkVec3f(0.f, 0.f, 0.f);
                }
                NkQuatf delta = (to * from.Conjugate()).Normalized();
                // Les deux hemispheres du quaternion codent la meme rotation ; on force le chemin court.
                if (delta.w < 0.f) {
                        delta = NkQuatf(-delta.x, -delta.y, -delta.z, -delta.w);
                }
                const float32 angleRad = delta.Angle().Rad();
                if (angleRad <= 1e-6f) {
                        return NkVec3f(0.f, 0.f, 0.f);
                }
                return delta.Axis() * (angleRad / dtSeconds);
        }

        float Propre(float v) { return std::fabs(v) < 5e-4f ? 0.0f : v; } // evite "-0.000"

        struct Releve {
                int image;
                double dtMs;
                NkVec3f vitesseDeg; // en degres par seconde
                int comptesX;
                int comptesY;
        };

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
                boite({0.9f, 1.0f, -1.2f}, {0.4f, 0.4f, 0.4f}, {1.0f, 0.85f, 0.2f}); // un repere pour voir tourner
        }
}

int nkmain(const NkEntryState &state) {
        (void)state;

        // ── Le tour programme : MASALLE_TOUR = vitesse en degres par seconde ───────
        bool programme = false;
        double tourDegS = 0.0;
        if (const char *e = std::getenv("MASALLE_TOUR"); e && e[0]) {
                programme = true;
                tourDegS = std::atof(e);
        }

        NkWindowConfig winCfg;
        winCfg.title = "Mesurer une vitesse (F1 = relever, Echap = quitter)";
        winCfg.width = 1280;
        winCfg.height = 720;
        winCfg.centered = true;
        winCfg.resizable = false;

        NkWindow window(winCfg);
        if (!window.IsValid()) {
                logger.Error("[Vitesse] Creation fenetre KO");
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
                logger.Error("[Vitesse] Creation device KO");
                window.Close();
                return 2;
        }

        NkRendererConfig cfg = NkRendererConfig::ForGame(devInfo.api, devInfo.width, devInfo.height);
        NkRenderer *renderer = NkRenderer::Create(device, cfg);
        if (!renderer || !renderer->Initialize()) {
                logger.Error("[Vitesse] Init renderer KO");
                NkRenderer::Destroy(renderer);
                NkDeviceFactory::Destroy(device);
                window.Close();
                return 3;
        }
        NkRender3D *r3d = renderer->GetRender3D();
        NkMeshHandle meshCube = renderer->GetMeshSystem()->GetCube();
        const float aspect = (float)devInfo.width / (float)devInfo.height;

        bool running = true;
        bool demande = false; // F1 : relever les 20 images suivantes (serie souris)
        NkEventSystem &events = NkEvents();
        events.AddEventCallback<NkWindowCloseEvent>([&](NkWindowCloseEvent *) { running = false; });
        events.AddEventCallback<NkKeyPressEvent>([&](NkKeyPressEvent *e) {
                if (e->GetKey() == NkKey::NK_ESCAPE) running = false;
                else if (e->GetKey() == NkKey::NK_F1) demande = true;
        });
        events.AddEventCallback<NkMouseRawEvent>([&](NkMouseRawEvent *e) {
                if (!programme) { // accumuler ...
                        gDeltaX += (float)e->GetDeltaX();
                        gDeltaY += (float)e->GetDeltaY();
                }
        });

        if (!programme) { // souris capturee, rendue a la fermeture
                window.ShowMouse(false);
                window.CaptureMouse(true);
                window.ClipMouseToClient(true);
        }

        // ── Boucle ────────────────────────────────────────────────────────────────
        std::vector<Releve> releves;
        double ns0 = 0.0, tPrev = 0.0;
        NkQuatf qPrev = NkQuatf::Identity();
        float lacet = 0.0f, tangage = 0.0f; // radians
        int frame = 0;
        bool enregistre = false;

        while (running && window.IsOpen() && (int)releves.size() < kReleves) {
                events.PollEvents();
                if (!running) break;

                const double ns = (double)NkChrono::Now().nanoseconds;
                if (frame == 0) ns0 = ns;
                const double t = (ns - ns0) * 1e-9; // secondes depuis la premiere image
                ++frame;

                // ... lire UNE fois par image, puis vider.
                const float dx = gDeltaX, dy = gDeltaY;
                gDeltaX = 0.0f;
                gDeltaY = 0.0f;

                if (programme) {
                        lacet = (float)(tourDegS * t * (double)kPi / 180.0); // le lacet suit l'horloge
                        tangage = 0.0f;
                        if (frame >= 61) enregistre = true;                    // images 61 a 80
                } else {
                        lacet -= dx * kSensibilite;
                        tangage -= dy * kSensibilite;
                        if (demande) enregistre = true; // F1 : les images suivantes
                }

                // Orientation : deux matrices puis une conversion.
                const NkMat4f rotation = NkMat4f::RotationY(NkAngle::FromRad(lacet)) *
                                         NkMat4f::RotationX(NkAngle::FromRad(tangage));
                const NkQuatf orientation = NkQuatf(rotation).Normalized();

                if (frame > 1 && enregistre && (int)releves.size() < kReleves) {
                        const float dtS = (float)(t - tPrev);
                        const NkVec3f v = VitesseAngulaire(qPrev, orientation, dtS);
                        Releve r;
                        r.image = frame;
                        r.dtMs = (t - tPrev) * 1e3;
                        r.vitesseDeg = NkVec3f(v.x * kRadVersDeg, v.y * kRadVersDeg, v.z * kRadVersDeg);
                        r.comptesX = (int)std::lround(dx);
                        r.comptesY = (int)std::lround(dy);
                        releves.push_back(r);
                }
                qPrev = orientation;
                tPrev = t;

                if (renderer->BeginFrame()) {
                        const NkVec3f oeil(0.0f, 1.6f, 0.0f);
                        NkCamera3DData camData;
                        camData.position = oeil;
                        camData.target = oeil + orientation * NkVec3f(0.0f, 0.0f, -1.0f);
                        camData.up = orientation * NkVec3f(0.0f, 1.0f, 0.0f);
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
        }

        if (!programme) { // rendre la souris AVANT de quitter
                window.ClipMouseToClient(false);
                window.CaptureMouse(false);
                window.ShowMouse(true);
        }
        NkRenderer::Destroy(renderer);
        NkDeviceFactory::Destroy(device);
        window.Close();

        // ── Sorties ───────────────────────────────────────────────────────────────
        if ((int)releves.size() < kReleves) {
                std::fprintf(stderr, "serie incomplete : %d releves sur %d (F1 n'a pas ete presse ?)\n",
                             (int)releves.size(), kReleves);
                return 4;
        }
        std::printf("%s\n", programme ? "serie programme" : "serie souris");
        for (const Releve &r : releves)
                std::printf("image %d : dt = %.3f ms, vitesse = (%.3f, %.3f, %.3f) deg/s\n", r.image, r.dtMs,
                            Propre(r.vitesseDeg.x), Propre(r.vitesseDeg.y), Propre(r.vitesseDeg.z));
        std::fflush(stdout);

        double somme = 0.0, somme2 = 0.0, nMin = 1e300, nMax = -1e300, dtMin = 1e300, dtMax = -1e300;
        for (const Releve &r : releves) {
                const double n = std::sqrt((double)r.vitesseDeg.x * r.vitesseDeg.x +
                                           (double)r.vitesseDeg.y * r.vitesseDeg.y +
                                           (double)r.vitesseDeg.z * r.vitesseDeg.z);
                somme += n;
                somme2 += n * n;
                if (n < nMin) nMin = n;
                if (n > nMax) nMax = n;
                if (r.dtMs < dtMin) dtMin = r.dtMs;
                if (r.dtMs > dtMax) dtMax = r.dtMs;
        }
        const double moy = somme / releves.size();
        const double ecart = std::sqrt(std::fmax(0.0, somme2 / releves.size() - moy * moy));
        std::fprintf(stderr, "norme (deg/s) : moyenne %.3f, ecart-type %.3f, min %.3f, max %.3f ; dt min %.3f ms, max %.3f ms\n",
                     moy, ecart, nMin, nMax, dtMin, dtMax);
        std::fprintf(stderr, "comptes souris par image (x) :");
        for (const Releve &r : releves) std::fprintf(stderr, " %d", r.comptesX);
        std::fprintf(stderr, "\ncomptes souris par image (y) :");
        for (const Releve &r : releves) std::fprintf(stderr, " %d", r.comptesY);
        std::fprintf(stderr, "\n");
        return 0;
}
