// =============================================================================
// Exercice — LE NOM DE VOTRE CARTE (chapitre 4, exo 1)
// Simule la detection automatique de l'interface graphique de NKRHI,
// machine par machine, selon un ordre de preference fixe par plateforme.
// =============================================================================
#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>

// Les huit interfaces possibles. INCONNUE est une valeur de secours : elle
// n'appartient a aucun ordre de plateforme, donc elle ne sera jamais choisie
// et comptera toujours comme ignoree si jamais elle apparaissait.
enum class Api {
    VULKAN,
    DX12,
    DX11,
    OPENGL,
    OPENGLES,
    WEBGL,
    METAL,
    SOFTWARE,
    INCONNUE
};

Api ApiDepuisNom(const std::string &nom) {
    static const std::unordered_map<std::string, Api> table = {
        {"VULKAN",   Api::VULKAN},
        {"DX12",     Api::DX12},
        {"DX11",     Api::DX11},
        {"OPENGL",   Api::OPENGL},
        {"OPENGLES", Api::OPENGLES},
        {"WEBGL",    Api::WEBGL},
        {"METAL",    Api::METAL},
        {"SOFTWARE", Api::SOFTWARE},
    };
    auto it = table.find(nom);
    return (it != table.end()) ? it->second : Api::INCONNUE;
}

// Nom lisible : uniquement pour les interfaces qui peuvent reellement etre
// choisies (celles qui figurent dans au moins un ordre de plateforme).
std::string NomLisible(Api api) {
    switch (api) {
        case Api::VULKAN:   return "Vulkan";
        case Api::DX12:     return "DirectX 12";
        case Api::DX11:     return "DirectX 11";
        case Api::OPENGL:   return "OpenGL";
        case Api::METAL:    return "Metal";
        case Api::SOFTWARE: return "Software";
        default:            return "";
    }
}

// L'ordre de preference d'une plateforme, SOFTWARE toujours en dernier.
// Ce vecteur sert a la fois a choisir l'interface ET a savoir ce qui n'est
// "pas ignore" pour cette plateforme.
const std::vector<Api> &OrdrePourPlateforme(const std::string &plateforme) {
    static const std::vector<Api> windows = {Api::VULKAN, Api::DX12, Api::DX11, Api::OPENGL, Api::SOFTWARE};
    static const std::vector<Api> macos   = {Api::METAL, Api::OPENGL, Api::SOFTWARE};
    static const std::vector<Api> ios     = {Api::METAL, Api::SOFTWARE};
    static const std::vector<Api> android = {Api::VULKAN, Api::OPENGL, Api::SOFTWARE};
    static const std::vector<Api> defaut  = {Api::VULKAN, Api::OPENGL, Api::SOFTWARE};

    if (plateforme == "WINDOWS") return windows;
    if (plateforme == "MACOS")   return macos;
    if (plateforme == "IOS")     return ios;
    if (plateforme == "ANDROID") return android;
    return defaut;
}

int main() {
    std::ios::sync_with_stdio(false);

    int n = 0;
    if (!(std::cin >> n) || n < 0) {
        return 0;
    }

    int ignorees = 0;
    int logiciel = 0;
    std::unordered_set<std::string> nomsDistincts;

    for (int i = 0; i < n; ++i) {
        std::string nomMachine, plateforme;
        int k = 0;
        std::cin >> nomMachine >> plateforme >> k;

        std::unordered_set<Api> apisDeLaMachine;
        for (int j = 0; j < k; ++j) {
            std::string nomApi;
            std::cin >> nomApi;
            apisDeLaMachine.insert(ApiDepuisNom(nomApi));
        }

        const std::vector<Api> &ordre = OrdrePourPlateforme(plateforme);
        const std::unordered_set<Api> ordreEnEnsemble(ordre.begin(), ordre.end());

        // Une interface listee par la machine mais absente de l'ordre de sa
        // plateforme n'est jamais essayee : elle est ignoree.
        for (Api api : apisDeLaMachine) {
            if (ordreEnEnsemble.find(api) == ordreEnEnsemble.end()) {
                ++ignorees;
            }
        }

        // Selection : premiere interface de l'ordre (sans son dernier
        // element, SOFTWARE) presente chez la machine. Si aucune ne
        // correspond, SOFTWARE est choisi par defaut, meme absent de la liste.
        Api choisie = Api::SOFTWARE;
        for (std::size_t p = 0; p + 1 < ordre.size(); ++p) {
            if (apisDeLaMachine.find(ordre[p]) != apisDeLaMachine.end()) {
                choisie = ordre[p];
                break;
            }
        }

        if (choisie == Api::SOFTWARE) {
            ++logiciel;
        }

        const std::string nomAffiche = NomLisible(choisie);
        nomsDistincts.insert(nomAffiche);

        std::cout << nomMachine << ' ' << nomAffiche << '\n';
    }

    std::cout << "IGNOREES " << ignorees << '\n';
    std::cout << "LOGICIEL " << logiciel << '\n';
    std::cout << "DIFFERENTES " << nomsDistincts.size() << '\n';

    return 0;
}
