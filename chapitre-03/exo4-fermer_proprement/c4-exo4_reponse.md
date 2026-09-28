# Chapitre 3 — Exercice 4 : fermer proprement

## Le changement

Repris du programme de l'exercice 1. La boucle ne teste plus `fenetre.IsOpen()`
mais un booléen local `enCours`, mis à `false` par deux rappels indépendants :

```cpp
bool enCours = true;

// Rappel 1 : fermeture demandee par le systeme (clic sur la croix, Alt+F4...)
NkEvents().AddEventCallback<NkWindowCloseEvent>([&enCours](NkWindowCloseEvent *e) {
        (void)e;
        enCours = false;
});

// Rappel 2 : touche Echap -> meme sortie que le clic sur la croix
NkEvents().AddEventCallback<NkKeyPressEvent>([&enCours](NkKeyPressEvent *e) {
        if (e->GetKey() == NkKey::NK_ESCAPE) {
                enCours = false;
        }
});

while (enCours) {
        NkEvents().PollEvents();
}
```

Fichier complet : [`00-FermerProprement/main.cpp`](00-FermerProprement/main.cpp).

Même méthode d'intégration que les exercices précédents : nouveau projet
`FermerProprement` ajouté au `Tutoriels3D.jenga` existant via le helper
`tutoproject()`.

## Accroc technique

Premier essai avec des lambdas prenant une référence (`NkWindowCloseEvent &e`,
`NkKeyPressEvent &e`) : échec de compilation. `NkEventSystem::AddEventCallback`
invoque en interne le callback avec un **pointeur** (`callback(typed)` où
`typed` est un `T*`), pas une référence. Correction : lambdas prenant
`NkWindowCloseEvent *e` / `NkKeyPressEvent *e`, et accès aux membres via `e->`
au lieu de `e.`.

## Construction et lancement

$ export JENGA_TRUST_ALL=1
$ export PATH="/c/msys64/ucrt64/bin:$PATH"
$ cd /c/Users/Lenovo/Nkentseu/Nkentseu
$ jenga build --target FermerProprement --config Release
$ ./Build/Bin/Release-Windows/FermerProprement/FermerProprement.exe


25/25 projets compilés et liés sans erreur après correction des lambdas.

## Tests des deux chemins de sortie

- **Touche Échap** : la fenêtre se ferme — conforme à l'attente.
- **Clic sur la croix** : la fenêtre se ferme également — conforme à l'attente.

Capture : [`capture-fermer_proprement.png`](capture-fermer_proprement.png).

## Pourquoi les deux chemins doivent aboutir au même endroit

La fermeture d'une fenêtre n'est pas qu'un détail visuel : c'est le signal
qui devrait déclencher tout le nettoyage de l'application (libération des
ressources graphiques, sauvegarde d'un état, arrêt propre de threads, etc.).
Si le clic sur la croix et la touche Échap avaient chacun leur propre chemin
de sortie (par exemple, un `return` direct dans le rappel du clavier, et un
`break` séparé pour l'événement de fermeture), on se retrouverait avec deux
implémentations de la même logique de fin de programme, à maintenir en
parallèle. Le jour où l'on ajoute une étape de nettoyage (sauvegarder la
scène avant de quitter, par exemple), il faudrait penser à la dupliquer aux
deux endroits — avec le risque réel d'oublier l'un des deux, de les laisser
diverger avec le temps, ou de laisser l'application dans un état
incohérent selon la façon dont l'utilisateur a choisi de quitter.

En faisant converger les deux rappels vers un seul booléen (`enCours`), qui
est l'unique condition de la boucle principale, il n'existe plus qu'un seul
point de sortie : la fin naturelle de `while (enCours)`. Tout ce qui doit se
passer à la fermeture — que ce soit du code de nettoyage ajouté plus tard,
ou simplement le `return 0;` final — s'exécute exactement une fois, de la
même manière, quel que soit le déclencheur. C'est le principe d'une seule
source de vérité (« single point of exit ») appliqué au cycle de vie de
l'application : plusieurs façons de déclencher un événement, un seul chemin
pour le traiter.
