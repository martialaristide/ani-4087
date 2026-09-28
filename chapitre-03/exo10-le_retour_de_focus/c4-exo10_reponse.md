# Exercice 3.10 — Le retour de focus

## Objectif

Reprendre le pattern accumulateur-et-consommateur de l'exercice 9, puis retirer volontairement la protection qui remet l'accumulateur à zéro pendant que la fenêtre n'a pas le focus, pour observer concrètement le défaut de moteur décrit au chapitre 4 :

> « Un accumulateur que personne ne vide fait dériver la tête. C'est un défaut réel du moteur : la variable qui porte le dernier déplacement brut n'est jamais remise à zéro, et l'intégrer à chaque image applique indéfiniment le même mouvement, même souris immobile. Le contournement est d'accumuler soi-même, et d'écrire pourquoi à l'endroit du contournement : un contournement sans commentaire ressemble à une complication inutile, et quelqu'un finira par le supprimer. »

## Principe du code

Un interrupteur de compilation contrôle le comportement :

```cpp
// true  : la remise a zero de l'accumulateur est SUSPENDUE tant que la fenetre
//         n'a pas le focus -> reproduit le defaut du chapitre.
// false : la remise a zero reste active en permanence, focus ou non (version
//         corrigee).
static constexpr bool SUSPENDRE_RESET_SANS_FOCUS = true; // ou false
```

La boucle principale :

```cpp
int32 accumulateurX = 0;
NkEvents().AddEventCallback<NkMouseRawEvent>([&accumulateurX](NkMouseRawEvent *e) {
        accumulateurX += e->GetDeltaX();
});

while (enCours) {
        NkEvents().PollEvents();

        if (SUSPENDRE_RESET_SANS_FOCUS && !aLeFocus) {
                // L'accumulateur continue de grossir en silence, sans etre
                // ni affiche ni vide, tant que la fenetre n'a pas le focus.
                continue;
        }

        int32 deltaXConsomme = accumulateurX;
        accumulateurX = 0;

        std::printf("focus=%d   deltaXConsomme = %d\n", aLeFocus ? 1 : 0, deltaXConsomme);
}
```

Deux callbacks marquent les changements de focus (`NkWindowFocusGainedEvent` / `NkWindowFocusLostEvent`), affichés respectivement `>>> FOCUS REGAGNE <<<` et `>>> FOCUS PERDU <<<`.

Important : la callback d'accumulation (`AddEventCallback<NkMouseRawEvent>`) tourne **sans condition** — elle continue de sommer les deltas même sans focus. Seule l'étape de consommation/affichage/reset est court-circuitée par le `continue` quand `SUSPENDRE_RESET_SANS_FOCUS` est actif et que la fenêtre n'a pas le focus.

## Protocole de test (identique pour les deux variantes)

1. Lancer le programme, fenêtre "Ma salle" focalisée.
2. Cliquer ailleurs (perte de focus) → `>>> FOCUS PERDU <<<`.
3. Bouger la souris pendant environ 10 secondes, sans recliquer.
4. Recliquer sur la fenêtre → `>>> FOCUS REGAGNE <<<`.
5. Observer les lignes suivantes, puis fermer (Echap).

## Résultat — variante « avant » (`SUSPENDRE_RESET_SANS_FOCUS = true`)

Fonctionnement normal tant que la fenêtre a le focus (petites valeurs, souvent 0) :

focus=1 deltaXConsomme = 0
focus=1 deltaXConsomme = -2
...


Perte de focus, silence total pendant ~10 secondes (aucune ligne imprimée, mais l'accumulateur continue de sommer en coulisse) :

FOCUS PERDU <
FOCUS PERDU <


Retour du focus — la toute première image après `FOCUS REGAGNE` déverse d'un coup tout ce qui s'est accumulé pendant l'absence de focus :

FOCUS REGAGNE <
FOCUS REGAGNE <
focus=1 deltaXConsomme = -357
focus=1 deltaXConsomme = 0
focus=1 deltaXConsomme = 0
...


Puis le comportement redevient normal (petites valeurs par image).

**Diagnostic** : pendant la perte de focus, la variable qui porte le déplacement brut de la souris (ici notre propre accumulateur, qui reproduit le rôle de la variable interne du moteur décrite au chapitre) n'est jamais consommée ni remise à zéro. Le mouvement de souris pendant ces 10 secondes ne « disparaît » pas : il s'accumule silencieusement, puis se déverse en un seul saut de -357 dès la première image où la consommation reprend. C'est exactement la mécanique du défaut décrit dans le cours, sauf qu'ici elle se manifeste comme un saut brutal ponctuel plutôt qu'une dérive continue, car l'accumulateur applicatif — contrairement à la variable interne du moteur qui, elle, se réécrit en continu même sans être lue — n'est alimenté que par les événements `NkMouseRawEvent` réellement reçus.

## Résultat — variante « après » (`SUSPENDRE_RESET_SANS_FOCUS = false`)

Même protocole exact, remise à zéro restaurée en permanence (focus ou non) :

focus=1 deltaXConsomme = 0
focus=1 deltaXConsomme = -2
...

FOCUS PERDU <
FOCUS PERDU <
focus=0 deltaXConsomme = 0
focus=0 deltaXConsomme = 0
focus=0 deltaXConsomme = -1
focus=0 deltaXConsomme = -7
focus=0 deltaXConsomme = -16
focus=0 deltaXConsomme = -23
...
focus=0 deltaXConsomme = 0
focus=0 deltaXConsomme = 0


**Aucun saut au retour du focus** : même après une dizaine de secondes de mouvements de souris parfois amples (`-23`, `-29`, `-33`...), chaque image consomme et vide l'accumulateur immédiatement, focus ou pas. Rien ne s'accumule d'une image à l'autre au-delà de ce qui vient d'arriver. Le retour du focus se fait donc en douceur, sans dump ni pic.

## Conclusion

Le test confirme empiriquement le défaut décrit au chapitre 4 : un accumulateur qui n'est pas vidé à chaque image continue de grossir tant qu'il reçoit des événements, même quand personne n'observe ni ne consomme sa valeur — ce qui produit soit une dérive continue (variable interne du moteur), soit un saut brutal différé (accumulateur applicatif alimenté par événements). Le contournement correct, comme indiqué dans le cours, est de vider l'accumulateur à *chaque* image sans condition — jamais de le suspendre selon un état externe (focus, pause, etc.) — et de documenter pourquoi ce vidage systématique existe, pour qu'il ne soit pas supprimé par erreur plus tard en le prenant pour du code mort.
