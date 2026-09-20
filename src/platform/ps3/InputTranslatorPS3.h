#ifndef FN_PLATFORM_PS3_INPUT_TRANSLATOR_PS3_H
#define FN_PLATFORM_PS3_INPUT_TRANSLATOR_PS3_H

//
// InputTranslatorPS3 -- DualShock 3 / Sixaxis (PSL1GHT ioPad) -> Mortar::Touch.
//
// Mirrors the two-role model used by InputTranslatorWii / InputTranslatorSDL:
//
//   Role 1 -- "press finger" (channels 0..3, one per pad):
//     Active while Cross OR R2 is held. Feeds Mortar::Touch so menus/widgets
//     receive real clicks in BOTH modes. When motion mode is OFF this is also
//     the blade (press-to-cut). When motion mode is ON the slice is speed-gated
//     (same as Wii Role 1).
//
//   Role 2 -- "hover blade" (channels 12..15):
//     Live only while FN::g_MotionMode is ON and the cut button is NOT held.
//     Blade tracks the left stick continuously; cuts are speed-gated by
//     SlashEntity. Pressing Cross/R2 lifts the hover blade (menu click rides
//     Role 1 instead) so there is never a double blade.
//
// Stick mapping:
//   Left analog stick -> pointer position in normalized [0,1] screen space
//   (centre = 0.5,0.5). Deadzone applied. Transformed via Layout::TouchToGame.
//
// Sixaxis tilt is read (sensor mode ON) and available for a future motion
// aim assist; the primary aim is always the left stick for predictable play
// on RPCS3 and real hardware.
//
// Only compiled when FRUIT_PLATFORM_PS3 is set.

#ifdef FRUIT_PLATFORM_PS3

#include <cstdint>

class InputTranslatorPS3 {
public:
    static const int MAX_PADS      = 4;   // practical multiplayer ceiling
    static const int CHANNEL_COUNT = 16;  // same space as SDL / Wii

    // Hover blade channels (same numbers as Wii pointer channels).
    static const int HOVER_CHANNEL_FIRST = 12;

    InputTranslatorPS3();

    // ioPadInit + enable sensors on connected ports. Call once at boot.
    void Init();

    // Poll all pads, push Role 1 / Role 2 into Mortar::Touch.
    // Call once per display frame from the main loop.
    void Poll();

    // Drain Mortar::Touch for one sim tick. Call before Game::stepUpdate().
    void DispatchForSimTick();

    // Release every held channel (suspend / XMB exit).
    void ReleaseAllFingers();

    // Optional: force motion mode from platform code (Settings also writes
    // FN::g_MotionMode directly).
    void SetMotionMode(bool enabled);

private:
    float fingerX[CHANNEL_COUNT];
    float fingerY[CHANNEL_COUNT];
    bool  fingerActive[CHANNEL_COUNT];

    bool  prevCutHeld[MAX_PADS];
    bool  padPresent[MAX_PADS];

    // Last aim position per pad (game-space), for GetPointer-style overlays later.
    float m_AimGX[MAX_PADS];
    float m_AimGY[MAX_PADS];
    bool  m_AimValid[MAX_PADS];

    bool motionModeWasOn_;

    void TransformStickNormalized(float nx, float ny, float& gx, float& gy);
    void PointerPressChannel(int ch, float gx, float gy);
    void PointerReleaseChannel(int ch);
    void DrainPad(int port, float nx, float ny, bool cutHeld, bool connected);
};

#endif // FRUIT_PLATFORM_PS3

#endif // FN_PLATFORM_PS3_INPUT_TRANSLATOR_PS3_H
