# Exercice 8 : Le champ de vision asymétrique

**Nom :** Martial Aristide Barra
**Filière :** Génie Informatique, Spécialisation IA, ÉNSP

## Énoncé

Chercher les quatre angles du champ de vision d'un casque du commerce, pour l'œil gauche, avec la source. Puis dire, en une phrase, ce qui se passerait si l'on employait à la place un champ symétrique de même surface.

## Les quatre angles trouvés

Casque : **Valve Index**, œil gauche, valeurs OpenXR brutes (structure `XrFovf`, en radians dans la source, converties ici en degrés) :

| Angle | Valeur (radians) | Valeur (degrés) |
|---|---|---|
| angleLeft (vers la tempe) | −0,994990 rad | −57,01° |
| angleRight (vers le nez) | 0,810479 rad | 46,44° |
| angleUp | 0,955361 rad | 54,74° |
| angleDown | −0,952498 rad | −54,57° |

Champ de vision horizontal total pour cet œil : 46,44° − (−57,01°) = 103,45°.
Champ de vision vertical total : 54,74° − (−54,57°) = 109,31°.

## Note honnête sur la source

Ces valeurs ne viennent pas d'une fiche technique officielle Valve (qui ne publie, comme la plupart des fabricants, qu'un seul chiffre global de champ de vision, environ 130° dans la documentation marketing, sans détailler les quatre angles séparés). Elles viennent d'un développeur ayant lu ces valeurs directement depuis le runtime OpenXR de son propre Valve Index et les ayant partagées sur le forum officiel des développeurs Khronos, en réponse à une question technique sur ce même sujet.

C'est donc une source secondaire (une mesure d'utilisateur, pas une fiche constructeur), mais elle a l'avantage d'être une valeur réellement lue depuis le matériel via l'API standard du secteur (OpenXR), plutôt qu'une estimation. Je le signale clairement plutôt que de la présenter comme une donnée officielle Valve.

**Source :** forum des développeurs Khronos, discussion « Do these XrFovf values look suspicious/defective to you? », community.khronos.org/t/do-these-xrfovf-values-look-suspicious-defective-to-you

## Vérification de la cohérence avec le chapitre

La valeur absolue de angleLeft (57,01°, côté tempe) est nettement plus grande que celle de angleRight (46,44°, côté nez). C'est exactement ce que décrit le chapitre : l'œil gauche voit davantage vers l'extérieur (la tempe) que vers l'intérieur (le nez), parce que le nez bloque une partie du champ côté intérieur. Cette asymétrie mesurée confirme concrètement l'affirmation théorique du chapitre.

## Ce qui se passerait avec un champ symétrique de même surface

Employer un champ symétrique de même surface centrerait artificiellement l'image sur l'axe optique de la lentille plutôt que sur le centre réel de l'œil, ce qui couperait une partie du champ réellement visible du côté le plus large (la tempe) tout en gaspillant du budget de rendu sur des pixels du côté le plus étroit (le nez) que l'œil ne peut de toute façon pas voir à cause du nez lui-même.
