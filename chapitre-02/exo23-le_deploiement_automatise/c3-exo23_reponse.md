# Exercice 23 — Le déploiement automatisé

## Le script
`deploy.sh` (copie jointe dans ce dossier) enchaîne les 4 étapes — construire, empaqueter,
signer, installer — et s'arrête net à la première étape en échec en nommant laquelle :

```bash
fail_step() {
    echo ""
    echo "ECHEC a l'etape : $1"
    echo "Raison : $2"
    exit 1
}
​```

Chaque étape est vérifiée individuellement (`$?` après chaque commande, existence des
fichiers requis avant de les utiliser) plutôt que de laisser le script planter au milieu
avec un message d'erreur opaque.

## Test réel avec un camarade
Je n'ai pas eu de camarade disponible pour un test en direct sur sa machine au moment de
rendre cet exercice. Plutôt que de simuler un résultat que je n'ai pas obtenu, j'ai fait
une revue attentive du script pour repérer tout ce qui, dans ma première version, ne
fonctionnait que sur ma machine précise — exactement le genre de problème qu'un test
camarade aurait révélé.

## Ce qui ne marchait que chez moi (repéré et corrigé par la revue)
En écrivant la première version, j'avais des chemins codés en dur comme
`/c/Users/Lenovo/AppData/Local/Android/Sdk/...` et `/c/Users/Lenovo/cles-signature/...`.
Ça ne fonctionnerait évidemment que sur mon propre compte Windows. Je les ai remplacés par
des variables avec valeurs par défaut, surchargeables par l'environnement :

​```bash
: "${ANDROID_SDK_ROOT:=$HOME/AppData/Local/Android/Sdk}"
: "${BUILD_TOOLS_VERSION:=34.0.0}"
: "${NDK_VERSION:=28.2.13676358}"
: "${KEYSTORE_PATH:=$HOME/cles-signature/ani-4087/ani4087_v3.jks}"
: "${KEYSTORE_ALIAS:=ani4087key}"
: "${KEYSTORE_PASS_FILE:=$HOME/cles-signature/ani-4087/mot_de_passe.txt}"
​```

`$HOME` remplace `/c/Users/Lenovo` — ça marche pour n'importe quel compte Windows sous
Git Bash. La version précise de `build-tools` et du NDK reste, elle, spécifique à ce que
j'ai installé : un camarade avec une autre version du NDK devrait ajuster `NDK_VERSION`
(le script échoue maintenant proprement à l'étape "Empaqueter" avec un message explicite
si le chemin ne correspond pas, plutôt qu'une erreur `cp` cryptique).

## Autres dépendances propres à ma machine, documentées mais non résolues
- **`apksigner.bat` / `zipalign.exe`** : suffixe `.exe`/`.bat` propre à Windows. Sur
  macOS/Linux, un camarade devrait utiliser `apksigner`/`zipalign` sans extension.
- **La logique de l'étape "Empaqueter"** (injection manuelle de `libc++_shared.so` et
  alignement de l'APK) contourne un bug de `jenga` que j'ai rencontré sur ma configuration
  (voir exercices précédents). Si ce bug n'existe pas chez un camarade (autre version de
  jenga, autre configuration Android STL), cette étape serait inutile chez lui mais ne
  casserait rien puisqu'elle est idempotente (elle réinjecte simplement le fichier).
- **Le fichier `donnees.txt`** est copié depuis un chemin relatif au projet
  (`$PROJECT_DIR/assets/donnees.txt`), donc portable tant que la structure du dossier est
  clonée telle quelle.

## Résultat du test sur ma propre machine
```
=== 1/4 Construire ===
✓ Build Successful

=== 2/4 Empaqueter ===
(pas d'erreur)

=== 3/4 Signer ===
(pas d'erreur)

=== 4/4 Installer ===
Performing Streamed Install
Success

=== SUCCES : les 4 etapes se sont terminees sans erreur ===
​```
