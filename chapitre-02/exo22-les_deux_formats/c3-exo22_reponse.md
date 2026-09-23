# Exercice 22 — Les deux formats

## Production des deux formats

### APK
Construit avec `jenga build` + `apksigner` (voir exercices precedents) : `Salle-Release.apk`.

### AAB
`bundletool` n'est integre a aucun outil du cours (ni jenga, ni un SDK autonome), donc je l'ai
recupere depuis le cache Gradle d'Android Studio puis telecharge la version executable complete
(`bundletool-all-1.18.3.jar`, depuis les releases GitHub officielles de Google).

Construction manuelle du bundle a partir de l'APK deja compile, en le convertissant au format
protobuf attendu par bundletool :
​```bash
aapt2 convert --output-format proto Salle-Release.apk -o base-proto.apk
# reorganisation : AndroidManifest.xml -> manifest/AndroidManifest.xml, suppression de META-INF/
# puis zip du dossier en base.zip
java -jar bundletool-all.jar build-bundle --modules=base.zip --output=Salle.aab
​```

## Comparaison des tailles
​```
-rw-r--r-- 1 Lenovo 197121 8.9M Sep 23 07:39 Salle-Release.apk
-rw-r--r-- 1 Lenovo 197121 2.3M Sep 23 07:56 Salle.aab
​```

**Attention, comparaison biaisee** : dans mon APK, `libc++_shared.so` est stockee **non
compressee** (`ZIP_STORED`), une contrainte que j'avais du appliquer manuellement pour que
l'alignement memoire fonctionne (voir exercices precedents). `bundletool` compresse
normalement le contenu du module. Donc l'ecart de taille ici vient surtout de la compression,
pas uniquement de la nature du format. Dans un vrai projet Gradle, l'APK serait lui aussi
compresse normalement et l'ecart serait plus faible sur un projet aussi simple.

## Comparaison des contenus
### APK
​```
AndroidManifest.xml          (XML binaire)
resources.arsc                (table de ressources compilee)
lib/arm64-v8a/libSalle.so
lib/arm64-v8a/libc++_shared.so
assets/donnees.txt
META-INF/ANI4087K.SF, .RSA, MANIFEST.MF   (signature JAR)
​```

### AAB
​```
BundleConfig.pb                    (config du bundle)
base/manifest/AndroidManifest.xml  (protobuf, pas XML binaire classique)
base/resources.pb                  (equivalent protobuf de resources.arsc)
base/assets.pb                     (table des assets, absente d'un APK)
base/native.pb                     (table des libs natives, absente d'un APK)
base/assets/donnees.txt
base/lib/arm64-v8a/libc++_shared.so, libSalle.so
​```
(pas de META-INF : le bundle n'est pas signe de la meme facon qu'un APK installable)

## Ce que l'AAB contient que l'APK n'a pas
- **Des tables de metadonnees supplementaires en protobuf** (`assets.pb`, `native.pb`,
  `resources.pb`, `BundleConfig.pb`) qui decrivent le contenu de maniere structuree, exploitable
  par un outil (contrairement au format binaire compact et fige d'un `resources.arsc` d'APK).
- **Une organisation par "module"** (`base/`) : un vrai projet avec plusieurs modules
  dynamiques (feature modules) aurait un dossier par module a l'interieur du meme `.aab`.
- **Aucune signature APK integree** : l'AAB n'est pas cense etre installe tel quel sur un
  appareil (`adb install` refuse un `.aab`), donc il ne porte pas de signature `META-INF/`
  au sens APK — la signature se fait differemment (cle d'upload), et c'est Google Play qui
  regenere et signe les APK finaux avec la cle de signature de l'application.

## Pourquoi une boutique (Google Play) prefere l'AAB
Le point cle : un `.aab` n'est **pas** un format installable, c'est un format de **publication**.
Google Play utilise son contenu pour generer, a la volee, des APK optimises et decoupes
("split APKs") specifiques a chaque appareil qui telecharge l'application :
- seules les bibliotheques natives de la **bonne architecture CPU** de l'appareil (arm64-v8a,
  armeabi-v7a, x86_64...) sont incluses, pas toutes ;
- seules les ressources de la **bonne densite d'ecran** sont incluses ;
- seules les **langues** effectivement utilisees par l'utilisateur sont incluses.

Un APK "universel" doit au contraire embarquer TOUT (toutes les architectures, toutes les
densites, toutes les langues) pour fonctionner sur n'importe quel appareil, ce qui gonfle
inutilement le telechargement pour chaque utilisateur individuel. L'AAB deplace cette
optimisation du developpeur (qui devrait sinon publier plusieurs APK differents a la main)
vers le Play Store, qui la fait automatiquement au moment du telechargement — d'ou des
applications plus legeres a installer, ce qui ameliore le taux de conversion des utilisateurs.
