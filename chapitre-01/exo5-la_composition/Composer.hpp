#pragma once

#include "NkQuatf.hpp"
#include "NkVec3f.hpp"
#include "Pose.hpp"

/**
 * Compose une pose parent P avec une pose enfant C : produit la pose unique
 * equivalente a "appliquer C, puis appliquer P".
 *
 * Exemple du chapitre 2 : la main est dans le repere du coude, lui-meme
 * dans celui de l'epaule. Plutot que d'appliquer deux poses separement a
 * chaque point de la main (une fois pour le coude, une fois pour l'epaule),
 * Composer() precalcule une seule pose equivalente, reutilisable pour
 * n'importe quel point sans repeter les deux etapes :
 *   ApplyPose(Composer(P, C), point) == ApplyPose(P, ApplyPose(C, point))
 *
 * La position composee est la position de l'enfant exprimee dans le monde
 * via la pose parent (ApplyPose), et l'orientation composee est le produit
 * des deux rotations (Multiply) -- combiner les rotations d'abord, avant de
 * placer le tout dans le monde.
 */
inline Pose Composer(const Pose& parent, const Pose& child) {
    NkVec3f pos = ApplyPose(parent, child.position);
    NkQuatf rot = Multiply(parent.orientation, child.orientation);
    return Pose{pos, rot};
}
