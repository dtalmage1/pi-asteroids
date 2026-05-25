#pragma once

namespace ast {

struct InputState {
    bool thrust      = false;
    bool rotateLeft  = false;
    bool rotateRight = false;
    bool fire        = false;
    bool hyperspace  = false;
    bool start       = false;
    bool connected   = false;
};

} // namespace ast
