# Chapitre 3 — Exercice 6 : état contre événement

## Le code

```cpp
bool enCours = true;
uint64 imagesEspaceTenu = 0;
uint64 evenementsPressionEspace = 0;

// Fermeture propre (meme mecanisme que l'exo 4)
NkEvents().AddEventCallback<NkWindowCloseEvent>([&enCours](NkWindowCloseEvent *e) {
        (void)e;
        enCours = false;
});

// Compteur "evenement" : incremente a chaque NkKeyPressEvent sur Espace
NkEvents().AddEventCallback<NkKeyPressEvent>([&evenementsPressionEspace](NkKeyPressEvent *e) {
        if (e->GetKey() == NkKey::NK_SPACE) {
                evenementsPressionEspace++;
        }
});

// Affichage des que Espace est relachee
NkEvents().AddEventCallback<NkKeyReleaseEvent>(
        [&imagesEspaceTenu, &evenementsPressionEspace](NkKeyReleaseEvent *e) {
                if (e->GetKey() == NkKey::NK_SPACE) {
                        std::printf("Images ou Espace etait tenue (IsKeyDown) : %llu\n",
                                                (unsigned long long)imagesEspaceTenu);
                        std::printf("NkKeyPressEvent recus sur Espace          : %llu\n",
                                                (unsigned long long)evenementsPressionEspace);
                }
        });

while (enCours) {
        NkEvents().PollEvents();

        // Compteur "etat" : incremente a chaque image ou la touche est tenue
        if (NkInput.IsKeyDown(NkKey::NK_SPACE)) {
                imagesEspaceTenu++;
        }
}
```

Fichier complet : [`00-EtatContreEvenement/main.cpp`](00-EtatContreEvenement/main.cpp).

## Accroc technique

Premier essai avec la boucle bornée sur `fenetre.IsOpen()` et l'affichage des
compteurs placé *après* la boucle : le clic sur la croix n'a pas fermé la
fenêtre (aucune réaction), et il a fallu tuer le processus au terminal
(`Ctrl+C`) — ce qui a empêché l'exécution des `printf`, puisqu'une
interruption brutale du processus ne passe jamais par la fin normale de la
boucle. Correction : reprise du mécanisme de fermeture propre de l'exercice 4
(rappel sur `NkWindowCloseEvent` qui bascule un booléen `enCours`), et
affichage des compteurs déplacé sur le rappel `NkKeyReleaseEvent` (dès que
Espace est relâchée), pour ne plus dépendre uniquement de la fermeture de la
fenêtre pour voir le résultat.

## Test : une pression d'environ une seconde

Espace maintenue environ 1 seconde puis relâchée :

Images ou Espace etait tenue (IsKeyDown) : 32905
NkKeyPressEvent recus sur Espace : 1


## Explication de l'écart

Les deux compteurs ne mesurent pas la même chose :

- **`NkKeyPressEvent`** est un événement **discret** : il n'est émis qu'une
  seule fois, au moment précis où la touche passe de « relâchée » à
  « enfoncée ». Peu importe combien de temps la touche reste ensuite
  maintenue, aucun nouvel événement `NkKeyPressEvent` n'est généré tant
  qu'elle n'est pas relâchée puis réappuyée — d'où le compteur à **1** pour
  une seule pression, aussi longue soit-elle.

- **`NkInput.IsKeyDown()`** est un **état interrogé en continu** (polling) :
  la boucle principale appelle `PollEvents()` puis teste l'état de la touche
  à *chaque tour de boucle*, c'est-à-dire potentiellement plusieurs dizaines
  de milliers de fois par seconde (32 905 tours en ~1 seconde ici, soit
  environ 33 000 images/s — la boucle n'a aucune limite de fréquence, ni
  `VSync` ni `sleep`, elle tourne donc aussi vite que le CPU le permet). Tant
  que la touche reste physiquement enfoncée, cette condition reste vraie à
  chaque tour, donc le compteur augmente d'une unité par tour de boucle.

En clair : l'événement compte des **transitions** (combien de fois la touche
a changé d'état), tandis que le polling d'état compte des **échantillons**
(combien de fois le programme a vérifié l'état pendant que la touche était
enfoncée). Le second dépend directement de la vitesse de la boucle — une
boucle deux fois plus rapide aurait donné un compteur deux fois plus grand
pour la même durée de pression — alors que le premier ne dépend que du
nombre de fois où l'utilisateur a réellement appuyé sur la touche. C'est
pourquoi, pour du gameplay, on préfère `IsKeyDown` pour un mouvement continu
(avancer tant que la touche est tenue) mais un événement `KeyPress` pour une
action ponctuelle (tirer, sauter) qu'on ne veut déclencher qu'une fois par
appui.
