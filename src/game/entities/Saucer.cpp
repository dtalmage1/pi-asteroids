#include "game/entities/Saucer.hpp"

namespace ast {

float Saucer::radius(SaucerSize s) noexcept {
    return (s == SaucerSize::Large) ? 12.0F : 6.0F;
}

} // namespace ast
