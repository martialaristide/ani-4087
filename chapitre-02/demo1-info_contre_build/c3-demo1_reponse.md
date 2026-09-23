# Demo1 — Info contre build

## L'erreur volontaire
Dans `Salle.jenga`, la ligne :
```python
androidminsdk(24)
```
a été changée en :
```python
androidminskd(24)
```
(deux lettres inversées — fonction inexistante dans le DSL Jenga)

## Ce que dit `jenga build`
Loading workspace...

! Jenga workspace file: ...\Salle.jenga
This file will execute as unrestricted Python code (not a sandbox).
Trust and run it? [n]: y
Error loading workspace: name 'androidminskd' is not defined
Failed to load workspace.


## Ce que dit `jenga info`
Error loading workspace: name 'androidminskd' is not defined
Failed to load workspace.


## Laquelle des deux sorties désigne la cause ?
Les deux, **à égalité** — et c'est le point intéressant de cette démo. Le fichier
`Salle.jenga` est exécuté comme du code Python (le message le dit lui-même : "This file
will execute as unrestricted Python code"). Cette exécution se produit lors du **chargement
du workspace**, une étape partagée par toutes les commandes de Jenga, *avant* que la logique
propre à `build` ou à `info` ne démarre. Résultat : `androidminskd` n'existe pas comme
fonction Python valide, donc le chargement échoue avec la même `NameError` quelle que soit
la commande appelée ensuite — `build` et `info` ne font qu'hériter du même échec en amont,
ils ne le diagnostiquent pas différemment. Une erreur *dans le fichier de projet lui-même*
remonte donc identiquement partout ; une différence entre `build` et `info` n'apparaîtrait
que pour une erreur propre à l'étape de compilation ou de liaison (une erreur qui, elle,
ne se produit qu'à l'intérieur de `build`).
