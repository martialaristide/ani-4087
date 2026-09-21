# Exercice 15 — Votre première clé

## Commande employée (sans le mot de passe)

```bash
mkdir -p /c/Users/Lenovo/cles-signature/ani-4087
jenga keygen --interactive --alias ani4087key --validity 10000 --output /c/Users/Lenovo/cles-signature/ani-4087/ani4087.jks
```

Sortie réelle (le mot de passe ne s'affiche jamais, c'est le comportement
normal d'une invite cachée) :

Generate Android Keystore

Key alias [ani4087key]: ani4087key
Validity (days) [10000]:
Distinguished Name (CN=Name, OU=Org, O=Company, L=City, ST=State, C=Country) [CN=Jenga User]: CN=Martial Aristide Barra, OU=ENSP, O=ANI-4087, L=Yaounde, ST=Centre, C=CM
Keystore password:
Key password (default: same as keystore):
Generating keystore...
Keystore generated: C:\Users\Lenovo\cles-signature\ani-4087\ani4087.jks
Alias: ani4087key


## Un piège trouvé avant de lancer la commande

En lisant le code source de Jenga (`Jenga/Commands/Keygen.py`), j'ai
découvert que `jenga keygen` **sans** l'option `--interactive` ne demande
aucun mot de passe : il utilise en silence le mot de passe codé en dur
`"android"` (le même que le keystore de debug Android par défaut) pour
le keystore ET pour la clé. Une clé générée ainsi serait signée avec un
mot de passe public et connu de tous. J'ai donc systématiquement utilisé
`--interactive`, seule option qui déclenche une vraie invite de mot de
passe masquée.

## Emplacement du fichier de clé

`C:\Users\Lenovo\cles-signature\ani-4087\ani4087.jks` — un dossier situé
en dehors du dépôt `ani-4087`, donc jamais suivi par git, jamais poussé
sur GitHub.

## Où le mot de passe est rangé

Dans un fichier texte séparé, dans ce même dossier hors dépôt :
`C:\Users\Lenovo\cles-signature\ani-4087\mot_de_passe.txt` — ni le fichier
`.jks`, ni ce fichier de mot de passe ne se trouvent nulle part sous
`ani-4087`, donc aucun des deux ne peut être poussé par erreur sur un
dépôt public.
