// Predire et comparer : a chaque image, on predit l'orientation de la tete pour l'instant ou l'image
// sera vue (une periode d'ecran plus tard), on garde cette prediction UNE image, puis on la compare a
// l'orientation mesuree a l'image suivante. On mesure aussi l'erreur sans prediction (la pose de l'image
// d'avant, prise telle quelle, contre celle de maintenant).
//   MASALLE_HZ=<frequence de l'ecran> est OBLIGATOIRE : la periode d'ecran en decoule.
//   Souris lue UNE fois par image (comptes accumules puis vides) ; lacet -= x * 0,0022 rad, tangage -= y * 0,0022 rad.
//   Les 30 premieres images sont ignorees ; ensuite 1000 comparaisons. Echap quitte (mesure incomplete).
// Sortie standard (2 lignes) :
//   avec prediction : moyenne M deg, max X deg, sur N images
//   sans prediction : moyenne M deg, max X deg, sur N images
// Sortie d'erreur : diagnostics (immobiles, rapports, dt reel, variante prediction sur dt reel, angle par atan2).
// MASALLE_API=<opengl|vulkan|dx11|dx12|sw> choisit l'API (defaut opengl).

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
        d.appName = "PredireEtComparer";
}
NK_REGISTER_ENTRY_APPDATA_UPDATER(ConfigureAppData)

namespace {
        const float kPi = 3.14159265358979f;
        const float kSensibilite = 0.0022f; // radian par compte du capteur
        const int kEchauffement = 30;       // images ignorees au depart
        const int kMesures = 1000;          // comparaisons retenues

        float gDeltaX = 0.0f; // accumulateurs des mouvements bruts
        float gDeltaY = 0.0f;

        // Vitesse angulaire entre deux poses horodatees (module de realite virtuelle du moteur).
        // Direction = axe de la rotation, longueur = vitesse en radians par seconde.
        NkVec3f VitesseAngulaire(const NkQuatf &from, const NkQuatf &to, float32 dtSeconds) {
                if (dtSeconds <= 0.f) {
                        return NkVec3f(0.f, 0.f, 0.f);
                }
                NkQuatf delta = (to * from.Conjugate()).Normalized();
                // Les deux hemispheres du quaternion codent la meme rotation ; on
                // force le chemin court, sinon un delta minuscule peut se lire
                // comme un tour presque complet dans l'autre sens.
                if (delta.w < 0.f) {
                        delta = NkQuatf(-delta.x, -delta.y, -delta.z, -delta.w);
                }
                const float32 angleRad = delta.Angle().Rad();
                if (angleRad <= 1e-6f) {
                        return NkVec3f(0.f, 0.f, 0.f);
                }
                return delta.Axis() * (angleRad / dtSeconds);
        }

        // Predire : faire tourner l'orientation a cette vitesse pendant le temps qui reste avant l'affichage.
        NkQuatf Extrapoler(const NkQuatf &orientation, const NkVec3f &vitesse, float32 dtSeconds) {
                const float32 speedRad = vitesse.Len();
                if (speedRad > 1e-6f) {
                        const NkVec3f axis = vitesse * (1.f / speedRad);
                        // Delta en espace de BASE : il se compose a GAUCHE de
                        // l'orientation courante (meme convention que la mesure).
                        const NkQuatf delta(NkAngle::FromRad(speedRad * dtSeconds), axis);
                        return (delta * orientation).Normalized();
                }
                return orientation;
        }

        // Angle (degres) qui separe a de b : formule de l'enonce.
        double AngleDeg(const NkQuatf &a, const NkQuatf &b) {
                return (double)(b * a.Conjugate()).Normalized().Angle().Deg();
        }
        // Le meme angle par atan2 en double : precis aussi pour les tres petits angles (diagnostic).
        double AngleDegPrecis(const NkQuatf &a, const NkQuatf &b) {
                const NkQuatf q = (b * a.Conjugate()).Normalized();
                const double s = std::sqrt((double)q.x * q.x + (double)q.y * q.y + (double)q.z * q.z);
                return 2.0 * std::atan2(s, std::fabs((double)q.w)) * 180.0 / (double)kPi;
        }

        void Stats(const std::vector<double> &v, double &moy, double &mx) {
                moy = 0.0;
                mx = 0.0;
                for (double e : v) {
                        moy += e;
                        if (e > mx) mx = e;
                }
                if (!v.empty()) moy /= (double)v.size();
        }

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

