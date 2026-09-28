# Chapitre 3 — Exercice 8 : le défaut reproduit

## Le code

```cpp
while (enCours) {
        NkEvents().PollEvents();

        std::printf("rawDeltaX = %d\n", NkInput.MouseRawDeltaX());
}
```

Aucune accumulation manuelle : la valeur affichée à chaque image est lue
directement, telle que le moteur la renvoie. Fichier complet :
[`00-LeDefautReproduit/main.cpp`](00-LeDefautReproduit/main.cpp).

## Test : bouger puis immobiliser la main

Vingt lignes consécutives, prélevées longtemps après l'arrêt complet du
mouvement de la souris (la main est restée parfaitement immobile pendant
plusieurs secondes avant ce prélèvement) :

rawDeltaX = -1
rawDeltaX = -1
rawDeltaX = -1
rawDeltaX = -1
rawDeltaX = -1
rawDeltaX = -1
rawDeltaX = -1
rawDeltaX = -1
rawDeltaX = -1
rawDeltaX = -1
rawDeltaX = -1
rawDeltaX = -1
rawDeltaX = -1
rawDeltaX = -1
rawDeltaX = -1
rawDeltaX = -1
rawDeltaX = -1
rawDeltaX = -1
rawDeltaX = -1
rawDeltaX = -1


Et ce n'est pas un cas isolé : sur l'ensemble de la capture (plus de 2000
images consécutives après l'arrêt de la main), la valeur reste figée à
`-1`, image après image, sans jamais retomber à `0`.

## Ce que ces lignes prouvent

Elles prouvent que `MouseRawDeltaX()` ne représente pas fidèlement « le
mouvement de la souris depuis la dernière image » : une fois qu'un dernier
delta non nul a été reçu, le moteur continue de le retourner indéfiniment à
chaque appel, au lieu de le remettre à `0` quand plus aucun nouveau
mouvement physique n'est détecté — c'est un défaut de rafraîchissement de
l'état brut de la souris, qui, si on l'utilisait tel quel pour piloter une
caméra, ferait tourner celle-ci en continu même après que le joueur a lâché
sa souris.
