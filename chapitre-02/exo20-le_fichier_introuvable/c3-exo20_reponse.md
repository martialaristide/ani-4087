# Exercice 20 — Le fichier introuvable

## Fichier de ressource
`assets/donnees.txt` :

Bonjour depuis le fichier de ressources !
​```
Étape 1 — fopen classique

​cpp FILE *f = fopen("assets/donnees.txt", "r"); if (f) { char buffer[256] = {0}; fread(buffer, 1, sizeof(buffer) - 1, f); fclose(f); __android_log_print(ANDROID_LOG_INFO, "ExoLePaquetVide", "fopen a REUSSI, contenu: %s", buffer); } else { __android_log_print(ANDROID_LOG_ERROR, "ExoLePaquetVide", "fopen a ECHOUE (errno=%d: %s)", errno, strerror(errno)); } ​

Sur ma machine (PC, test isole compile avec g++)

​ $ g++ test_fopen_pc.cpp -o test_fopen_pc.exe $ ./test_fopen_pc.exe fopen a REUSSI, contenu: Bonjour depuis le fichier de ressources ! ​
Ca marche : sur PC, assets/donnees.txt est un vrai fichier sur le disque, accessible par
chemin relatif au dossier de travail.

Sur l'appareil (APK installe)

​ 09-23 06:20:59.553 18478 18494 E ExoLePaquetVide: fopen a ECHOUE (errno=2: No such file or directory) ​
Ca echoue : dans un APK, les ressources declarees sont stockees a l'interieur de l'archive ZIP
(dans le dossier assets/), pas extraites sur le systeme de fichiers reel. fopen cherche un
chemin de fichier ordinaire et ne sait pas lire a l'interieur d'un ZIP — d'ou errno=2
(No such file or directory), meme si le fichier existe bel et bien dans l'APK.

Etape 2 — correction avec AAssetManager (API du moteur/NDK)

​cpp AAssetManager *assetManager = app->activity->assetManager; AAsset *asset = AAssetManager_open(assetManager, "donnees.txt", AASSET_MODE_BUFFER); if (asset) { off_t length = AAsset_getLength(asset); char buffer2[256] = {0}; AAsset_read(asset, buffer2, length < 255 ? length : 255); AAsset_close(asset); __android_log_print(ANDROID_LOG_INFO, "ExoLePaquetVide", "AAssetManager a REUSSI, contenu: %s", buffer2); } else { __android_log_print(ANDROID_LOG_ERROR, "ExoLePaquetVide", "AAssetManager a ECHOUE a ouvrir le fichier"); } ​

Sur l'appareil, avec la correction

​ 09-23 06:20:59.553 18478 18494 I ExoLePaquetVide: AAssetManager a REUSSI, contenu: Bonjour depuis le fichier de ressources ! ​
AAssetManager est l'API NDK dediee : elle sait lire directement a l'interieur de l'archive
APK (via app->activity->assetManager, fourni par android_native_app_glue), sans passer
par le systeme de fichiers classique. Le chemin passe est relatif au dossier assets/ de
l'APK (donc juste "donnees.txt", sans prefixe).

Resume
Methode	PC	Appareil (APK)
fopen	Reussit	Echoue (errno=2)
AAssetManager	N/A (API Android uniquement)	Reussit


