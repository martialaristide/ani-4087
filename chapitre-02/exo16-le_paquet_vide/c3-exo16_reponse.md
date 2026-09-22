# Exercice 16 — Le paquet vide

## Signature
L'APK a été signé avec `jenga sign` en utilisant un keystore JKS (alias `ani4087key`),
puis vérifié avec `apksigner verify --verbose` :

- Vérifié : oui (`Verifies`)
- Schéma v2 (APK Signature Scheme v2) : oui
- Schéma v3 (APK Signature Scheme v3) : oui
- Nombre de signataires : 1

## Taille du paquet
`Salle-Release.apk` : **17K** (27 940 octets de contenu non compressé)

## Contenu du paquet
Sortie de `unzip -l Salle-Release.apk` :

Archive: Salle-Release.apk
Length Date Time Name

 2192  1980-01-01 01:00   AndroidManifest.xml
   40  1980-01-01 01:00   resources.arsc
23816  2026-09-22 02:14   lib/arm64-v8a/libSalle.so
  426  2026-09-22 02:14   META-INF/ANDROIDD.SF
 1167  2026-09-22 02:14   META-INF/ANDROIDD.RSA
  299  2026-09-22 02:14   META-INF/MANIFEST.MF
27940                     6 files


Un APK est une archive ZIP ordinaire : elle contient le manifeste (`AndroidManifest.xml`),
les ressources compilées (`resources.arsc`), la bibliothèque native compilée pour
l'architecture ARM64 (`lib/arm64-v8a/libSalle.so`), et les métadonnées de signature
(`META-INF/MANIFEST.MF`, `ANDROIDD.SF`, `ANDROIDD.RSA`) ajoutées lors de la signature.


