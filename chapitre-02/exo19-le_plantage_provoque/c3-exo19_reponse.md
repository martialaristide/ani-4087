# Exercice 19 — Le plantage provoqué

## Code du plantage
Dans `src/main.cpp`, après les 3 logs de démarrage, avant la boucle de rendu :

​```cpp
int *pointeurNul = nullptr;
*pointeurNul = 42;  // plantage volontaire : dereferencement d'un pointeur nul
​```

## Commande de capture
```bash
adb logcat -c
adb shell monkey -p com.ani4087.exo16 -c android.intent.category.LAUNCHER 1
adb logcat -d | grep -i "AndroidRuntime\|libc\s*:\s*Fatal\|DEBUG\s*:"
​```

## Trace obtenue
​```
09-22 23:37:54.069 28759 28775 F libc    : Fatal signal 11 (SIGSEGV), code 1, fault addr 0x32 in tid 28775 (m.ani4087.exo16)
09-22 23:37:54.161 28776 28776 F DEBUG   : Build fingerprint: 'HUAWEI/MLA-L11/HWMLA:7.0/HUAWEIMLA-L11/C432B330:user/release-keys'
09-22 23:37:54.161 28776 28776 F DEBUG   : ABI: 'arm64'
09-22 23:37:54.161 28776 28776 F DEBUG   : pid: 28759, tid: 28775, name: m.ani4087.exo16  >>> com.ani4087.exo16 <
09-22 23:37:54.161 28776 28776 F DEBUG   : signal 11 (SIGSEGV), code 1 (SEGV_MAPERR), fault addr 0x32
09-22 23:37:54.161 28776 28776 F DEBUG   :     x0   0000000000000032  x1   0000007fa1ca3c70  ...
09-22 23:37:54.161 28776 28776 F DEBUG   :     pc   0000007fa1ce8240  pstate 0000000060000000
09-22 23:37:54.162 28776 28776 F DEBUG   : backtrace:
09-22 23:37:54.162 28776 28776 F DEBUG   :     #00 pc 0000000000000240  /data/app/com.ani4087.exo16-1/lib/arm64/libSalle.so (offset 0x2000)
09-22 23:37:54.162 28776 28776 F DEBUG   :     #01 pc 0000000000000228  /data/app/com.ani4087.exo16-1/lib/arm64/libSalle.so (offset 0x2000)
​```

## Ce que cette trace m'apprend
- **La cause générale** : `SIGSEGV` avec `code 1 (SEGV_MAPERR)` signifie une tentative d'accès
  à une adresse mémoire non mappée — le symptôme classique d'un pointeur nul ou invalide déréférencé.
- **L'adresse fautive** (`fault addr 0x32`) : très proche de zéro, cohérent avec un accès via
  un pointeur nul (ou nul + petit décalage, si le compilateur a réorganisé le code).
- **Le processus et le thread concernés** (`pid 28759`, `tid 28775`, nom du processus) — utile
  pour savoir si le crash vient du thread principal ou d'un thread secondaire.
- **La bibliothèque et l'offset en cause** (`libSalle.so`, offset `0x2000` + décalage dans la pile
  d'appel) — ça confirme que le crash vient bien de mon code natif, pas du framework Android.
- **L'empreinte de build de l'appareil** (modèle, version Android, ABI) — utile pour reproduire
  dans le même environnement.

## Ce qu'elle ne m'apprend PAS
- **La ligne de code exacte ni le nom de la fonction** : le backtrace ne donne que des offsets
  binaires (`pc 0000000000000240`), pas de numéros de ligne ni de noms symboliques, car l'APK
  n'embarque pas les symboles de débogage. Il faudrait passer ces offsets dans `ndk-stack` ou
  `addr2line` avec la version non strippée de `libSalle.so` pour retrouver la ligne source.
- **La valeur exacte que je tentais d'écrire** (`42`) : les registres ne montrent pas directement
  l'intention du code, seulement l'état bas niveau au moment du crash.
- **Le contexte applicatif complet** : pas de nom de variable, pas d'état des objets C++ de haut
  niveau, seuls des registres CPU et des adresses brutes.
- **Si le bug est systématique ou intermittent** : une seule trace ne prouve pas que le crash
  se reproduit à chaque exécution dans toutes les conditions (même si ici, un pointeur nul
  déréférencé sans condition crashera toujours).
- **La cause métier du bug** : la trace dit *où* ça a cassé techniquement, pas *pourquoi*
  algorithmiquement le pointeur était nul à cet endroit — ça reste à l'humain de le déduire
  du code source.
```

