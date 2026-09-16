# Exercice 4 : Le décalage des deux yeux

**Nom :** Martial Aristide Barra
**Filière :** Génie Informatique, Spécialisation IA, ÉNSP

## Énoncé

Tenir un doigt à trente centimètres du visage et fermer alternativement un œil puis l'autre. Mesurer de combien le doigt semble se déplacer par rapport au mur du fond. Recommencer à un mètre, puis à trois mètres. Rendre les trois mesures, et dire ce qu'elles annoncent du travail du chapitre 9.

## Les mesures

| Distance du doigt | Décalage apparent observé |
|---|---|
| 30 cm | 5 cm |
| 1 m | 6 cm |
| 3 m | 5,5 cm |

## Analyse honnête des mesures

Le chapitre annonce qu'un objet proche se décale nettement d'un œil à l'autre, et qu'un objet lointain se décale presque pas. On s'attendrait donc à voir une diminution nette du décalage à mesure que la distance augmente : quelque chose comme 5 cm à 30 cm, puis beaucoup moins à 1 m, puis presque rien à 3 m.

Mes trois mesures ne suivent pas ce schéma de façon nette : elles restent proches les unes des autres (5, 6 puis 5,5 cm), sans la chute attendue. Plutôt que de forcer une conclusion qui collerait artificiellement à la théorie, je préfère le signaler tel quel, avec les explications les plus probables :

- **La méthode d'estimation à l'œil nu est imprécise.** Juger "de combien le doigt semble se déplacer" par rapport à un mur, sans instrument de mesure de l'angle, laisse une marge d'erreur importante, surtout à 3 mètres où le décalage réel est petit et donc difficile à distinguer d'une erreur d'appréciation.
- **La distance au mur du fond compte autant que la distance du doigt.** Le décalage apparent dépend de la position relative du doigt et de l'arrière-plan, pas seulement de la distance du doigt seule. Si le mur n'était pas à une distance suffisamment grande ou constante entre les trois essais, la comparaison entre les trois mesures perd une partie de sa validité.
- **À 3 mètres en intérieur, on approche vite les limites de la pièce.** Le mur du fond n'est peut-être plus assez loin pour que l'effet de perspective attendu se manifeste clairement.

Je ne cherche donc pas à maquiller ce résultat : mes chiffres montrent surtout la difficulté de mesurer ce phénomène à l'œil nu, plus qu'ils ne contredisent le principe physique lui-même (qui, lui, est bien établi et ne dépend pas de ma mesure).

## Ce que ces mesures annoncent du travail du chapitre 9

Même avec des mesures imparfaites, l'expérience touche à quelque chose qui devient central au chapitre 9 : le calcul de la matrice de projection propre à chaque œil, avec ses quatre angles séparés plutôt qu'un champ de vision symétrique classique.

Ce que le doigt donne à sentir, c'est que le décalage entre les deux points de vue n'est pas une valeur fixe : il dépend de la profondeur de l'objet regardé. Un objet proche crée un grand décalage, un objet lointain un petit décalage. Or c'est exactement ce que la matrice de projection de chaque œil doit reproduire mathématiquement pour chaque pixel affiché, en fonction de sa distance à la caméra, faute de quoi la perception de profondeur du casque serait fausse.

Si un jour le module calcule les deux vues avec un champ de vision symétrique, ou sans tenir compte correctement de l'écart interpupillaire, ce n'est pas un plantage qu'on obtiendrait, mais exactement le genre de défaut déjà repéré au chapitre 1 : un monde qui paraît à la mauvaise échelle, sans qu'aucune erreur ne s'affiche. L'exercice du doigt est donc une façon physique, avec son propre corps, de ressentir à l'avance ce que le chapitre 9 devra traduire en mathématiques.
