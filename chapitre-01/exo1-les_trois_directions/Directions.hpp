#pragma once

#include "NkVec3f.hpp"

/**
 * Vecteur unitaire pointant vers l'avant : -Z, convention du module (main
 * droite, style OpenXR).
 *
 * Fixe ici plutot que code en dur a chaque usage : le +Z "avant" d'un moteur
 * classique generique peut etre l'arriere d'une pose XR. Une convention ne
 * se retient pas, elle s'ecrit dans une fonction, une fois pour toutes.
 */
NkVec3f Avant();

/**
 * Vecteur unitaire pointant vers le haut : +Y, convention du module (main
 * droite, style OpenXR).
 *
 * Fixe ici plutot que code en dur a chaque usage, pour la meme raison que
 * Avant() : le sens de chaque axe doit venir d'un seul endroit du code.
 */
NkVec3f Haut();

/**
 * Vecteur unitaire pointant vers la droite : +X, convention du module (main
 * droite, style OpenXR).
 *
 * Fixe ici plutot que code en dur a chaque usage, pour la meme raison que
 * Avant() : le sens de chaque axe doit venir d'un seul endroit du code.
 */
NkVec3f Droite();