        // ── La periode d'ecran : MASALLE_HZ, obligatoire ──────────────────────────
        const char *hzEnv = std::getenv("MASALLE_HZ");
        const double hz = (hzEnv && hzEnv[0]) ? std::atof(hzEnv) : 0.0;
        if (hz <= 1.0) {
                std::fprintf(stderr, "MASALLE_HZ manquante ou invalide : donner la frequence de l'ecran (ex. MASALLE_HZ=60)\n");
                return 5;
        }
        const float periodeS = (float)(1.0 / hz);

        NkWindowConfig winCfg;
        winCfg.title = "Predire et comparer (bougez la souris ; Echap = quitter)";
        winCfg.width = 1280;
        winCfg.height = 720;
        winCfg.centered = true;
        winCfg.resizable = false;

        NkWindow window(winCfg);
        if (!window.IsValid()) {
                logger.Error("[Prediction] Creation fenetre KO");
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
                logger.Error("[Prediction] Creation device KO");
                window.Close();
                return 2;
        }

        NkRendererConfig cfg = NkRendererConfig::ForGame(devInfo.api, devInfo.width, devInfo.height);
        NkRenderer *renderer = NkRenderer::Create(device, cfg);
        if (!renderer || !renderer->Initialize()) {
                logger.Error("[Prediction] Init renderer KO");
                NkRenderer::Destroy(renderer);
                NkDeviceFactory::Destroy(device);
                window.Close();
                return 3;
        }
        NkRender3D *r3d = renderer->GetRender3D();
        NkMeshHandle meshCube = renderer->GetMeshSystem()->GetCube();
        const float aspect = (float)devInfo.width / (float)devInfo.height;

        bool running = true;
        const char *raison = "boucle terminee sans cause enregistree";
        NkEventSystem &events = NkEvents();
        events.AddEventCallback<NkWindowCloseEvent>([&](NkWindowCloseEvent *) { running = false; raison = "fermeture de la fenetre"; });
        events.AddEventCallback<NkKeyPressEvent>([&](NkKeyPressEvent *e) {
                if (e->GetKey() == NkKey::NK_ESCAPE) { running = false; raison = "touche Echap"; }
        });
        events.AddEventCallback<NkMouseRawEvent>([&](NkMouseRawEvent *e) { // accumuler ...
                gDeltaX += (float)e->GetDeltaX();
                gDeltaY += (float)e->GetDeltaY();
        });

        window.ShowMouse(false); // souris capturee, rendue a la fermeture
        window.CaptureMouse(true);
        window.ClipMouseToClient(true);

        // ── Boucle ────────────────────────────────────────────────────────────────
        std::vector<double> ePred, eAucune, eDt, ePredP, eAucuneP, eDtP, dts;
        int immobiles = 0;
        double ns0 = 0.0, tPrev = 0.0;
        float lacet = 0.0f, tangage = 0.0f; // radians
        NkQuatf qPrev = NkQuatf::Identity();  // orientation de l'image d'avant
        NkQuatf qPred = NkQuatf::Identity();  // prediction faite a l'image d'avant
        NkVec3f vPrev(0.f, 0.f, 0.f);         // vitesse mesuree a l'image d'avant
        bool aPrecedent = false;
        int frame = 0;

