#include "input/Sdl2InputSource.hpp"

// PiHut SNES-style USB gamepad button indices.
// Verify with: jstest /dev/input/js0
// Hat 0 is the D-pad; axis values use SDL_HAT_* masks.
namespace {
    constexpr int kHatIndex    = 0;
    constexpr int kButtonB     = 0; // bottom face — secondary thrust
    constexpr int kButtonA     = 1; // right face  — fire
    constexpr int kButtonX     = 3; // top face    — hyperspace
    constexpr int kButtonStart = 7; // start
} // namespace

namespace ast {

void SdlJoystickDeleter::operator()(SDL_Joystick* j) const {
    SDL_JoystickClose(j);
}

Sdl2InputSource::Sdl2InputSource()
    : joystick_(SDL_NumJoysticks() > 0 ? SDL_JoystickOpen(0) : nullptr)
{}

Sdl2InputSource::~Sdl2InputSource() = default;

InputState Sdl2InputSource::query() const {
    InputState state;
    state.connected = true; // hot-plug detection deferred to POL-1

    if (!joystick_) {
        return state;
    }

    const auto hat = SDL_JoystickGetHat(joystick_.get(), kHatIndex);
    state.rotateLeft  = (hat & SDL_HAT_LEFT)  != 0;
    state.rotateRight = (hat & SDL_HAT_RIGHT) != 0;
    // D-pad up OR face button B both trigger thrust (classic arcade feel)
    state.thrust     = ((hat & SDL_HAT_UP) != 0)
                    || (SDL_JoystickGetButton(joystick_.get(), kButtonB) != 0);
    state.fire       = SDL_JoystickGetButton(joystick_.get(), kButtonA) != 0;
    state.hyperspace = SDL_JoystickGetButton(joystick_.get(), kButtonX) != 0;
    state.start      = SDL_JoystickGetButton(joystick_.get(), kButtonStart) != 0;

    return state;
}

} // namespace ast
