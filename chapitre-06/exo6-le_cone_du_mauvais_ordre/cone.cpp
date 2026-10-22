// Le cone du mauvais ordre : une petite salle dont la camera suit un quaternion
// (regard ET haut), dans les deux ordres de composition.
//
//   lacet*tangage : qLacet * qTangage   (ordre sans defaut, celui d'une tete)
//   tangage*lacet : qTangage * qLacet   (l'autre ordre)
//
// La souris ne pilote que le lacet (mouvements bruts accumules, lus une fois par
// image, puis vides). Le tangage est fixe a 30 degres. F1 passe d'un ordre a l'autre.
//
// Variables d'environnement :
//   MASALLE_LACET=<deg>     pose figee : lacet donne en degres, souris coupee
//   MASALLE_ORDRE=1         demarre dans l'ordre fautif (tangage*lacet)
//   MASALLE_SHOT=<image>    capture l'image donnee, puis quitte
//   MASALLE_SHOT_PATH=<png> nom de la capture (defaut masalle.png)
//   MASALLE_HAUTEURS=1      imprime les dix hauteurs puis quitte (sans fenetre)
//   MASALLE_API=<opengl|vulkan|dx11|dx12|sw>  API graphique (defaut opengl : DX12 ne compile pas les shaders ici)

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
#include "NKEvent/NkMouseEvent.h"
#include "NKTime/NkTime.h"
#include "NKLogger/NkLog.h"
#include "NKMath/NkAngle.h"
#include "NKMath/NkQuat.h"
#include "NKMath/NkMat.h"

#include "NKRHI/Core/NkDeviceFactory.h"
#include "NKRenderer/NkRenderer.h"
#include "NKRenderer/Core/NkCamera.h"
#include "NKRenderer/Mesh/NkMeshSystem.h"
#include "NKRenderer/Tools/Render3D/NkRender3D.h"
#include "NKRenderer/Tools/Render2D/NkRender2D.h"
#include "NKRenderer/Tools/Overlay/NkOverlayRenderer.h"
#include "NKRenderer/Tools/Offscreen/NkOffscreenTarget.h"

#ifdef DrawText
#undef DrawText
#endif

using namespace nkentseu;
using namespace nkentseu::math;
using namespace nkentseu::renderer;

static void ConfigureAppData(NkAppData &d) {
        d.appName = "LeConeDuMauvaisOrdre";
}
NK_REGISTER_ENTRY_APPDATA_UPDATER(ConfigureAppData)

namespace {
        const float kPi = 3.14159265358979f;
        const float kTangageDeg = 30.0f;     // tangage fixe
        const float kSensibilite = 0.1146f;  // degres par compte du capteur (mesure a l'exo 2)
        const NkVec3f kOeil(0.0f, 1.6f, 0.0f);

        float gDeltaX = 0.0f; // accumulateur des mouvements bruts

        // Orientation de la tete : les deux ordres de composition.
        NkQuatf Orientation(float lacetDeg, float tangageDeg, bool fautif) {
                const NkQuatf qLacet = NkQuatf::RotateY(NkAngle(lacetDeg));
                const NkQuatf qTangage = NkQuatf::RotateX(NkAngle(tangageDeg));
                return fautif ? (qTangage * qLacet) : (qLacet * qTangage);
        }

        // Hauteur du regard : angle au-dessus de l'horizon, en degres.
        float Hauteur(const NkQuatf &q) {
                const NkVec3f regard = q * NkVec3f(0.0f, 0.0f, -1.0f);
                float y = regard.y;
                if (y > 1.0f) y = 1.0f;
                if (y < -1.0f) y = -1.0f;
                const float h = std::asin(y) * 180.0f / kPi;
                return std::fabs(h) < 5e-3f ? 0.0f : h; // evite "-0.00"
        }

        void ImprimerHauteurs() {
                const int lacets[5] = {0, 45, 90, 135, 180};
                for (int ordre = 0; ordre < 2; ++ordre) {
                        for (int i = 0; i < 5; ++i) {
                                const NkQuatf q = Orientation((float)lacets[i], kTangageDeg, ordre == 1);
                                std::printf("%s, lacet %d : hauteur %.2f deg\n",
                                            ordre == 0 ? "lacet*tangage" : "tangage*lacet", lacets[i], Hauteur(q));
                        }
                }
                std::fflush(stdout);
        }
}

