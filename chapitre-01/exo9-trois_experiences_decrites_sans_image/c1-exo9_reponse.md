# Exercice 9 : Trois expériences décrites sans image

**Nom :** Martial Aristide Barra
**Filière :** Génie Informatique, Spécialisation IA, ÉNSP

## Énoncé

Choisir trois applications de réalité virtuelle existantes, de genres différents. Pour chacune, décrire en dix lignes ce qu'elle demande à l'utilisateur de faire de son corps. Classer ces trois expériences de la moins à la plus susceptible de rendre malade, et justifier.

## Les trois applications choisies

### 1. Beat Saber (jeu de rythme)

Beat Saber demande à l'utilisateur de rester debout, à un seul endroit, sans jamais se déplacer dans l'espace virtuel. Le joueur tient deux contrôleurs représentant des sabres, et doit trancher des blocs colorés qui foncent vers lui en rythme avec la musique. Le corps reste globalement fixe : seuls les bras, les poignets, le torse et parfois les genoux (pour esquiver certains obstacles) bougent réellement. La tête peut légèrement se pencher ou s'incliner pour éviter des murs qui approchent, mais il n'y a aucun déplacement du point de vue généré artificiellement par le jeu : la caméra ne bouge jamais toute seule. Tout ce que l'œil voit bouger correspond directement à un mouvement réel de la tête ou du corps du joueur. Il n'y a ni marche, ni téléportation, ni glissement continu du décor.

### 2. Half-Life: Alyx (jeu narratif à la première personne)

Half-Life: Alyx demande à l'utilisateur de rester debout ou assis à un endroit fixe de la pièce, avec la possibilité d'un espace room-scale optionnel pour se pencher physiquement ou éviter de petits obstacles. Le joueur se déplace dans le monde virtuel de deux façons principales : par téléportation (l'option par défaut, où l'on pointe un endroit et l'on y est instantanément transporté avec un bref fondu), ou par déplacement continu au joystick, où le décor défile devant les yeux du joueur comme dans un jeu de tir classique sur écran, sans que le corps ne bouge réellement. Les mains, via les contrôleurs, servent à attraper des objets, recharger des armes et interagir avec l'environnement à portée de bras. La tête et le buste peuvent physiquement s'incliner pour se cacher derrière un obstacle. C'est le mode de déplacement continu, en particulier, qui déconnecte le mouvement vu par les yeux du mouvement réellement ressenti par le corps.

### 3. Job Simulator (simulation comique)

Job Simulator demande à l'utilisateur de se tenir debout dans un espace room-scale réel (au moins 2 m sur 1,5 à 2 m recommandés par l'éditeur), et d'utiliser ce vrai espace physique pour se déplacer, se pencher, s'accroupir et tendre les bras vers les objets. Le jeu suit fidèlement les mouvements réels du joueur avec un suivi 1:1 des mains via les contrôleurs, pour attraper, lancer et manipuler des objets du quotidien dans un décor de bureau, de cuisine ou de magasin. Lorsque l'espace physique disponible est trop petit pour atteindre un point du décor, une téléportation ponctuelle permet de se repositionner, mais l'essentiel du jeu repose sur le fait de vraiment marcher et se pencher dans la pièce réelle pour atteindre les objets virtuels, sans déplacement artificiel continu du point de vue.

## Classement, de la moins à la plus susceptible de rendre malade

1. **Beat Saber** (la moins susceptible)
2. **Job Simulator**
3. **Half-Life: Alyx** (la plus susceptible)

## Justification

Le critère qui explique ce classement est directement celui du chapitre 1 : le malaise vient du conflit entre ce que l'oreille interne ressent (le corps est immobile) et ce que les yeux voient (le monde semble bouger). Le classement suit exactement l'ampleur de ce conflit potentiel dans chaque application.

**Beat Saber** ne présente aucun déplacement du point de vue généré artificiellement : le joueur reste à un seul endroit, et tout mouvement visuel correspond à un vrai mouvement de sa tête ou de son corps. Il n'y a donc, par construction, aucune source du conflit décrit au chapitre 1.

**Job Simulator** repose sur un principe similaire : le déplacement se fait par le vrai corps dans le vrai espace de la pièce (room-scale), donc ce que les yeux voient bouger correspond, l'essentiel du temps, à un vrai mouvement physique. La seule source de conflit possible vient des téléportations ponctuelles quand l'espace réel manque, mais celles-ci sont des sauts instantanés avec fondu, une technique reconnue pour limiter fortement le malaise, puisqu'elle supprime la phase de "mouvement visuel continu sans mouvement réel" plutôt que de la simuler.

**Half-Life: Alyx** est la seule des trois à proposer, en option, un déplacement continu au joystick : dans ce mode, le décor défile devant les yeux du joueur comme s'il marchait, alors que son corps reste immobile dans le salon. C'est exactement le scénario que le chapitre 1 identifie comme déclencheur du mal des transports en VR, et c'est un problème suffisamment documenté et reconnu par les développeurs eux-mêmes pour qu'ils aient ajouté, après la sortie du jeu, des options de confort supplémentaires (vitesse de rotation réglable, rotation par paliers) spécifiquement pour l'atténuer. Même si le mode de téléportation par défaut du jeu limite ce risque pour la majorité des joueurs, la possibilité même du déplacement continu place cette application au-dessus des deux autres sur ce critère.
