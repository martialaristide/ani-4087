#include <cstdint>
#include <iomanip>
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>

namespace {

// Les treize drapeaux simples.
const std::uint32_t RENDER2D = 1;
const std::uint32_t RENDER3D = 2;
const std::uint32_t TEXT = 4;
const std::uint32_t UI = 8;
const std::uint32_t SHADOW = 16;
const std::uint32_t POST_PROCESS = 32;
const std::uint32_t VFX = 64;
const std::uint32_t ANIMATION = 128;
const std::uint32_t OVERLAY = 256;
const std::uint32_t SIMULATION = 512;
const std::uint32_t OFFSCREEN = 1024;
const std::uint32_t RAYTRACING = 2048;
const std::uint32_t GPU_CULLING = 4096;

// La valeur par defaut, quand aucun drapeau n'est nomme (N == 0).
const std::uint32_t ALL = 4294967295u;

std::unordered_map<std::string, std::uint32_t> tableDesNoms() {
    return {
        {"RENDER2D", RENDER2D},
        {"RENDER3D", RENDER3D},
        {"TEXT", TEXT},
        {"UI", UI},
        {"SHADOW", SHADOW},
        {"POST_PROCESS", POST_PROCESS},
        {"VFX", VFX},
        {"ANIMATION", ANIMATION},
        {"OVERLAY", OVERLAY},
        {"SIMULATION", SIMULATION},
        {"OFFSCREEN", OFFSCREEN},
        {"RAYTRACING", RAYTRACING},
        {"GPU_CULLING", GPU_CULLING},
        {"NONE", 0u},
        {"2D_ESSENTIALS", RENDER2D | TEXT},
        {"3D_BASE", RENDER3D | SHADOW | POST_PROCESS},
        {"DEBUG", OVERLAY | SIMULATION},
        {"ALL", ALL},
    };
}

// Une dependance : un drapeau simple, et ce dont il a besoin, dans l'ordre d'affichage impose.
struct Dependance {
    const char* nom;
    std::uint32_t drapeau;
    std::vector<std::pair<const char*, std::uint32_t>> requis;
};

} // namespace

int main() {
    const std::unordered_map<std::string, std::uint32_t> noms = tableDesNoms();

    int n = 0;
    std::cin >> n;

    std::vector<std::string> inconnus;
    std::uint32_t valeur = (n == 0) ? ALL : 0u;

    for (int i = 0; i < n; ++i) {
        std::string nom;
        std::cin >> nom;

        auto it = noms.find(nom);
        if (it == noms.end()) {
            inconnus.push_back(nom);
            continue;
        }
        valeur |= it->second;
    }

    for (const std::string& nom : inconnus) {
        std::cout << "INCONNU " << nom << "\n";
    }

    std::cout << "VALEUR " << valeur << "\n";
    std::cout << "HEXA 0x" << std::hex << std::uppercase << std::setfill('0')
               << std::setw(8) << valeur << std::dec << std::nouppercase << "\n";

    const std::vector<Dependance> dependances = {
        {"TEXT", TEXT, {{"RENDER2D", RENDER2D}}},
        {"UI", UI, {{"RENDER2D", RENDER2D}, {"TEXT", TEXT}}},
        {"SHADOW", SHADOW, {{"RENDER3D", RENDER3D}}},
        {"OVERLAY", OVERLAY, {{"RENDER2D", RENDER2D}, {"TEXT", TEXT}}},
    };

    for (const Dependance& dep : dependances) {
        if ((valeur & dep.drapeau) == 0) {
            continue;
        }
        for (const auto& req : dep.requis) {
            if ((valeur & req.second) == 0) {
                std::cout << "MANQUE " << dep.nom << " " << req.first << "\n";
            }
        }
    }

    const std::vector<std::uint32_t> simples = {
        RENDER2D, RENDER3D, TEXT, UI, SHADOW, POST_PROCESS, VFX,
        ANIMATION, OVERLAY, SIMULATION, OFFSCREEN, RAYTRACING, GPU_CULLING,
    };

    int allumes = 0;
    for (std::uint32_t drapeau : simples) {
        if ((valeur & drapeau) != 0) {
            ++allumes;
        }
    }

    std::cout << "ALLUMES " << allumes << "\n";
    std::cout << "ETEINTS " << (static_cast<int>(simples.size()) - allumes) << "\n";

    return 0;
}
