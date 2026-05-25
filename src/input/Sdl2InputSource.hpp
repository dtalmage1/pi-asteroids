#pragma once
#include <memory>
#include <SDL.h>
#include "game/IInputSource.hpp"

namespace ast {

struct SdlJoystickDeleter {
    void operator()(SDL_Joystick* j) const;
};

class Sdl2InputSource : public IInputSource {
public:
    Sdl2InputSource();
    ~Sdl2InputSource() override;

    Sdl2InputSource(const Sdl2InputSource&)            = delete;
    Sdl2InputSource& operator=(const Sdl2InputSource&) = delete;
    Sdl2InputSource(Sdl2InputSource&&)                 = delete;
    Sdl2InputSource& operator=(Sdl2InputSource&&)      = delete;

    InputState query() const override;

private:
    std::unique_ptr<SDL_Joystick, SdlJoystickDeleter> joystick_;
};

} // namespace ast
