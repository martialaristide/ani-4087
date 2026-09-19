# Exercice 5 — Le define qui manque

## L'en-tete conditionnel

\`\`\`cpp
// Widget.hpp
#pragma once
#ifdef WITH_WIDGET
class Widget {
public:
    Widget();
    int GetValue() const;
private:
    int m_value;
};
#else
class Widget {};
#endif
\`\`\`

\`\`\`cpp
// Widget.cpp
#include "Widget.hpp"
#ifdef WITH_WIDGET
Widget::Widget() : m_value(42) {}
int Widget::GetValue() const { return m_value; }
#endif
\`\`\`

\`\`\`cpp
// main.cpp
#include "Widget.hpp"
int main() {
    Widget w;
    return w.GetValue();
}
\`\`\`

## Scenario A — le define absent partout (Widget.cpp ET main.cpp sans -DWITH_WIDGET)

Commande :
\`\`\`
g++ -std=c++17 -c Widget.cpp -o WidgetA.o
g++ -std=c++17 -c main.cpp -o mainA.o
\`\`\`

Message exact :
\`\`\`
main.cpp: In function 'int main()':
main.cpp:4:14: error: 'class Widget' has no member named 'GetValue'
    4 |     return w.GetValue();
\`\`\`

## Scenario B — main.cpp AVEC le define, Widget.cpp SANS (le vrai piege)

Commandes :
\`\`\`
g++ -std=c++17 -c Widget.cpp -o WidgetB.o
g++ -std=c++17 -DWITH_WIDGET -c main.cpp -o mainB.o
\`\`\`
Les deux compilent sans la moindre erreur. C'est a l'edition de liens que ca casse :
\`\`\`
g++ -std=c++17 mainB.o WidgetB.o -o progB.exe
\`\`\`

Message exact :
\`\`\`
C:/msys64/ucrt64/bin/../lib/gcc/x86_64-w64-mingw32/16.1.0/../../../../x86_64-w64-mingw32/bin/ld.exe: mainB.o:main.cpp:(.text+0x15): undefined reference to `Widget::Widget()'
C:/msys64/ucrt64/bin/../lib/gcc/x86_64-w64-mingw32/16.1.0/../../../../x86_64-w64-mingw32/bin/ld.exe: mainB.o:main.cpp:(.text+0x21): undefined reference to `Widget::GetValue() const'
collect2.exe: error: ld returned 1 exit status
\`\`\`

## Lequel des deux aurais-je su diagnostiquer sans cet exercice ?

Le scenario A, sans hesiter. Le message de compilation nomme directement le
probleme : "class Widget has no member named GetValue". N'importe quel
debutant en C++ comprend immediatement qu'il manque quelque chose a la
declaration de la classe, et va chercher du cote de la classe elle-meme.

Le scenario B est celui que je n'aurais pas su diagnostiquer avant cet
exercice. Le message ne parle que de symboles manquants
(`undefined reference to Widget::Widget()` et `Widget::GetValue() const`),
sans jamais mentionner ni `#define`, ni `WITH_WIDGET`, ni meme le fichier
Widget.hpp. Rien dans le message n'indique que la cause est une
incoherence de define ENTRE deux fichiers .cpp compiles separement : les
deux fichiers compilent chacun individuellement sans la moindre erreur, ce
qui masque completement le probleme jusqu'a l'edition de liens. Sans avoir
vu ce piege a l'avance, j'aurais probablement cherche du cote d'un
probleme de link classique (bibliotheque manquante, ordre de liens
incorrect) plutot que de penser a verifier que le meme define est bien
passe de maniere coherente a TOUTES les unites de compilation qui
partagent le meme en-tete conditionnel.
