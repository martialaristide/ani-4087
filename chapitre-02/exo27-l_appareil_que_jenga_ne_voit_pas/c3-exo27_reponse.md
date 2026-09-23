# Exercice 27 — L'appareil que Jenga ne voit pas

## 1. État `unauthorized` (autorisations USB révoquées, popup affichée, non touchée)

**adb devices**
List of devices attached
EJL4C17401046978 unauthorized
emulator-5554 device


**jenga deploy** (même appareil ciblé)
adb.EXE: device unauthorized.
This adb server's $ADB_VENDOR_KEYS is not set
Try 'adb kill-server' if that seems wrong.
Otherwise check for a confirmation dialog on your device.
adb install failed.


## 2. Après avoir accepté l'autorisation sur le téléphone

**adb devices**
List of devices attached
EJL4C17401046978 device
emulator-5554 device


**jenga deploy** (même commande, même appareil)
Performing Streamed Install
Success
APK installed: ...\Salle-Release.apk


## 3. Pourquoi le message de Jenga est trompeur

`adb` dit précisément *pourquoi* ça échoue (`device unauthorized`, avec la bonne piste —
regarder la popup sur l'appareil) ; Jenga se contente de relayer ce texte brut d'adb puis
d'ajouter sa propre ligne `adb install failed.`, qui est **exactement la même** qu'il
afficherait pour n'importe quel autre échec d'installation (APK manquant, signature
incompatible, ABI incorrecte...) — donc si on ne lit pas attentivement les lignes d'adb
juste au-dessus, on ne sait pas si c'est un problème d'autorisation, un APK cassé ou autre
chose. À la place de `jenga`, j'aurais écrit quelque chose qui interprète l'état lui-même
plutôt que de juste renvoyer le texte brut, par exemple :
`Deploy failed: device EJL4C17401046978 is 'unauthorized' — accept the USB debugging prompt
on the device, then retry.`
