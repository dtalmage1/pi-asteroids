#pragma once
#include "game/Vec2.hpp"

namespace ast {

inline bool circlesOverlap(Vec2 aPos, float aRadius, Vec2 bPos, float bRadius) {
    return Vec2::distance(aPos, bPos) < aRadius + bRadius;
}

} // namespace ast
