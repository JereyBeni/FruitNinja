// PS3 input translator -- scaffolding stub.
// Real Sixaxis / libpad integration still TODO.

#include "platform/ps3/InputTranslatorPS3.h"

namespace fn {
namespace ps3 {

InputTranslatorPS3::InputTranslatorPS3()
    : motion_mode_(true)
{
    // TODO: ioPadInit / cellPadInit, open ports, etc.
}

InputTranslatorPS3::~InputTranslatorPS3()
{
    // TODO: shutdown pad
}

void InputTranslatorPS3::Poll()
{
    // TODO:
    // 1. Read pad data (buttons + sticks + Sixaxis accel if useful).
    // 2. Map to Mortar::Touch finger events (or motion-mode point+flick).
    // 3. Push into InputManager / shared event queue.
    // For now this is a no-op so the binary can still link.
}

void InputTranslatorPS3::SetMotionMode(bool enabled)
{
    motion_mode_ = enabled;
}

}  // namespace ps3
}  // namespace fn
