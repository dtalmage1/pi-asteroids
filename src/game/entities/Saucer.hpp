#pragma once
#include "game/Vec2.hpp"

namespace ast {

enum class SaucerSize { Large, Small };

struct Saucer {
    Vec2       position  {};
    Vec2       velocity  {};
    SaucerSize size      = SaucerSize::Large;
    float      fireTimer = 0.0F;
    bool       active    = false;

    static float radius(SaucerSize s) noexcept;
};

} // namespace ast
