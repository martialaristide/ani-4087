# Demo3 — La coquille vide

## L'en-tête partagé (`widget.h`)
```cpp
#pragma once

class Widget {
public:
#ifdef WITH_EXTRA
    void configure(int a, int b);
#else
    void configure(int a);
#endif
};
```

## Le même en-tête, compilé deux fois, avec des define différents

**`impl.cpp`** (implémente `Widget::configure`, compilé **sans** define) :
```cpp
#include "widget.h"
void Widget::configure(int a) { /* ... */ }
```
```bash
$ g++ -c impl.cpp -o impl.o
Code retour: 0
```

**`main.cpp`** (appelle `Widget::configure`, compilé **avec** `-DWITH_EXTRA`) :
```cpp
#include "widget.h"
int main() {
    Widget w;
    w.configure(1, 2);
    return 0;
}
```
```bash
$ g++ -c main.cpp -o main.o -DWITH_EXTRA
Code retour: 0
```

Les deux compilations réussissent — **le compilateur ne voit rien d'anormal**, dans les deux
cas.

## La preuve que ce n'est pas la même classe
```bash
$ nm impl.o | grep -i configure
0000000000000000 T _ZN6Widget9configureEi

$ nm main.o | grep -i configure
                 U _ZN6Widget9configureEii
```
`impl.o` définit (`T`) le symbole `Widget::configure(int)` — un seul paramètre. `main.o` a
besoin (`U`, non défini) du symbole `Widget::configure(int, int)` — deux paramètres. Le nom
mangé du symbole encode la signature complète de la méthode, donc deux états différents du
même `#define` produisent littéralement deux classes `Widget` différentes, avec des
symboles incompatibles — bien qu'elles portent le même nom et proviennent du même fichier
`.h`.

## L'édition de liens échoue
```bash
$ g++ impl.o main.o -o programme.exe
C:/msys64/.../ld.exe: main.o:main.cpp:(.text+0x20): undefined reference to `Widget::configure(int, int)'
collect2.exe: error: ld returned 1 exit status
```

## Pourquoi le lieur proteste, et non le compilateur
Le compilateur travaille **une unité de traduction à la fois** : quand il compile `impl.cpp`,
il ne voit que l'état du `#define` de ce fichier-là, et le fichier est parfaitement cohérent
avec lui-même — rien à signaler. Il en va de même pour `main.cpp`, compilé séparément, avec
un état différent du `#define`. Le compilateur n'a **aucune visibilité** sur l'autre fichier
au moment où il travaille : il ne peut donc pas savoir que deux traductions différentes de
la "même" classe existent quelque part dans le programme. C'est seulement à l'édition de
liens, quand tous les fichiers objets sont assemblés en un seul programme et que le lieur
doit faire correspondre chaque appel à sa définition, que l'incohérence entre les deux
traductions devient visible — et le lieur la voit uniquement comme un symbole absent
(`configure(int, int)` n'existe nulle part), sans jamais savoir que la vraie cause est une
violation de la règle de définition unique (ODR) provoquée par un `#define` incohérent entre
fichiers.
