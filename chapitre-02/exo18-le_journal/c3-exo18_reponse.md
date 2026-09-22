# Exercice 18 — Le journal

## Affichage des trois lignes au démarrage
Dans `src/main.cpp`, au début de `android_main`, avant la boucle de rendu :

​```cpp
__android_log_print(ANDROID_LOG_INFO, "ExoLePaquetVide", "Ligne 1 : demarrage de l'application");
__android_log_print(ANDROID_LOG_INFO, "ExoLePaquetVide", "Ligne 2 : fenetre native prete");
__android_log_print(ANDROID_LOG_INFO, "ExoLePaquetVide", "Ligne 3 : boucle de rendu lancee");
​```

## Commande de filtrage employée
Le journal système Android (`logcat`) mélange les messages de toutes les applications et
du système. Pour isoler uniquement les miens, j'ai filtré par tag avec l'option `-s`
(« silence tout le reste ») :

​```bash
adb logcat -d -s ExoLePaquetVide
​```

`-d` : affiche le contenu du journal puis quitte (au lieu de rester en flux continu).
`-s ExoLePaquetVide` : ne garde que les entrées portant ce tag, coupe tout le bruit système.

## Résultat obtenu
​```
09-22 23:18:00.783 27614 27630 I ExoLePaquetVide: Ligne 1 : demarrage de l'application
09-22 23:18:00.783 27614 27630 I ExoLePaquetVide: Ligne 2 : fenetre native prete
09-22 23:18:00.783 27614 27630 I ExoLePaquetVide: Ligne 3 : boucle de rendu lancee
​```

Remarque : pour observer un redémarrage complet (et donc un nouvel appel à `android_main`),
il faut forcer l'arrêt du processus avant de relancer, sinon l'application déjà en mémoire
est simplement ramenée au premier plan sans réexécuter le code de démarrage :

​```bash
adb shell am force-stop com.ani4087.exo16
adb logcat -c
adb shell monkey -p com.ani4087.exo16 -c android.intent.category.LAUNCHER 1
​```
