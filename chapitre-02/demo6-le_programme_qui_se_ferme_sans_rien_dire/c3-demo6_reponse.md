# Demo6 — Le programme qui se ferme sans rien dire

## Provoquer le crash silencieux
J'ai reconstruit l'APK **sans** l'étape de correction habituelle (pas d'injection de
`libc++_shared.so` dans l'archive) et je l'ai installé tel quel sur le téléphone :
```bash
jenga build --platform Android-arm64 --config Release --no-cache
adb -s EJL4C17401046978 uninstall com.ani4087.exo16
adb -s EJL4C17401046978 install -r Salle-Release.apk
adb -s EJL4C17401046978 shell monkey -p com.ani4087.exo16 -c android.intent.category.LAUNCHER 1
```

## Ce qu'on voit sur l'appareil
Rien — l'écran affiche brièvement l'app puis revient directement au bureau, sans aucun
message d'erreur visible. Capture de l'écran juste après le crash (`ecran_apres_crash.png`) :
l'écran d'accueil du téléphone, comme si l'app n'avait jamais été lancée.

## Ouvrir le journal en direct pour trouver la trace
```bash
adb -s EJL4C17401046978 logcat -c
adb -s EJL4C17401046978 shell monkey -p com.ani4087.exo16 -c android.intent.category.LAUNCHER 1
adb -s EJL4C17401046978 logcat -d | grep -B2 -A20 "FATAL EXCEPTION"
```

## La trace trouvée
```
--------- beginning of crash
09-24 08:55:33.783  5299  5299 E AndroidRuntime: FATAL EXCEPTION: main
09-24 08:55:33.783  5299  5299 E AndroidRuntime: Process: com.ani4087.exo16, PID: 5299
09-24 08:55:33.783  5299  5299 E AndroidRuntime: java.lang.UnsatisfiedLinkError: Unable to load native library "/data/app/com.ani4087.exo16-1/lib/arm64/libSalle.so": dlopen failed: library "libc++_shared.so" not found
09-24 08:55:33.783  5299  5299 E AndroidRuntime:        at android.app.NativeActivity.onCreate(NativeActivity.java:202)
09-24 08:55:33.783  5299  5299 E AndroidRuntime:        at android.app.Activity.performCreate(Activity.java:6910)
...
09-24 08:55:33.787  2025  2872 I FreezeScreenScene: scheduleCheckFreezeScreen sendMessageDelayed
09-24 08:55:33.788  5299  5299 I Process : Sending signal. PID: 5299 SIG: 9
```

## Ce que ça raconte
L'écran ne dit jamais "erreur" parce que ce n'est pas une erreur affichée par l'interface :
c'est le processus Java de l'app (`ActivityThread`) qui plante avant même que la première
image ne soit dessinée — le système Android tue le processus (`SIG: 9`) et rend simplement
la main au bureau, comme si rien ne s'était passé. Le journal, lui, montre que la cause est
la même qu'à l'exo17 et au demo5 : `libSalle.so` a besoin de `libc++_shared.so`
(bibliothèque C++ standard partagée) qui n'a pas été embarquée dans l'APK à cette
construction. Sans le journal, ce crash est totalement muet ; avec lui, la cause exacte
apparaît en une seule ligne lisible.
