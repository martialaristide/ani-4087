# Exercice 24 — Le journal de bord du portage

Liste de tout ce qui a casse pendant ce chapitre (exo16 a exo23), dans l'ordre chronologique.
Temps approximatifs, estimes a partir du temps effectif passe a diagnostiquer chaque probleme
(pas le temps d'horloge brut, qui inclut des pauses).

## 1. Mot de passe keystore incorrect
- **Symptome** : `keystore password was incorrect` / `BadPaddingException` au moment de signer.
- **Ce que j'ai cru** : le mot de passe stocke dans mon fichier etait faux.
- **Ce que c'etait** : j'avais copie-colle le placeholder litteral `VotreMotDePasseExact` au lieu
  du vrai mot de passe dans la commande.
- **Temps perdu** : ~5 min.

## 2. `--storepass: command not found`
- **Symptome** : bash refuse la commande.
- **Ce que j'ai cru** : erreur de syntaxe dans l'option `--storepass`.
- **Ce que c'etait** : j'avais colle un fragment de commande isole au lieu de la ligne complete.
- **Temps perdu** : ~2 min.

## 3. `apksigner`/`adb` introuvables apres reouverture du terminal
- **Symptome** : `command not found` alors que ca marchait juste avant.
- **Ce que j'ai cru** : l'outil avait disparu ou etait mal installe.
- **Ce que c'etait** : les `export PATH=...` ne persistent pas d'une fenetre Git Bash a l'autre ;
  il faut les refaire (ou les mettre dans `~/.bashrc`).
- **Temps perdu** : ~3 min a chaque occurrence, plusieurs fois dans le chapitre.

## 4. Crash au lancement : `UnsatisfiedLinkError: libc++_shared.so not found`
- **Symptome** : l'installation reussit, l'app plante immediatement au lancement.
- **Ce que j'ai cru** : un probleme de signature ou d'installation.
- **Ce que c'etait** : `libSalle.so` est lie dynamiquement a la libc++ partagee
  (`androidstl("c++_shared")`), mais jenga ne l'embarque pas automatiquement dans l'APK malgre
  cette declaration — un vrai bug/limite de l'outil de build.
- **Temps perdu** : ~20 min (diagnostic via logcat + recherche de la lib dans le NDK).

## 5. Signature cassee apres injection manuelle du `.so`
- **Symptome** : `DOES NOT VERIFY... Signature stripped?` apres avoir ajoute le fichier manquant
  dans l'APK avec Python.
- **Ce que j'ai cru** : `jenga sign` avait echoue silencieusement.
- **Ce que c'etait** : modifier le ZIP apres signature casse le bloc de signature v2/v3 (qui vit
  hors de la structure ZIP standard) ; il fallait nettoyer `META-INF/` et re-signer entierement
  apres toute modification du contenu.
- **Temps perdu** : ~10 min.

## 6. `zip: command not found`
- **Symptome** : impossible d'ajouter un fichier a l'APK avec `zip`.
- **Ce que j'ai cru** : outil manquant a installer.
- **Ce que c'etait** : `zip` n'est simplement pas fourni par defaut sous Git Bash sur Windows —
  j'ai utilise le module `zipfile` de Python a la place, deja disponible.
- **Temps perdu** : ~3 min.

## 7. Crash persistant malgre la lib presente dans l'APK
- **Symptome** : meme erreur `libc++_shared.so not found` alors que le fichier etait bien dans
  l'archive.
- **Ce que j'ai cru** : le fichier n'avait pas ete correctement ajoute.
- **Ce que c'etait** : la lib etait compressee (`ZIP_DEFLATED`) alors qu'un APK avec
  `extractNativeLibs` implicite exige des bibliotheques natives **non compressees et alignees**
  pour etre chargees directement (`mmap`) depuis l'archive.
- **Temps perdu** : ~15 min.

## 8. `adb devices` affiche `unauthorized`
- **Symptome** : l'appareil est detecte mais toutes les commandes echouent.
- **Ce que j'ai cru** : probleme de cable ou de driver.
- **Ce que c'etait** : la popup d'autorisation de debogage USB n'avait pas ete acceptee sur le
  telephone (ou etait cachee derriere l'ecran verrouille).
- **Temps perdu** : ~5 min.

## 9. `INSTALL_FAILED_UPDATE_INCOMPATIBLE`
- **Symptome** : `adb install` refuse la nouvelle version.
- **Ce que j'ai cru** : APK corrompu.
- **Ce que c'etait** : `jenga build` re-signe automatiquement chaque nouvel APK avec
  `debug.keystore`, different de ma cle de signature — Android refuse une mise a jour dont la
  signature ne correspond pas a la version deja installee. Il fallait desinstaller avant de
  reinstaller.
- **Temps perdu** : ~5 min.

## 10. `screencap`/`pull` echouent avec un chemin `/sdcard/...`
- **Symptome** : `adb shell screencap` renvoie l'aide de la commande au lieu de s'executer.
- **Ce que j'ai cru** : mauvaise syntaxe de la commande `screencap`.
- **Ce que c'etait** : Git Bash (MSYS) convertit automatiquement les chemins commencant par `/`
  en chemins Windows avant de les transmettre a `adb`, corrompant l'argument. Contournement :
  doubler le slash initial (`//sdcard/...`).
- **Temps perdu** : ~5 min.

## 11. Bloc `else` orphelin -> erreur de compilation
- **Symptome** : `expected expression` / `expected '}'` a la compilation.
- **Ce que j'ai cru** : erreur dans le nouveau code juste ajoute (AAssetManager).
- **Ce que c'etait** : un residu de copier-coller avait laisse un `else {` sans `if` correspondant
  juste avant la boucle principale.
- **Temps perdu** : ~5 min.

## 12. `bundletool-*.jar` du cache Gradle sans classe principale
- **Symptome** : `aucun attribut manifest principal` puis `NoClassDefFoundError` sur les
  dependances protobuf.
- **Ce que j'ai cru** : le jar recupere dans le cache Gradle etait suffisant.
- **Ce que c'etait** : c'est une bibliotheque (sans dependances embarquees), pas l'executable
  autonome ; il fallait telecharger `bundletool-all-*.jar` depuis les releases GitHub.
- **Temps perdu** : ~5 min.

## 13. `git push` : `Could not resolve host` / `Failed to connect`
- **Symptome** : le push echoue apres un commit local reussi.
- **Ce que j'ai cru** : au debut, un probleme lie a Git ou au depot.
- **Ce que c'etait** : coupures reseau/DNS locales, intermittentes, sans rapport avec Git — le
  commit restait intact localement en attendant une connexion valide.
- **Temps perdu** : ~5-10 min a chaque occurrence, plusieurs fois dans le chapitre.

## Total estime
Environ **1h30 a 2h** de temps de diagnostic cumule sur l'ensemble du chapitre, pour des
problemes qui, une fois compris, se corrigent chacun en quelques secondes.

## Le vrai enseignement
La quasi-totalite de ces incidents ne sont pas des bugs dans MON code — c'est de la tuyauterie
d'outillage (chemins qui ne persistent pas, conversions de chemin silencieuses, signature qui
se casse en silence, bibliotheques qui ne s'embarquent pas malgre leur declaration). Le reflexe
utile n'est pas "mon code est faux", c'est "que dit exactement le journal/l'erreur, et est-ce
deja arrive dans ce chapitre ?" — d'ou l'interet de ce document, a relire avant de repartir dans
une session de debogage.
