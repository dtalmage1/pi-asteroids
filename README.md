# Asteroids

A faithful Asteroids arcade clone running on a Raspberry Pi 400, controlled via the
PiHut wireless USB gamepad. Written in C++17 using SDL2 for rendering, input, and audio.

## Controls

| Action      | Button                        |
|-------------|-------------------------------|
| Rotate left | Left joystick — left          |
| Rotate right| Left joystick — right         |
| Thrust      | Left joystick — up            |
| Fire        | O button (right face)         |
| Hyperspace  | Left shoulder button          |
| Start       | Start button                  |

## Build prerequisites

- CMake 3.24 or later
- A C++17 compiler (MSVC on Windows, GCC on Raspberry Pi OS)
- SDL2 and SDL2_mixer are fetched automatically via CMake FetchContent
- On Raspberry Pi: `sudo apt install libasound2-dev` (required for SDL2 ALSA audio support)

## Building on Raspberry Pi

```bash
cmake -B build
cmake --build build
```

## Building on Windows (host / development)

Open a Visual Studio 2022 Developer Command Prompt, then:

```powershell
cmd /c '"C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat" >nul 2>&1 && cmake -G Ninja -B build .'
cmd /c '"C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat" >nul 2>&1 && cmake --build build'
```

## Running

```bash
./build/asteroids
```

Optional flags:

```
--width  <pixels>    Window width  (default: 800)
--height <pixels>    Window height (default: 600)
--fullscreen         Fullscreen mode
```

On the Raspberry Pi, fullscreen at native resolution:

```bash
./build/asteroids --fullscreen
```

## Audio setup (Raspberry Pi)

SDL2 uses the ALSA `default` device. If your display is connected to HDMI port 1
(the port labelled HDMI on the Pi 400), create `~/.asoundrc` to route audio there:

```
pcm.!default {
    type plug
    slave.pcm "hw:1,0"
}
ctl.!default {
    type hw
    card 1
}
```

If your display is on HDMI port 0, use `hw:0,0` and `card 0` instead.

## Running tests

```bash
ctest --test-dir build --output-on-failure
```

## Deploying to Raspberry Pi

```bash
bash scripts/deploy.sh
```

Then build and run on the Pi:

```bash
ssh dan@dtdan "cd ~/Documents/Projects/Asteroids && cmake --build build"
ssh dan@dtdan "cd ~/Documents/Projects/Asteroids/build && ./asteroids --fullscreen"
```
