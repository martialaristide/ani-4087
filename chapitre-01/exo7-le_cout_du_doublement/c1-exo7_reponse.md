# Exercice 7 : Le coût du doublement

**Nom :** Martial Aristide Barra
**Filière :** Génie Informatique, Spécialisation IA, ÉNSP

## Énoncé

Reprendre le même programme et mesurer le temps que prend son rendu seul, sans la logique. Puis estimer ce que coûterait ce rendu fait deux fois, et ce qu'il resterait pour le reste. Rendre la mesure, l'estimation, et la conclusion sur ce qu'il faudrait réduire.

## Méthode

Le programme de l'exercice 6 a été repris et modifié pour chronométrer séparément deux temps à chaque image, sur les mêmes 1000 images :

- **T_rendu** : `glClear()` + dessin du graphique en barres + `glfwSwapBuffers()`, soit la partie qui, en VR, s'exécuterait deux fois (une fois par œil).
- **T_logique** : `glfwPollEvents()` et la mise à jour des tableaux de mesure, soit la partie qui ne se double pas en VR.

## Mesures obtenues

| Mesure | Valeur |
|---|---|
| T_rendu moyen | 0,4965 ms |
| T_logique moyen | 0,0169 ms |
| Coût du rendu doublé (2 x T_rendu) | 0,9930 ms |
| Reste dans le budget de 11,1 ms | 10,0901 ms |

Calcul du reste : reste = 11,1 - 0,9930 - 0,0169
reste = 10,0901 ms 


## Ce que ces chiffres montrent, et leur limite

Sur ces mesures, le rendu de ce programme est très bon marché : même doublé pour les deux yeux, il ne consomme qu'environ 1 ms, laissant plus de 10 ms de marge dans le budget de 11,1 ms. En apparence, ce programme tiendrait donc largement une cadence de 90 Hz, même en dessinant deux fois par image.

Mais cette conclusion, prise seule, tomberait exactement dans le piège que le chapitre 1 dénonce explicitement : elle repose sur une **moyenne**, alors que le chapitre insiste sur le fait qu'une expérience VR se juge à sa pire image, pas à sa moyenne. Or l'exercice précédent (exercice 6), sur ce même programme non modifié, a mesuré une pire image à 23,906 ms sur 1000, avec une seule image dépassant 11 ms. Cette moyenne de rendu très faible (0,4965 ms) ne contredit pas ce résultat : elle montre simplement qu'un pic isolé peut coexister avec une moyenne excellente, précisément parce qu'une moyenne dilue un seul événement rare dans 999 autres qui se sont bien passés.

## Conclusion : qu'est-ce qu'il faudrait réduire ?

Sur la base de ces chiffres, ce n'est pas le coût moyen du rendu ni celui de la logique qu'il faudrait réduire en priorité : les deux sont déjà largement dans le budget, même doublés. Ce qu'il faudrait plutôt identifier et corriger, c'est la cause du pic isolé observé à l'exercice 6, probablement lié à un coût d'initialisation du contexte graphique en tout début d'exécution plutôt qu'à un problème récurrent de rendu ou de logique.

Autrement dit, réduire le temps moyen de rendu ou de logique n'apporterait presque rien ici : la marge est déjà confortable. Le vrai travail de réduction, pour ce programme précis, porterait sur l'élimination ou le déplacement hors de la boucle principale de ce coût d'initialisation ponctuel, puisque c'est lui, et non le rendu récurrent, qui a fait échouer le budget à l'exercice 6.
