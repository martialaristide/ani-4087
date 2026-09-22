# Exercice 17 — L'installation

## Première tentative — échec au lancement
L'installation de l'APK a réussi (`adb install -r`), mais le lancement a échoué avec le
message exact suivant :

​```
java.lang.UnsatisfiedLinkError: Unable to load native library
"/data/app/com.ani4087.exo16-1/lib/arm64/libSalle.so":
dlopen failed: library "libc++_shared.so" not found
​```

Cause : `libSalle.so` est lié dynamiquement à la bibliothèque standard C++, mais celle-ci
n'était pas embarquée dans l'APK.

## Correction
- Ajout de `libc++_shared.so` (NDK, arch `arm64-v8a`) dans `lib/arm64-v8a/` de l'APK,
  stockée sans compression (`ZIP_STORED`).
- Alignement de l'APK avec `zipalign -p 4`.
- Re-signature avec `apksigner sign` (v2 + v3).

## Installation et lancement réussis

adb install -r Salle-Release.apk
Performing Streamed Install
Success
​```

L'application a été lancée sur un appareil physique Android (série EJL4C17401046978) via :
​ adb shell monkey -p com.ani4087.exo16 -c android.intent.category.LAUNCHER 1 ​

https://claude.ai/chat/exo17_screenshot.jpeg


