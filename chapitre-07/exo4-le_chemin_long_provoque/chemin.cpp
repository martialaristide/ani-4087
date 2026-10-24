// Le chemin long provoque : on retire de la vitesse angulaire le test qui force le chemin court,
// on fait tourner la tete lentement (un tour complet) et on cherche l'image ou la vitesse est fausse.
//   MASALLE_TOUR=<deg/s> : le lacet suit l'HORLOGE (lacet = vitesse * t), pas la souris. Defaut : 20.
//   orientation = RotationY(lacet) * RotationX(tangage) (deux matrices), puis conversion en quaternion normalise.
//   La salle tourne jusqu'a ce que le lacet atteigne 360 degres.
// Sortie standard : une ligne par image,
//   image N : lacet = L deg, vitesse = (x, y, z) deg/s, delta.w = W
// Sortie d'erreur : la plus grande vitesse, l'image d'avant, le nombre d'images a delta.w < 0.
// delta.w est lu dans VitesseAngulaire, juste apres la normalisation (avant tout test eventuel).
// Echap quitte. MASALLE_API=<opengl|vulkan|dx11|dx12|sw> choisit l'API (defaut opengl).

#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
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
        d.appName = "LeCheminLong";
}
NK_REGISTER_ENTRY_APPDATA_UPDATER(ConfigureAppData)

namespace {
        const float kPi = 3.14159265358979f;
        const float kRadVersDeg = 180.0f / kPi;

        float gDeltaW = 1.0f; // w du quaternion delta, lu dans VitesseAngulaire (pour l'affichage seulement)

        // Vitesse angulaire entre deux poses horodatees (module de realite virtuelle du moteur).
        // Direction = axe de la rotation, longueur = vitesse en radians par seconde.
        NkVec3f VitesseAngulaire(const NkQuatf &from, const NkQuatf &to, float32 dtSeconds) {
                if (dtSeconds <= 0.f) {
                        return NkVec3f(0.f, 0.f, 0.f);
                }
                NkQuatf delta = (to * from.Conjugate()).Normalized();
                gDeltaW = delta.w; // lecture pour l'affichage
                const float32 angleRad = delta.Angle().Rad();
                if (angleRad <= 1e-6f) {
                        return NkVec3f(0.f, 0.f, 0.f);
                }
                return delta.Axis() * (angleRad / dtSeconds);
        }

        float Propre(float v) { return std::fabs(v) < 5e-4f ? 0.0f : v; } // evite "-0.000"

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

        // MASALLE_TOUR : vitesse du lacet programme, en degres par seconde (defaut 20)
        double tourDegS = 20.0;
        if (const char *e = std::getenv("MASALLE_TOUR"); e && e[0]) {
                tourDegS = std::atof(e);
        }

        NkWindowConfig winCfg;
        winCfg.title = "Le chemin long (Echap = quitter)";
        winCfg.width = 1280;
        winCfg.height = 720;
        winCfg.centered = true;
        winCfg.resizable = false;

        NkWindow window(winCfg);
        if (!window.IsValid()) {
                logger.Error("[Chemin] Creation fenetre KO");
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
                logger.Error("[Chemin] Creation device KO");
                window.Close();
                return 2;
        }

        NkRendererConfig cfg = NkRendererConfig::ForGame(devInfo.api, devInfo.width, devInfo.height);
        NkRenderer *renderer = NkRenderer::Create(device, cfg);
        if (!renderer || !renderer->Initialize()) {
                logger.Error("[Chemin] Init renderer KO");
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

        // ── Boucle : un tour complet ──────────────────────────────────────────────
        std::vector<std::string> lignes;
        std::vector<double> normes;
        std::vector<int> numeros;
        double ns0 = 0.0, tPrev = 0.0, lacetDeg = 0.0;
        NkQuatf qPrev = NkQuatf::Identity();
        const float tangage = 0.0f;
        int frame = 0, negatifs = 0;

        while (running && window.IsOpen() && lacetDeg < 360.0) {
                events.PollEvents();
                if (!running) break;

                const double ns = (double)NkChrono::Now().nanoseconds;
                if (frame == 0) ns0 = ns;
                const double t = (ns - ns0) * 1e-9; // secondes depuis la premiere image
                ++frame;

                lacetDeg = tourDegS * t; // le lacet suit l'horloge
                const float lacet = (float)(lacetDeg * (double)kPi / 180.0);

                // Orientation : deux matrices puis une conversion.
                const NkMat4f rotation = NkMat4f::RotationY(NkAngle::FromRad(lacet)) *
                                         NkMat4f::RotationX(NkAngle::FromRad(tangage));
                const NkQuatf orientation = NkQuatf(rotation).Normalized();

                if (frame > 1) {
                        const NkVec3f v = VitesseAngulaire(qPrev, orientation, (float)(t - tPrev));
                        const double vx = (double)v.x * kRadVersDeg, vy = (double)v.y * kRadVersDeg,
                                     vz = (double)v.z * kRadVersDeg;
                        char buf[256];
                        std::snprintf(buf, sizeof(buf),
                                      "image %d : lacet = %.3f deg, vitesse = (%.3f, %.3f, %.3f) deg/s, delta.w = %.6f",
                                      frame, lacetDeg, Propre((float)vx), Propre((float)vy), Propre((float)vz), gDeltaW);
                        lignes.push_back(buf);
                        normes.push_back(std::sqrt(vx * vx + vy * vy + vz * vz));
                        numeros.push_back(frame);
                        if (gDeltaW < 0.0f) ++negatifs;
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

        NkRenderer::Destroy(renderer);
        NkDeviceFactory::Destroy(device);
        window.Close();

        // ── Sorties ───────────────────────────────────────────────────────────────
        if (lacetDeg < 360.0) {
                std::fprintf(stderr, "tour incomplet : lacet final %.3f deg (Echap ou fermeture avant 360)\n", lacetDeg);
                return 4;
        }
        for (const std::string &l : lignes) std::printf("%s\n", l.c_str());
        std::fflush(stdout);

        size_t iMax = 0;
        for (size_t i = 1; i < normes.size(); ++i)
                if (normes[i] > normes[iMax]) iMax = i;
        std::fprintf(stderr, "%d lignes, tour de %.3f deg a %.3f deg/s\n", (int)lignes.size(), lacetDeg, tourDegS);
        std::fprintf(stderr, "plus grande vitesse : image %d, norme %.3f deg/s\n", numeros[iMax], normes[iMax]);
        if (iMax > 0)
                std::fprintf(stderr, "image d'avant : image %d, norme %.3f deg/s\n", numeros[iMax - 1], normes[iMax - 1]);
        std::fprintf(stderr, "images avec delta.w < 0 : %d\n", negatifs);
        return 0;
}
