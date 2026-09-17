#include "Directions.hpp"

NkVec3f Avant() {
    return NkVec3f{0.0f, 0.0f, -1.0f};
}

NkVec3f Haut() {
    return NkVec3f{0.0f, 1.0f, 0.0f};
}

NkVec3f Droite() {
    return NkVec3f{1.0f, 0.0f, 0.0f};
}
