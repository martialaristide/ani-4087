# Exercice 11 — Le fichier de projet lu en entier

## Fichier analysé

`Applications/NKXRDemo/NKXRDemo.jenga`, dépôt réel du moteur Nkentseu :
https://github.com/Rihen-Universe/Nkentseu/blob/main/Applications/NKXRDemo/NKXRDemo.jenga

## Ce qu'il construit

Un projet nommé `NKXRDemo`, déclaré `windowedapp()` (ligne 16) — une
application **avec fenêtre**, pas une application console, pas une
bibliothèque. Chacun des trois blocs `filter("system:...")` (Windows,
Linux, macOS) reconfirme `windowedapp()` dans son propre contexte, ce qui
suggère que le type de sortie peut varier selon la plateforme si besoin,
même s'il reste identique ici sur les trois.

## Ce dont il dépend

Un seul appel, `nkentseudependson([...])` (lignes 25-29), qui liste 22
modules du moteur d'un coup : `NKXR`, `NKRenderer`, `NKRHI`, `NKSL`,
`NKGLSlang`, `NKSPIRVCross`, `NKSerialization`, `NKReflection`,
`NKFileSystem`, `NKFont`, `NKImage`, `NKGlad`, `NKEvent`, `NKWindow`,
`NKMath`, `NKTime`, `NKLogger`, `NKStream`, `NKContainers`, `NKMemory`,
`NKCore`, `NKPlatform`, `NKThreading`. C'est un raccourci qui fait à la
fois ce que `links()` + `dependson()` font séparément (vu à l'exercice 4).

Un paramètre `extra_includes` (lignes 30-32) ajoute aussi le dossier
`src` et le dossier des en-têtes Vulkan externes — nécessaire parce que
`NkVulkanDevice.h` (utilisé pour la liaison OpenXR) inclut `vulkan.h`.

## Ce qui change d'un système à l'autre

| OS | Toolchain | Bibliothèques / frameworks liés | Define(s) propre(s) |
|---|---|---|---|
| Windows | `TC_WINDOWS` | `user32, gdi32, opengl32, dwmapi, shell32, advapi32, d3d11, d3d12, dxgi, dxguid, d3dcompiler, uuid, ole32` | `WIN32_LEAN_AND_MEAN, _UNICODE, UNICODE` |
| Linux (XLib) | `clang-native` | `pthread, X11, Xext, GL` | `NKENTSEU_FORCE_WINDOWING_XLIB_ONLY` |
| macOS | `clang-native` | frameworks `Cocoa, QuartzCore, OpenGL` | (aucun) |

Windows est la seule plateforme qui lie les bibliothèques Direct3D
(`d3d11`, `d3d12`, `dxgi`, `dxguid`, `d3dcompiler`) en plus d'OpenGL —
cohérent avec un moteur multi-backend. Linux reste sur X11/OpenGL
uniquement. macOS passe par des *frameworks* Apple (`Cocoa`,
`QuartzCore`, `OpenGL`) plutôt que des bibliothèques classiques.

S'y ajoute une différenciation Debug/Release (lignes 70-77) : Debug fixe
`optimize("Off")` et `symbols(True)`, Release fixe `optimize("Speed")` et
`symbols(False)` — sans quoi, comme vu à l'exercice 10, l'optimisation
resterait désactivée par défaut même en Release.

## Les trois pièges documentés, et ce qui se passerait sans la ligne

### Piège 1 — dépendance transitive non récupérée (lignes 22-24)

> « NKGLSlang/NKSPIRVCross explicites : le linker d'un exécutable qui
> tire NKSL/NKRHI ne les récupère pas transitivement (piège documenté
> dans NkLocomotionDemo.jenga, 2026-07-23). »

`NKSL` et `NKRHI` dépendent en interne de `NKGLSlang` et `NKSPIRVCross`,
mais un exécutable qui dépend seulement de `NKSL`/`NKRHI` ne récupère pas
automatiquement leurs propres dépendances au moment de l'édition de
liens — c'est exactement le piège reproduit à la main à l'exercice 4.

**Sans cette ligne** (si `NKGLSlang` et `NKSPIRVCross` étaient retirés de
la liste `nkentseudependson`) : la compilation réussirait normalement,
mais l'édition de liens échouerait avec des `undefined reference` sur des
symboles internes à ces deux modules — une erreur qui n'apparaît qu'au
lien, pas à la compilation.

### Piège 2 — define local non propagé (lignes 35-37)

> « NK_RHI_VK_ENABLED est un define LOCAL de NKRHI (non propagé) : sans
> lui, NkVulkanDevice.h montre sa classe STUB et la liaison OpenXR ne
> compile pas. »

C'est le piège `#ifdef` de l'exercice 5 : un header qui expose soit la
vraie classe, soit une classe STUB vide selon qu'un define est actif ou
non dans l'unité de compilation qui l'inclut.

**Sans cette ligne** (sans `defines(["NK_RHI_VK_ENABLED"])`) :
`NkVulkanDevice.h` présenterait sa version STUB (vide). Le code de
`NKXRDemo` qui utilise les vrais membres de cette classe pour la liaison
OpenXR échouerait alors à la **compilation** (membres inexistants sur la
classe stub), et non au lien — contrairement au piège 1.

### Piège 3 — bibliothèque système manquante pour un appel Win32 précis (lignes 52-53)

> « advapi32 : RegGetValueA de la découverte du runtime OpenXR actif
> (NkXrOpenXRBackend, étape 2a). »

`RegGetValueA` est une fonction Win32 (lecture du registre) utilisée pour
détecter quel runtime OpenXR est actif sur la machine. Sa déclaration
vient de `<windows.h>` (toujours disponible), mais son implémentation
vit dans `advapi32.lib`/`.dll`.

**Sans cette ligne** (sans lier `advapi32` dans la liste `links()` du
bloc Windows) : la compilation réussirait, car l'en-tête suffit à
satisfaire le compilateur — mais l'édition de liens échouerait avec un
`undefined reference to RegGetValueA` (ou l'équivalent MSVC), la fonction
n'étant présente dans aucune des autres bibliothèques déjà liées.

## Constat général

Les trois pièges suivent le même schéma : chacun est **invisible dans le
code C++ lui-même** — rien n'indique, en lisant `NkVulkanDevice.h` ou en
appelant `RegGetValueA`, qu'une ligne précise du fichier de build est ce
qui rend ce code utilisable. Seul le commentaire au-dessus de la ligne
concernée, dans le `.jenga`, explique pourquoi elle existe — ce qui
confirme l'intérêt de lire un fichier de projet en entier, commentaires
compris, avant d'y toucher.
