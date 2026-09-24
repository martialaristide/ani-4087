
# Demo 9 — Trois appareils, une commande

Démo faite avec deux appareils connectés : un téléphone physique et un
émulateur (pas de vrai casque VR disponible pour cette session, le
téléphone en tient lieu et sera désigné comme "le casque").

## 1. La liste détaillée

Jenga a sa propre commande de listing, plus riche que `adb devices` brut :
$ jenga deploy --platform android --list-devices --detailed

SERIAL MARQUE MODELE ANDROID ABI

EJL4C17401046978 HUAWEI HUAWEI MLA-L11 7.0 arm64-v8a
emulator-5554 unknown Android SDK built for x86_64 11 x86_64


Contrairement à `adb devices` qui ne donne qu'un numéro de série et un
état, Jenga interroge chaque appareil (marque, modèle, version Android,
ABI) — c'est cette liste qu'on montre à la classe.

## 2. La classe désigne le casque

Le téléphone (`EJL4C17401046978`, HUAWEI MLA-L11, arm64-v8a) est désigné
comme "le casque" — c'est l'appareil physique de référence utilisé depuis
le début du chapitre.

## 3. Installation en direct sur le casque

$ jenga deploy --platform android --config Release --target EJL4C17401046978 --run
...
Performing Streamed Install
adb.exe: failed to install ...Salle.apk: Failure [INSTALL_FAILED_UPDATE_INCOMPATIBLE:
Package com.ani4087.exo16 signatures do not match the previously installed version; ignoring!]
adb install failed.


Premier accroc, honnête : une version du paquet signée avec une autre clé
(le vrai keystore de release, via `deploy.sh`) était déjà installée sur cet
appareil, et Android refuse une mise à jour signée différemment. On
désinstalle l'ancienne version puis on relance exactement la même commande :

$ adb -s EJL4C17401046978 uninstall com.ani4087.exo16
Success

$ jenga deploy --platform android --config Release --target EJL4C17401046978 --run
...
Performing Streamed Install
Success
APK installed successfully.


L'app se lance sur le téléphone.

## 4. Sur un autre appareil, en changeant un mot

On relance exactement la même commande, en ne changeant que le mot du
`--target` (`EJL4C17401046978` devient `emulator-5554`) :
$ jenga deploy --platform android --config Release --target emulator-5554 --run
...
Performing Streamed Install
adb.exe: failed to install ...Salle.apk: Failure [INSTALL_FAILED_NO_MATCHING_ABIS:
Failed to extract native libraries, res=-113]
adb install failed.


Ce deuxième accroc est lui aussi instructif, et on le garde tel quel plutôt
que de le maquiller : `Salle.jenga` ne construit la bibliothèque native que
pour `arm64-v8a` (`androidabis(["arm64-v8a"])`), alors que cet émulateur
tourne en `x86_64`. Changer un seul mot dans la commande d'installation ne
change rien à ce qui a été *compilé* : la commande de déploiement et la
compatibilité binaire sont deux choses indépendantes. Pour que ce deuxième
appareil reçoive l'app, il faudrait soit un émulateur/appareil ARM64, soit
ajouter `x86_64` à la liste des ABI ciblées dans `Salle.jenga` et
reconstruire.

## 5. Débrancher l'autorisation d'un appareil

Sur le téléphone : Paramètres → Options pour les développeurs → Révoquer
les autorisations de débogage USB. La connexion `adb` déjà active ne se
coupe pas immédiatement (elle reste valide tant qu'elle n'est pas
renégociée) — on force la renégociation :
adb kill-server
adb start-server
adb devices -l

List of devices attached
EJL4C17401046978 unauthorized transport_id:2
emulator-5554 device product:sdk_phone_x86_64 model:Android_SDK_built_for_x86_64 ...


Le téléphone reste bien **présent** dans la liste `adb` — juste marqué
`unauthorized` : `adb` sait qu'un appareil physique est branché sur ce
port USB, il en connaît le transport, mais n'a plus la permission
d'exécuter des commandes dessus (pas de session shell autorisée par une
clé RSA acceptée sur l'appareil).

$ jenga deploy --platform android --list-devices --detailed

SERIAL MARQUE MODELE ANDROID ABI

emulator-5554 unknown Android SDK built for x86_64 11 x86_64


Le téléphone a **disparu** de la liste de Jenga. Pour afficher marque,
modèle, version Android et ABI, Jenga doit exécuter des commandes shell
sur l'appareil (`getprop` et consorts) — ce qui suppose justement une
session autorisée. Un appareil non autorisé ne peut répondre à aucune de
ces requêtes, donc Jenga l'exclut silencieusement de sa liste "détaillée",
alors qu'`adb devices` continue de le voir au niveau transport USB, sans
avoir besoin de lui parler.

**En résumé :** `adb devices` liste ce qui est *branché* (au niveau du
protocole de transport), tandis que la liste détaillée de Jenga liste ce
qui est *interrogeable* (nécessite une autorisation active). Un appareil
peut donc être vu par l'un et invisible pour l'autre, et c'est exactement
ce qui vient de se produire ici.

