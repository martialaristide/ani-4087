// Le double emprunt : montre ce que rendent AcquireImage et ReleaseImage appeles deux fois de suite, et ce que devient l'index.
// Entree : "n v N" (nombre d'images, valeur de depart de toutes les variables, nombre d'ordres), puis N ordres :
//   acquire X (X : lettre a-z), release, last.
// Sortie : une ligne par ordre, puis EMPRUNTEE OUI|NON et PROCHAINE p.
#include <iostream>
#include <string>

int main() {
    unsigned long long n = 1, v = 0;
    int nbOrdres = 0;
    if (!(std::cin >> n >> v >> nbOrdres)) nbOrdres = 0;
    if (n < 1) n = 1;

    // une valeur par lettre ; la valeur de depart tient sur 32 bits non signes (elle peut depasser un int signe)
    unsigned int variables[26];
    for (unsigned int &x : variables) x = static_cast<unsigned int>(v);

    unsigned int prochaine = 0;     // la prochaine image a preter
    bool empruntee = false;         // une image est-elle empruntee ?

    for (int i = 0; i < nbOrdres; ++i) {
        std::string ordre;
        if (!(std::cin >> ordre)) break;

        if (ordre == "acquire") {
            std::string nomVariable;
            std::cin >> nomVariable;
            const int lettre = nomVariable.empty() ? 0 : nomVariable[0] - 'a';
            if (empruntee) {
                // emprunt refuse : la variable n'est pas touchee
                std::cout << "ACQUIRE FAUX " << nomVariable << " " << variables[lettre] << "\n";
            } else {
                variables[lettre] = prochaine;
                empruntee = true;
                std::cout << "ACQUIRE VRAI " << nomVariable << " " << variables[lettre] << "\n";
            }
        } else if (ordre == "release") {
            if (!empruntee) {
                // un release refuse ne fait pas avancer la prochaine image
                std::cout << "RELEASE FAUX\n";
            } else {
                prochaine = (prochaine + 1) % static_cast<unsigned int>(n);
                empruntee = false;
                std::cout << "RELEASE VRAI\n";
            }
        } else if (ordre == "last") {
            const unsigned int j = (prochaine + static_cast<unsigned int>(n) - 1) % static_cast<unsigned int>(n);
            std::cout << "LAST " << j << "\n";
        }
    }

    std::cout << "EMPRUNTEE " << (empruntee ? "OUI" : "NON") << "\n";
    std::cout << "PROCHAINE " << prochaine << "\n";
    return 0;
}
