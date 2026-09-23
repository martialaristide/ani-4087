# Exercice 25 — L'inventaire de ce qui est branché

## Remarque preliminaire
`jenga` (notre outil de build) n'a pas de commande `--list-devices`/`--detailed` — j'ai verifie
tout son catalogue de commandes (`jenga --help` et l'aide de chaque sous-commande individuelle :
`build`, `run`, `gdb`, `info`...), aucune ne propose cette fonctionnalite. J'ai donc utilise
l'equivalent reel du SDK Android, `adb`, qui offre exactement la meme distinction a deux niveaux
de detail :
- `adb devices` = liste simple
- `adb devices -l` = liste detaillee (long format)

## Un seul appareil disponible
Je n'ai qu'un seul telephone Android sous la main au moment de cet exercice (pas de second
appareil physique, pas d'emulateur deja configure). Les deux commandes ci-dessous n'affichent
donc qu'une seule ligne chacune — mais la difference entre les deux niveaux de detail reste
visible et repond a la question posee.

## Sortie 1 — liste simple
`````bash
$ adb devices
List of devices attached
EJL4C17401046978        device
​```

## Sortie 2 — liste detaillee
````bash
$ adb devices -l
List of devices attached
EJL4C17401046978       device product:MLA-L11 model:HUAWEI_MLA_L11 device:HWMLA transport_id:4
​```

## Ce que la seconde m'apprend que la première ne disait pas
La liste simple ne donne que deux informations : le **numero de serie** (`EJL4C17401046978`) et
l'**etat de connexion** (`device` = pret et autorise, par opposition a `unauthorized` ou
`offline`, deux etats que j'ai deja rencontres plus tot dans ce chapitre).

La liste detaillee ajoute :
- **`product:MLA-L11`** — le nom de code produit interne du fabricant.
- **`model:HUAWEI_MLA_L11`** — le nom du modele commercial, utile pour identifier visuellement
  quel appareil physique correspond a quel numero de serie quand plusieurs sont branches.
- **`device:HWMLA`** — le nom de code de la configuration materielle (board/device codename),
  distinct du modele commercial.
- **`transport_id:4`** — l'identifiant de la connexion de transport ADB elle-meme (utile pour
  distinguer deux connexions au meme appareil, par exemple USB et WiFi simultanement).

Autrement dit : la liste simple dit seulement *"quelque chose de pret est branche"*, la liste
detaillee dit *"quoi precisement, avec quel materiel"* — indispensable des qu'on a plusieurs
appareils branches en meme temps et qu'il faut cibler le bon avec `--target`/`--device` lors
d'un `jenga run` ou d'un `adb install`.

## ABI de l'appareil
Ni `adb devices` ni `adb devices -l` ne donnent l'ABI — il faut l'interroger separement :
```bash
$ adb shell getprop ro.product.cpu.abi
arm64-v8a
$ adb shell getprop ro.product.cpu.abilist
arm64-v8a,armeabi-v7a,armeabi
​```

| Appareil | Serie | Modele | ABI principale | ABIs supportees |
|-----------|--------------------|-------------------|------------------|--------------------------------|
| Telephone | EJL4C17401046978 | HUAWEI_MLA_L11 | arm64-v8a | arm64-v8a, armeabi-v7a, armeabi |

C'est cette ABI (`arm64-v8a`) qui doit correspondre a celle declaree dans `Salle.jenga`
(`androidabis(["arm64-v8a"])`) pour que l'APK que je construis et signe dans les exercices
precedents puisse s'installer et se charger correctement sur cet appareil precis.
```
