# Demo 7 — Le fichier qui n'existe que chez vous

## Le programme

On reprend exactement le même code que dans l'exercice 16 : une ouverture de
`assets/donnees.txt` par deux méthodes différentes, l'une juste après l'autre.

```cpp
FILE *f = fopen("assets/donnees.txt", "r");
if (f) {
    // ... lecture ...
    printf("fopen a REUSSI, contenu: %s", buffer);
} else {
    printf("fopen a ECHOUE (errno=%d: %s)", errno, strerror(errno));
}

AAssetManager *assetManager = app->activity->assetManager;
AAsset *asset = AAssetManager_open(assetManager, "donnees.txt", AASSET_MODE_BUFFER);
if (asset) {
    // ... lecture ...
    printf("AAssetManager a REUSSI, contenu: %s", buffer2);
} else {
    printf("AAssetManager a ECHOUE a ouvrir le fichier");
}
```

Sur PC, seule la partie `fopen()` a un sens (pas d'`AAssetManager` en dehors
d'Android) : on compile une version isolée de ce test, `test_fopen_pc.cpp`,
exécutée depuis un dossier qui contient un `assets/donnees.txt` normal.

## Résultat sur ma machine (PC)

$ ./test_fopen_pc
Tentative d'ouverture de "assets/donnees.txt" avec fopen()...
fopen() a REUSSI !
Contenu lu : Bonjour depuis le fichier de ressources !


`fopen()` trouve le fichier sans problème : sur le PC, `assets/donnees.txt`
est un fichier ordinaire du système de fichiers, situé au bon endroit
relativement au dossier d'exécution du programme.

## Résultat sur l'appareil (téléphone Android)

Même code, cette fois exécuté dans l'app native installée sur le téléphone
(logcat filtré sur le tag `ExoLePaquetVide`) :

09-24 09:32:26.487 I ExoLePaquetVide: Ligne 1 : demarrage de l'application
09-24 09:32:26.487 I ExoLePaquetVide: Ligne 2 : fenetre native prete
09-24 09:32:26.487 I ExoLePaquetVide: Ligne 3 : boucle de rendu lancee
09-24 09:32:26.487 E ExoLePaquetVide: fopen a ECHOUE (errno=2: No such file or directory)
09-24 09:32:26.487 I ExoLePaquetVide: AAssetManager a REUSSI, contenu: Bonjour depuis le fichier de ressources !


Cette fois, `fopen("assets/donnees.txt", "r")` échoue avec `errno=2`
(`ENOENT`, "No such file or directory") — exactement comme si le fichier
n'existait pas.

## À vous de deviner

Avant de lire la suite : le fichier `donnees.txt` est bien présent, embarqué
dans l'APK installé sur le téléphone (l'`AAssetManager`, juste après, le
prouve en le lisant avec succès). Alors pourquoi `fopen()` — qui a
fonctionné sans problème sur le PC, avec le même chemin relatif
`"assets/donnees.txt"` — échoue-t-il ici avec « fichier introuvable » ?

*(Prenez un instant pour formuler une hypothèse avant de continuer.)*

## Explication

`fopen()` est une fonction de la libc standard : elle cherche un fichier
**réel, sur le système de fichiers**, à un chemin donné — ici, relatif au
répertoire de travail courant du processus. Sur un PC, quand on exécute le
programme depuis le dossier du projet, ce chemin relatif pointe bien vers un
fichier `assets/donnees.txt` qui existe vraiment sur le disque.

Sur Android, un APK installé n'est **pas décompressé sur le disque** comme
un dossier de projet. C'est une archive ZIP, restée telle quelle, dont le
contenu (bibliothèque native, ressources, manifeste...) est lu directement
depuis l'archive par le système. Le processus de l'application n'a pas de
répertoire de travail qui contiendrait un dossier `assets/` avec de vrais
fichiers dedans — il n'y a tout simplement rien à cet endroit du système de
fichiers du téléphone. D'où l'`ENOENT` : ce n'est pas que le fichier
"n'existe pas" au sens absolu, c'est que `fopen()` regarde au mauvais
endroit — un chemin sur le système de fichiers, alors que la ressource vit
ailleurs.

C'est précisément pour ça qu'Android fournit une API séparée,
`AAssetManager`, spécialement conçue pour lire les ressources **empaquetées
dans l'APK** : elle sait aller chercher les entrées du dossier `assets/`
directement dans l'archive ZIP (potentiellement encore compressée), sans
jamais passer par le système de fichiers classique. C'est pour ça que
`AAssetManager_open(assetManager, "donnees.txt", ...)` réussit juste après,
avec le même contenu, alors que `fopen()` a échoué une ligne plus haut.

**En résumé :** sur PC, les ressources d'un projet sont de vrais fichiers,
accessibles par n'importe quelle fonction d'E/S standard. Dans un paquet
Android, elles sont des entrées d'archive, et seule l'API dédiée
(`AAssetManager`) sait les en extraire — `fopen()` n'a tout simplement pas
la bonne vue sur le système de fichiers de l'appareil.
