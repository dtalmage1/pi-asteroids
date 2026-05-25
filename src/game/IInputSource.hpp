#pragma once
#include "game/InputState.hpp"

namespace ast {

class IInputSource {
public:
    virtual ~IInputSource() = default;

    IInputSource()                                 = default;
    IInputSource(const IInputSource&)             = delete;
    IInputSource& operator=(const IInputSource&)  = delete;
    IInputSource(IInputSource&&)                  = delete;
    IInputSource& operator=(IInputSource&&)       = delete;

    virtual InputState query() const = 0;
};

} // namespace ast
