# Exercice 26 — Choisir, et se tromper exprès

## 1. Déploiement sans cible, avec deux appareils branchés

Appareils connectés au moment du test :

$ adb devices
List of devices attached
EJL4C17401046978 device
emulator-5554 device


Commande lancée sans préciser d'appareil :
```bash
jenga deploy --platform android --config Release
```

Le build et l'empaquetage se déroulent normalement, mais l'installation échoue avec le
message exact suivant :

adb.exe: more than one device/emulator
adb install failed.


`jenga deploy` ne devine pas quel appareil cibler quand plusieurs sont branchés — il laisse
`adb` refuser, et `adb` refuse net plutôt que de choisir au hasard.

## 2. Installation ciblée sur chacun des deux, à tour de rôle

Les deux commandes ci-dessous ne diffèrent que par le numéro de série :

```bash
adb -s EJL4C17401046978 install -r Salle-Release.apk
```
```bash
adb -s emulator-5554 install -r Salle-Release.apk
```

### Remarque honnête : une limite que je n'ai pas pu contourner
L'énoncé demande de changer *seulement* le numéro de série. Dans les faits, je n'ai pas pu
utiliser le même fichier APK pour les deux appareils : mon téléphone est en `arm64-v8a`
(ABI native de son SoC), tandis que l'émulateur que j'ai dû utiliser tourne en `x86_64`
(mon processeur hôte ne supporte pas les instructions AVX qu'exige l'émulation ARM64 sur
QEMU2 — `FATAL | Avd's CPU Architecture 'arm64' is not supported by the QEMU2 emulator on
x86_64 host`). J'ai donc dû construire une variante x86_64 du projet
(`jenga build --platform Android-x86_64`) en plus de la variante arm64-v8a existante, et
c'est le chemin du fichier APK qui diffère entre les deux commandes ci-dessus (dans le
dossier de travail utilisé pour chacune), pas seulement le `-s`. C'est une contrainte
matérielle réelle, pas un choix : sur deux vrais téléphones de même architecture, une seule
commande avec seulement `-s <serial>` changeant aurait suffi.

Les deux commandes ont aussi été testées via `jenga deploy --apk <chemin> --target <serial>`
(le flag propre à l'outil du cours), mais `jenga deploy` reconstruit l'APK en interne à
chaque appel — même avec `--apk`, il écrase le fichier fourni par une version fraîchement
signée avec `debug.keystore` sans la bibliothèque `libc++_shared.so` injectée — ce qui casse
le lancement (`INSTALL_FAILED_UPDATE_INCOMPATIBLE`, puis
`UnsatisfiedLinkError: library "libc++_shared.so" not found`). J'ai donc utilisé `adb install`
directement sur l'APK déjà construit, empaqueté et signé (mêmes étapes que `deploy.sh` de
l'exo23), ce qui est fiable et correspond de toute façon à ce que `jenga deploy` fait en
coulisses à la toute dernière étape.

## 3. Vérification sur les deux écrans

Les deux captures ci-dessous montrent l'écran bleu (couleur de fond OpenGL du projet
minimal "Salle") affiché sur chacun des deux appareils après installation et lancement
(`adb shell monkey -p com.ani4087.exo16 -c android.intent.category.LAUNCHER 1`) :

- Téléphone (série `EJL4C17401046978`) : `telephone_ecran.png`
- Émulateur (série `emulator-5554`, AVD `Pixel_test_api30`, API 30 x86_64) : `emulateur_ecran.png`

L'app est bien arrivée et s'exécute au bon endroit sur les deux appareils.
