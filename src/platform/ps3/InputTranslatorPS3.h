#pragma once

// PS3 input translator (Sixaxis / pad).
// Scaffolding only -- real implementation still TODO.
// Mirrors the shape of InputTranslatorWii / InputTranslatorSDL.

#include "platform/InputEvent.h"

namespace fn {
namespace ps3 {

class InputTranslatorPS3 {
public:
    InputTranslatorPS3();
    ~InputTranslatorPS3();

    // Poll pad state and emit Mortar::Touch / button events into the
    // shared InputManager. Called once per frame from the main loop.
    void Poll();

    // Optional: enable/disable motion or pointer-style aiming later.
    void SetMotionMode(bool enabled);

private:
    bool motion_mode_;
    // TODO: pad handles, previous button state, pointer coords, etc.
};

}  // namespace ps3
}  // namespace fn
