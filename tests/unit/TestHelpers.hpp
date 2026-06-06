#pragma once
#include "game/Game.hpp"

namespace ast::test {

inline void startGame(Game& game) {
    InputState input;
    input.start     = true;
    input.connected = true;
    game.update(1.0F / 60.0F, input);
}

inline void tickFrames(Game& game, int n) {
    const InputState noInput;
    for (int i = 0; i < n; ++i) { game.update(1.0F / 60.0F, noInput); }
}

} // namespace ast::test
