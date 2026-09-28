# Chapitre 3 — Exercice 9 : le défaut corrigé

## Le code

```cpp
// Accumulateur : chaque NkMouseRawEvent recu pendant l'image ajoute son
// delta au total en attente.
int32 accumulateurX = 0;
NkEvents().AddEventCallback<NkMouseRawEvent>([&accumulateurX](NkMouseRawEvent *e) {
        accumulateurX += e->GetDeltaX();
});

while (enCours) {
        NkEvents().PollEvents();

        // Consommation : on prend le total accumule pendant cette image,
        // puis on remet l'accumulateur a zero pour l'image suivante.
        int32 deltaXConsomme = accumulateurX;
        accumulateurX = 0;

        std::printf("etat (defectueux) = %-5d   accumulateur (corrige) = %d\n",
                                NkInput.MouseRawDeltaX(), deltaXConsomme);
}
```

Fichier complet : [`00-LeDefautCorrige/main.cpp`](00-LeDefautCorrige/main.cpp).

## Test : même manipulation que l'exercice 8 (bouger puis immobiliser la main)

Les deux séries côte à côte, pendant le mouvement puis après l'arrêt complet :

etat (defectueux) = 2 accumulateur (corrige) = 0
etat (defectueux) = -2 accumulateur (corrige) = -2
etat (defectueux) = -2 accumulateur (corrige) = 0
etat (defectueux) = -2 accumulateur (corrige) = 0
etat (defectueux) = -4 accumulateur (corrige) = -4
etat (defectueux) = -4 accumulateur (corrige) = 0
etat (defectueux) = -8 accumulateur (corrige) = -8
etat (defectueux) = -8 accumulateur (corrige) = 0
...
etat (defectueux) = -16 accumulateur (corrige) = -16 <- pic du mouvement
etat (defectueux) = -16 accumulateur (corrige) = 0
etat (defectueux) = -15 accumulateur (corrige) = -15
...
etat (defectueux) = -2 accumulateur (corrige) = -2 <- derniere micro-secousse
etat (defectueux) = -2 accumulateur (corrige) = 0
etat (defectueux) = -2 accumulateur (corrige) = 0 <- main immobile a partir d'ici
etat (defectueux) = -2 accumulateur (corrige) = 0
etat (defectueux) = -2 accumulateur (corrige) = 0
... (des centaines de lignes identiques, main parfaitement immobile)
etat (defectueux) = -2 accumulateur (corrige) = 0


## Ce que ça change

Dès que la main s'arrête, la colonne « état (défectueux) » se fige à sa
dernière valeur non nulle (`-2` ici) et n'en bouge plus jamais — c'est
exactement le défaut de l'exercice 8. La colonne « accumulateur (corrigé) »,
elle, revient à `0` dès l'image suivante et y reste tant qu'aucun nouvel
`NkMouseRawEvent` n'arrive.

Le mécanisme est simple : le callback n'ajoute au total que ce qui a
réellement été *reçu* comme nouvel événement pendant l'image — s'il n'y a
aucun mouvement physique, aucun `NkMouseRawEvent` n'est émis, donc rien n'est
ajouté, et la valeur consommée (puis remise à zéro) est naturellement `0`.
Contrairement à l'état interrogé (`NkInput.MouseRawDeltaX()`), qui garde en
mémoire la dernière valeur connue sans se soucier de savoir si elle est
encore d'actualité, l'accumulateur ne peut représenter que ce qui s'est
passé *entre deux consommations* — jamais plus, jamais moins, et jamais un
souvenir périmé du passé. C'est le patron correct pour tout delta destiné à
piloter un mouvement (caméra, visée) : accumuler par événement, consommer et
remettre à zéro à chaque image.
