// Le garde-fou : une classe qui termine la trame toute seule, quel que soit le chemin de sortie, et la comparaison d'une
// boucle avec elle et sans elle.
// Fausse session : un drapeau "trame ouverte" (faux au depart) et trois methodes.
//   Wait : refuse si la trame est ouverte. Begin : refuse si elle est ouverte, sinon l'ouvre.
//   End : refuse si elle est fermee, sinon la ferme. La session compte les debuts et les fins acceptes, et tous les refus.
// GardeTrame est creee juste apres Begin et appelle End dans son destructeur ; elle ne se copie pas.
// Entree : T, puis (si T > 0) un mot de T signes, le chemin de la boucle a chaque image :
//   N : on rend. V : rien a rendre. C : les vues echouent, la boucle fait continue. R : la boucle fait return et s'arrete.
//   Chaque image appelle Wait puis Begin. Si Wait est refuse, la boucle n'a pas d'etat : N et C deviennent V.
//   Sans garde, End est ecrit en bas du corps de boucle (C et R le sautent). Avec garde, le destructeur le fait partout.
// Sortie : deux lignes, "SANS" puis "AVEC" : DEBUTS, FINS, REFUS, IMAGES (images rendues), OUVERTE OUI|NON.
#include <iostream>
#include <string>

class Session {
public:
    bool Wait() {
        if (ouverte_) { ++refus_; return false; }
        return true;
    }
    bool Begin() {
        if (ouverte_) { ++refus_; return false; }
        ouverte_ = true;
        ++debuts_;
        return true;
    }
    bool End() {
        if (!ouverte_) { ++refus_; return false; }
        ouverte_ = false;
        ++fins_;
        return true;
    }
    long long debuts() const { return debuts_; }
    long long fins() const { return fins_; }
    long long refus() const { return refus_; }
    bool ouverte() const { return ouverte_; }

private:
    bool ouverte_ = false;
    long long debuts_ = 0, fins_ = 0, refus_ = 0;
};

// Termine la trame dans son destructeur : le return, le continue et la fin du corps de boucle passent tous par la.
class GardeTrame {
public:
    explicit GardeTrame(Session& session) : session_(session) {}
    ~GardeTrame() { session_.End(); }
    GardeTrame(const GardeTrame&) = delete;            // une copie fermerait la trame deux fois
    GardeTrame& operator=(const GardeTrame&) = delete;

private:
    Session& session_;
};

// Boucle sans garde : End est ecrit en bas du corps, donc C (continue) et R (return) le sautent.
static long long boucleSansGarde(Session& session, const std::string& signes) {
    long long images = 0;
    for (const char signeBrut : signes) {
        char signe = signeBrut;
        const bool attenteAcceptee = session.Wait();
        session.Begin();
        if (!attenteAcceptee && (signe == 'N' || signe == 'C')) signe = 'V'; // pas d'etat : on ne rend pas
        if (signe == 'C') continue;   // les vues echouent : End est saute
        if (signe == 'R') return images; // la boucle s'arrete : End est saute
        if (signe == 'N') ++images;
        session.End();
    }
    return images;
}

// Boucle avec garde : le destructeur de GardeTrame appelle End sur tous les chemins.
static long long boucleAvecGarde(Session& session, const std::string& signes) {
    long long images = 0;
    for (const char signeBrut : signes) {
        char signe = signeBrut;
        const bool attenteAcceptee = session.Wait();
        session.Begin();
        GardeTrame garde(session);    // creee juste apres Begin
        if (!attenteAcceptee && (signe == 'N' || signe == 'C')) signe = 'V';
        if (signe == 'C') continue;
        if (signe == 'R') return images;
        if (signe == 'N') ++images;
    }
    return images;
}

static void afficher(const char* nom, const Session& session, long long images) {
    std::cout << nom << " DEBUTS " << session.debuts() << " FINS " << session.fins() << " REFUS " << session.refus()
              << " IMAGES " << images << " OUVERTE " << (session.ouverte() ? "OUI" : "NON") << "\n";
}

int main() {
    long long nbImages = 0;
    if (!(std::cin >> nbImages)) nbImages = 0;
    std::string signes;
    if (nbImages > 0) std::cin >> signes;

    Session sans;
    const long long imagesSans = boucleSansGarde(sans, signes);
    afficher("SANS", sans, imagesSans);

    Session avec;
    const long long imagesAvec = boucleAvecGarde(avec, signes);
    afficher("AVEC", avec, imagesAvec);
    return 0;
}