        while (running && window.IsOpen() && (int)ePred.size() < kMesures) {
                events.PollEvents();
                if (!running) break;

                const double ns = (double)NkChrono::Now().nanoseconds;
                if (frame == 0) ns0 = ns;
                const double t = (ns - ns0) * 1e-9;
                ++frame;

                // ... lire UNE fois par image, puis vider.
                const float dx = gDeltaX, dy = gDeltaY;
                gDeltaX = 0.0f;
                gDeltaY = 0.0f;
                lacet -= dx * kSensibilite;
                tangage -= dy * kSensibilite;

                // Orientation : deux matrices puis une conversion.
                const NkMat4f rotation = NkMat4f::RotationY(NkAngle::FromRad(lacet)) *
                                         NkMat4f::RotationX(NkAngle::FromRad(tangage));
                const NkQuatf orientation = NkQuatf(rotation).Normalized();

                NkVec3f v(0.f, 0.f, 0.f);
                if (aPrecedent) {
                        const float dtS = (float)(t - tPrev);
                        if (frame > kEchauffement) {
                                // La prediction faite a l'image d'avant est jugee contre la pose d'AUJOURD'HUI.
                                ePred.push_back(AngleDeg(qPred, orientation));
                                eAucune.push_back(AngleDeg(qPrev, orientation));
                                // Diagnostic : prediction faite sur l'intervalle reel jusqu'a cette image.
                                const NkQuatf qDt = Extrapoler(qPrev, vPrev, dtS);
                                eDt.push_back(AngleDeg(qDt, orientation));
                                ePredP.push_back(AngleDegPrecis(qPred, orientation));
                                eAucuneP.push_back(AngleDegPrecis(qPrev, orientation));
                                eDtP.push_back(AngleDegPrecis(qDt, orientation));
                                dts.push_back((double)dtS);
                                if (dx == 0.0f && dy == 0.0f) ++immobiles;
                                if (ePred.size() % 100 == 0) std::fprintf(stderr, "progression : %d/%d\n", (int)ePred.size(), kMesures);
                        }
                        v = VitesseAngulaire(qPrev, orientation, dtS);
                }
                // Predire pour l'instant ou cette image sera vue : une periode d'ecran plus tard.
                qPred = Extrapoler(orientation, v, periodeS);
                qPrev = orientation;
                vPrev = v;
                tPrev = t;
                aPrecedent = true;

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

        window.ClipMouseToClient(false); // rendre la souris AVANT de quitter
        window.CaptureMouse(false);
        window.ShowMouse(true);
        NkRenderer::Destroy(renderer);
        NkDeviceFactory::Destroy(device);
        window.Close();

        // ── Sorties ───────────────────────────────────────────────────────────────
        if ((int)ePred.size() < kMesures) {
                std::fprintf(stderr, "mesure incomplete : %d comparaisons sur %d (arret : %s)\n",
                             (int)ePred.size(), kMesures, raison);
                return 4;
        }
        double mP, xP, mA, xA;
        Stats(ePred, mP, xP);
        Stats(eAucune, mA, xA);
        std::printf("avec prediction : moyenne %.4f deg, max %.4f deg, sur %d images\n", mP, xP, (int)ePred.size());
        std::printf("sans prediction : moyenne %.4f deg, max %.4f deg, sur %d images\n", mA, xA, (int)eAucune.size());
        std::fflush(stdout);

        // Diagnostics (sortie d'erreur)
        const int n = (int)ePred.size();
        int mobiles = 0, ge10 = 0, pire = 0;
        std::vector<double> rapports;
        for (int i = 0; i < n; ++i) {
                if (ePred[i] > eAucune[i]) ++pire;
                if (eAucune[i] > 1e-6) {
                        ++mobiles;
                        if (ePred[i] * 10.0 <= eAucune[i]) ++ge10;
                        if (ePred[i] > 1e-9) rapports.push_back(eAucune[i] / ePred[i]);
                }
        }
        std::sort(rapports.begin(), rapports.end());
        const double mediane = rapports.empty() ? 0.0 : rapports[rapports.size() / 2];
        double dtMoy = 0.0, dtMin = 1e300, dtMax = 0.0;
        for (double d : dts) {
                dtMoy += d;
                dtMin = std::min(dtMin, d);
                dtMax = std::max(dtMax, d);
        }
        dtMoy /= (double)dts.size();
        double mD, xD, mPp, xPp, mAp, xAp, mDp, xDp;
        Stats(eDt, mD, xD);
        Stats(ePredP, mPp, xPp);
        Stats(eAucuneP, mAp, xAp);
        Stats(eDtP, mDp, xDp);
        std::fprintf(stderr, "periode d'ecran : %.3f ms (MASALLE_HZ=%g)\n", (double)periodeS * 1e3, hz);
        std::fprintf(stderr, "images immobiles (aucun compte souris) : %d sur %d\n", immobiles, n);
        std::fprintf(stderr, "images en mouvement (erreur sans prediction > 1e-6 deg) : %d ; dont facteur >= 10 : %d ; prediction pire que rien (toutes images) : %d\n",
                     mobiles, ge10, pire);
        std::fprintf(stderr, "rapport des moyennes sans/avec : %.3f ; mediane des rapports par image : %.3f\n",
                     mP > 0.0 ? mA / mP : 0.0, mediane);
        std::fprintf(stderr, "dt reel entre images : moyenne %.3f ms, min %.3f ms, max %.3f ms\n", dtMoy * 1e3, dtMin * 1e3, dtMax * 1e3);
        std::fprintf(stderr, "variante prediction sur le dt reel : moyenne %.4f deg, max %.4f deg (rapport des moyennes %.3f)\n",
                     mD, xD, mD > 0.0 ? mA / mD : 0.0);
        std::fprintf(stderr, "angle par atan2 : avec %.4f / %.4f, sans %.4f / %.4f, dt reel %.4f / %.4f (moyenne / max, deg)\n",
                     mPp, xPp, mAp, xAp, mDp, xDp);
        return 0;
}
