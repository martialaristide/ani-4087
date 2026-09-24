# Demo5 — Du bureau à la main

## Précision honnête
Cette démo suppose une salle de classe avec quelqu'un pour tenir l'appareil pendant que je
tape la commande — je travaille seul chez moi, donc personne d'autre n'a tenu le téléphone.
Le pipeline complet (construire → empaqueter → signer → installer) a néanmoins été exécuté
et commenté étape par étape, en ciblant explicitement le téléphone par son numéro de série
pour éviter toute ambiguïté avec l'émulateur également branché.

## Étape 1/4 — Construire
Compile `main.cpp` en bibliothèque native `.so` pour l'ABI `arm64-v8a` (celle du téléphone).
```bash
jenga clean --platform Android-arm64 --config Release
jenga build --platform Android-arm64 --config Release --no-cache
```
```
Configuration: Release
Target:        Android arm64
Toolchain:     android-ndk
...
✓ Built: Build\Bin\Release-Android\Salle\libSalle.so
...
✓ APK generated: ...\Salle-Release.apk
```
Jenga produit déjà un `.apk` à cette étape, mais signé avec une clé de debug automatique
(pas la clé du projet) — c'est pour ça qu'il faut re-signer plus loin.

## Étape 2/4 — Empaqueter
Le projet utilise la STL partagée (`androidstl("c++_shared")`) : `libc++_shared.so` doit
être physiquement copiée dans l'APK, sous `lib/arm64-v8a/`, sinon l'app plante au lancement
(`dlopen failed: library "libc++_shared.so" not found` — vu dans l'exo17). Injection
manuelle du fichier dans l'archive APK (au format ZIP) :
```bash
cp <NDK>/.../libc++_shared.so lib/arm64-v8a/
python -c "... injecte lib/arm64-v8a/libc++_shared.so dans Salle-Release.apk ..."
```
Code retour : `0`.

## Étape 3/4 — Signer
Deux sous-étapes obligatoires avant qu'Android accepte l'APK :
- `zipalign -p 4` : aligne les entrées de l'archive sur 4 octets — requis pour
  `resources.arsc` depuis Android 11, sinon `INSTALL_FAILED` (vu dans l'exo26).
- `apksigner sign` : signe l'APK avec la clé du projet (`ani4087_v3.jks`), v1+v2+v3.
```bash
zipalign.exe -f -p 4 Salle-Release.apk Salle-Release.apk.aligned
apksigner.bat sign --ks ... Salle-Release.apk
apksigner.bat verify Salle-Release.apk
```
```
zipalign termine, code: 0
Signature terminee, code: 0
Verification, code: 0
```

## Étape 4/4 — Installer
Installation ciblée par numéro de série (téléphone `EJL4C17401046978`), pour ne pas se
heurter à l'ambiguïté "more than one device/emulator" (vue à l'exo26) puisque l'émulateur
est aussi branché :
```bash
adb -s EJL4C17401046978 uninstall com.ani4087.exo16
adb -s EJL4C17401046978 install -r Salle-Release.apk
adb -s EJL4C17401046978 shell monkey -p com.ani4087.exo16 -c android.intent.category.LAUNCHER 1
```
```
Success
Performing Streamed Install
Success
```
L'écran du téléphone affiche bien la couleur unie de l'application "Salle" — le pipeline
complet, du code source à l'appareil physique, a fonctionné de bout en bout.
