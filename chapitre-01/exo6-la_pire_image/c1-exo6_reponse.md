# Exercice 6 : La pire image

**Nom :** Martial Aristide Barra
**Filière :** Génie Informatique, Spécialisation IA, ÉNSP

## Énoncé

Prendre un programme qui dessine en boucle, ou en écrire un qui ne fait qu'effacer l'écran. Mesurer, sur mille images, non pas la cadence moyenne mais la durée de la plus longue image, et le nombre d'images qui dépassent onze millisecondes. Rendre les deux chiffres, et dire si le programme tiendrait dans un casque.

## Méthode

Le programme est écrit en C++, avec une vraie fenêtre et un vrai contexte graphique (GLFW + OpenGL), plutôt qu'une simple boucle en console. Ce choix est volontaire : une boucle console ne mesurerait que le temps CPU, en ignorant tout ce que le chapitre 1 identifie comme faisant partie du budget réel d'une image (appel au GPU, effacement de l'écran, échange de buffer). Le VSync est désactivé pour mesurer le vrai coût de la boucle, et non le taux de rafraîchissement de l'écran.

Chaque image est chronométrée individuellement avec `std::chrono::high_resolution_clock`, sur 1000 images consécutives. Un graphique en barres affiche en direct les 200 dernières images, avec une ligne de seuil à 11 ms (vert en dessous, rouge au-dessus), pour repérer visuellement les pics pendant l'exécution.

## Résultats mesurés

| Mesure | Valeur |
|---|---|
| Pire image (sur 1000) | **23,906 ms** |
| Images au-dessus de 11 ms | **1 / 1000** |

## Ce que ces chiffres disent

Sur les 1000 images mesurées, une seule dépasse le seuil de 11 ms, avec un pic à 23,906 ms, soit plus du double du budget visé à 90 Hz (11,1 ms). Les 999 autres images restent sous ce seuil.

Une hypothèse probable, cohérente avec ce qui a été observé pendant le développement du programme, est que ce pic isolé corresponde à l'initialisation du contexte graphique (création de la fenêtre, premier échange de buffer avec le driver GPU), qui coûte typiquement plus cher que les images suivantes une fois la boucle stabilisée. Je ne peux cependant pas l'affirmer avec certitude sans avoir instrumenté le programme pour savoir à quel numéro d'image précis ce pic est survenu : c'est une explication plausible, pas une certitude établie par la mesure elle-même.

## Le programme tiendrait-il dans un casque ?

**Non, pas tel quel, en suivant strictement la règle du chapitre 1.** Le chapitre est explicite sur ce point : une expérience en casque se juge à sa pire image, pas à sa moyenne, précisément parce qu'une seule image qui dépasse largement le budget se voit ou se sent, même si elle est rare. Ici, 999 images sur 1000 respectent le budget, mais la règle du chapitre ne laisse pas de marge pour "une seule exception" : un seul dépassement suffit, par définition, à invalider la tenue du budget à chaque image.

Cela dit, la nuance mérite d'être posée honnêtement plutôt que balayée : si ce pic est bien un coût d'initialisation ponctuel, survenant avant que l'utilisateur ne soit réellement plongé dans l'expérience (par exemple pendant un écran de chargement), son impact réel sur le confort ressenti ne serait pas le même qu'un pic survenant au hasard pendant que l'utilisateur bouge activement la tête. Le chapitre ne fait cependant pas cette distinction dans sa règle de base, et je préfère donc répondre strictement à la question posée : en l'état, mesuré tel quel sur ses 1000 images, ce programme ne respecte pas le budget à chaque image, et ne tiendrait donc pas dans un casque sans investigation et correction supplémentaires.
