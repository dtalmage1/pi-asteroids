#include "input/Sdl2InputSource.hpp"

// SHANWAN Android Gamepad (verified with: jstest /dev/input/js0)
// Axes:    0=LeftX  1=LeftY  6=Hat0X  7=Hat0Y
// Buttons: 1=BtnB(O/fire)  6=BtnTL(L-shoulder/hyperspace)  11=BtnStart
namespace {
    constexpr int   kAxisLeftX    = 0;      // left joystick horizontal
    constexpr int   kAxisLeftY    = 1;      // left joystick vertical
    constexpr Sint16 kDeadZone    = 8000;   // ignore drift below this magnitude
    constexpr int   kButtonFire   = 1;      // BtnB — right face button (O)
    constexpr int   kButtonHyper  = 6;      // BtnTL — left shoulder
    constexpr int   kButtonStart  = 11;     // BtnStart
} // namespace

namespace ast {

void SdlJoystickDeleter::operator()(SDL_Joystick* j) const {
    SDL_JoystickClose(j);
}

Sdl2InputSource::Sdl2InputSource()
    : joystick_(SDL_NumJoysticks() > 0 ? SDL_JoystickOpen(0) : nullptr)
{}

Sdl2InputSource::~Sdl2InputSource() = default;

void Sdl2InputSource::handleEvent(const SDL_Event& event) {
    if (event.type == SDL_JOYDEVICEADDED) {
        if (!joystick_) {
            joystick_.reset(SDL_JoystickOpen(static_cast<int>(event.jdevice.which)));
        }
    } else if (event.type == SDL_JOYDEVICEREMOVED) {
        if (joystick_ &&
            SDL_JoystickInstanceID(joystick_.get()) == event.jdevice.which) {
            joystick_.reset();
        }
    }
}

InputState Sdl2InputSource::query() const {
    InputState state;

    if (!joystick_ || SDL_JoystickGetAttached(joystick_.get()) == SDL_FALSE) {
        return state;  // connected defaults to false
    }

    state.connected = true;

    const auto axisX = SDL_JoystickGetAxis(joystick_.get(), kAxisLeftX);
    const auto axisY = SDL_JoystickGetAxis(joystick_.get(), kAxisLeftY);
    state.rotateLeft  = axisX < -kDeadZone;
    state.rotateRight = axisX >  kDeadZone;
    state.thrust      = axisY < -kDeadZone;  // push up = negative Y
    state.fire        = SDL_JoystickGetButton(joystick_.get(), kButtonFire)  != 0;
    state.hyperspace  = SDL_JoystickGetButton(joystick_.get(), kButtonHyper) != 0;
    state.start       = SDL_JoystickGetButton(joystick_.get(), kButtonStart) != 0;

    return state;
}

} // namespace ast