int nkmain(const NkEntryState &state) {
        (void)state;

        // ── 0) Les dix hauteurs, avant d'ouvrir la fenetre ────────────────────────
        ImprimerHauteurs();
        if (const char *h = std::getenv("MASALLE_HAUTEURS"); h && h[0] == '1')
                return 0;

        // ── 1) Reglages par l'environnement ───────────────────────────────────────
        float lacetDeg = 0.0f;
        bool figee = false;
        if (const char *e = std::getenv("MASALLE_LACET"); e && e[0]) {
                lacetDeg = (float)std::atof(e);
                figee = true;
        }
        bool fautif = false;
        if (const char *e = std::getenv("MASALLE_ORDRE"); e && e[0] == '1')
                fautif = true;
        const char *shotEnv = std::getenv("MASALLE_SHOT");
        uint64 captureFrame = shotEnv ? (uint64)std::atoll(shotEnv) : 0;
        const char *shotPathEnv = std::getenv("MASALLE_SHOT_PATH");
        const char *capturePath = shotPathEnv ? shotPathEnv : "masalle.png";
        const uint64 captureLead = 3;

        // ── 2) Fenetre, device, renderer (comme au tutoriel 04) ───────────────────
        NkWindowConfig winCfg;
        winCfg.title = "Le cone du mauvais ordre (F1 = changer d'ordre, Echap = quitter)";
        winCfg.width = 1280;
        winCfg.height = 720;
        winCfg.centered = true;
        winCfg.resizable = false;

        NkWindow window(winCfg);
        if (!window.IsValid()) {
                logger.Error("[Cone] Creation fenetre KO");
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
                logger.Error("[Cone] Creation device KO");
                window.Close();
                return 2;
        }

        NkRendererConfig cfg = NkRendererConfig::ForGame(devInfo.api, devInfo.width, devInfo.height);
        NkRenderer *renderer = NkRenderer::Create(device, cfg);
        if (!renderer || !renderer->Initialize()) {
                logger.Error("[Cone] Init renderer KO");
                NkRenderer::Destroy(renderer);
                NkDeviceFactory::Destroy(device);
                window.Close();
                return 3;
        }

        NkRender3D *r3d = renderer->GetRender3D();
        NkOverlayRenderer *overlay = renderer->GetOverlay();
        NkRender2D *r2d = renderer->GetRender2D();
        NkMeshHandle meshCube = renderer->GetMeshSystem()->GetCube();
        NkMeshHandle meshSphere = renderer->GetMeshSystem()->GetSphere();

        // ── 3) Evenements ─────────────────────────────────────────────────────────
        bool running = true;
        const uint32 W = devInfo.width;
        const uint32 H = devInfo.height;
        NkEventSystem &events = NkEvents();

        events.AddEventCallback<NkWindowCloseEvent>([&](NkWindowCloseEvent *) { running = false; });
        events.AddEventCallback<NkKeyPressEvent>([&](NkKeyPressEvent *e) {
                if (e->GetKey() == NkKey::NK_ESCAPE)
                        running = false;
                else if (e->GetKey() == NkKey::NK_F1)
                        fautif = !fautif; // une touche passe d'un ordre a l'autre
        });
        events.AddEventCallback<NkMouseRawEvent>([&](NkMouseRawEvent *e) {
                if (!figee)
                        gDeltaX += (float)e->GetDeltaX(); // accumuler ...
        });

        if (!figee) { // souris capturee, rendue a la fermeture
                window.ShowMouse(false);
                window.CaptureMouse(true);
                window.ClipMouseToClient(true);
        }

        // ── 4) Capture : meme patron que la demo du moteur (NK_CAPTURE) ───────────
        NkOffscreenTarget captureTarget;
        bool captureArmed = false;

        // ── 5) Boucle ─────────────────────────────────────────────────────────────
        NkClock clock;
        uint64 frame = 0;

        while (running && window.IsOpen()) {
                events.PollEvents();
                if (!running)
                        break;
                (void)clock.Tick();
                ++frame;

                // ... lire une fois par image, puis vider. Droite = tourner a droite = lacet qui diminue.
                if (!figee) {
                        lacetDeg -= gDeltaX * kSensibilite;
                        gDeltaX = 0.0f;
                }

                // Capture : redirection -> rendu -> lecture -> restauration (one-shot).
                if (captureFrame > 0) {
                        if (!captureArmed && frame + captureLead >= captureFrame) {
                                NkOffscreenDesc od;
                                od.width = W;
                                od.height = H;
                                od.hasDepth = false;
                                od.colorFmt = ::nkentseu::NkGPUFormat::NK_RGBA8_UNORM;
                                od.readback = true;
                                od.name = "masalle_capture";
                                if (captureTarget.Init(device, renderer->GetTextures(), od)) {
                                        renderer->SetFinalColorTarget(
                                                renderer->GetTextures()->GetRHIHandle(captureTarget.GetColorResult()));
                                        captureArmed = true;
                                } else {
                                        logger.Warnf("[Cone] capture : offscreen KO, annulee\n");
                                        captureFrame = 0;
                                }
                        } else if (captureArmed && frame > captureFrame) {
                                device->WaitIdle();
                                const bool ok = captureTarget.Capture(capturePath);
                                logger.Infof("[Cone] capture image %llu -> %s (%s)\n",
                                             (unsigned long long)frame, capturePath, ok ? "OK" : "ECHEC");
                                renderer->SetFinalColorTarget(NkTextureHandle{});
                                captureTarget.Shutdown();
                                captureArmed = false;
                                captureFrame = 0;
                                running = false; // la pose est prise : on quitte
                                continue;
                        }
                }

                if (!renderer->BeginFrame())
                        continue;

                // La camera suit le quaternion : le regard ET le haut en viennent.
                const NkQuatf q = Orientation(lacetDeg, kTangageDeg, fautif);
                NkCamera3DData camData;
                camData.position = kOeil;
                camData.target = kOeil + q * NkVec3f(0.0f, 0.0f, -1.0f);
                camData.up = q * NkVec3f(0.0f, 1.0f, 0.0f);
                camData.fovY = 60.f;
                camData.aspect = (float32)W / (float32)H;
                camData.nearPlane = 0.05f;
                camData.farPlane = 50.f;
                NkCamera3D cam(camData);

                NkSceneContext sctx;
                sctx.camera = cam;
                sctx.time = 0.f;
                sctx.ambientIntensity = 2.0f;

                NkLightDesc sun;
                sun.type = NkLightType::NK_DIRECTIONAL;
                sun.direction = {-0.4f, -1.f, -0.3f};
                sun.color = {1.f, 0.95f, 0.85f};
                sun.intensity = 2.f;
                sun.castShadow = false;
                sctx.lights.PushBack(sun);
                NkLightDesc fill; // lumiere vers le haut : eclaire le plafond
                fill.type = NkLightType::NK_DIRECTIONAL;
                fill.direction = {0.3f, 1.f, 0.2f};
                fill.color = {1.f, 1.f, 1.f};
                fill.intensity = 1.5f;
                fill.castShadow = false;
                sctx.lights.PushBack(fill);

                r3d->BeginScene(sctx);

                auto boite = [&](NkVec3f c, NkVec3f s, NkVec3f tint) {
                        NkDrawCall3D dc;
                        dc.mesh = meshCube;
                        dc.transform = NkMat4f::Translate(c) * NkMat4f::Scale(s);
                        dc.aabb = {{c.x - s.x * 0.5f, c.y - s.y * 0.5f, c.z - s.z * 0.5f},
                                   {c.x + s.x * 0.5f, c.y + s.y * 0.5f, c.z + s.z * 0.5f}};
                        dc.castShadow = false;
                        dc.tint = tint;
                        dc.metallic = 0.f;
                        dc.roughness = 1.f;
                        r3d->Submit(dc);
                };

                // Salle 4 x 4 m, hauteur 2,5 m. L'avant (lacet 0) regarde le mur de la porte (-Z).
                boite({0.f, -0.05f, 0.f}, {4.2f, 0.1f, 4.2f}, {0.50f, 0.45f, 0.40f}); // sol
                boite({0.f, 2.55f, 0.f}, {4.2f, 0.1f, 4.2f}, {0.90f, 0.90f, 0.90f});  // plafond
                boite({0.f, 1.25f, -2.05f}, {4.2f, 2.5f, 0.1f}, {0.55f, 0.65f, 0.85f}); // mur avant (porte), bleu
                boite({0.f, 1.25f, 2.05f}, {4.2f, 2.5f, 0.1f}, {0.85f, 0.80f, 0.50f});  // mur arriere, jaune
                boite({-2.05f, 1.25f, 0.f}, {0.1f, 2.5f, 4.2f}, {0.80f, 0.45f, 0.45f}); // mur gauche, rouge
                boite({2.05f, 1.25f, 0.f}, {0.1f, 2.5f, 4.2f}, {0.45f, 0.75f, 0.50f});  // mur droit, vert
                boite({0.f, 1.0f, -1.97f}, {0.9f, 2.0f, 0.06f}, {0.45f, 0.28f, 0.12f}); // porte 0,9 x 2,0
                boite({0.f, 2.45f, 0.f}, {0.6f, 0.1f, 0.6f}, {1.0f, 1.0f, 1.0f});       // lampe au plafond

                // Bande sombre a hauteur des yeux (1,6 m) : l'horizon de la salle.
                boite({0.f, 1.6f, -1.92f}, {4.0f, 0.06f, 0.04f}, {0.04f, 0.04f, 0.08f});
                boite({0.f, 1.6f, 1.92f}, {4.0f, 0.06f, 0.04f}, {0.04f, 0.04f, 0.08f});
                boite({-1.92f, 1.6f, 0.f}, {0.04f, 0.06f, 4.0f}, {0.04f, 0.04f, 0.08f});
                boite({1.92f, 1.6f, 0.f}, {0.04f, 0.06f, 4.0f}, {0.04f, 0.04f, 0.08f});

                { // poignee de la porte
                        NkDrawCall3D dc;
                        dc.mesh = meshSphere;
                        dc.transform = NkMat4f::Translate({0.3f, 1.0f, -1.92f}) * NkMat4f::Scale({0.08f, 0.08f, 0.08f});
                        dc.aabb = {{0.2f, 0.9f, -2.0f}, {0.4f, 1.1f, -1.84f}};
                        dc.castShadow = false;
                        dc.tint = {1.f, 0.85f, 0.2f};
                        dc.metallic = 0.f;
                        dc.roughness = 0.5f;
                        r3d->Submit(dc);
                }

                if (overlay && !shotEnv) { // pas de panneau sur les captures
                        overlay->BeginOverlay(renderer->GetCmd(), W, H);
                        if (r2d)
                                r2d->FillRect({10.f, 10.f, 360.f, 70.f}, {0.f, 0.f, 0.f, 0.6f});
                        overlay->DrawText({20.f, 30.f}, "ordre : %s", fautif ? "tangage*lacet (fautif)" : "lacet*tangage");
                        overlay->DrawText({20.f, 50.f}, "lacet %.1f deg, tangage %.1f deg", lacetDeg, kTangageDeg);
                        overlay->DrawText({20.f, 70.f}, "F1 = changer d'ordre | Echap = quitter");
                        overlay->EndOverlay();
                }

                renderer->Present();
                renderer->EndFrame();
        }

        // ── 6) Fermeture propre : rendre la souris AVANT de quitter ───────────────
        if (!figee) {
                window.ClipMouseToClient(false);
                window.CaptureMouse(false);
                window.ShowMouse(true);
        }
        if (captureArmed) {
                renderer->SetFinalColorTarget(NkTextureHandle{});
                captureTarget.Shutdown();
        }
        device->WaitIdle();
        NkRenderer::Destroy(renderer);
        NkDeviceFactory::Destroy(device);
        window.Close();
        return 0;
}
