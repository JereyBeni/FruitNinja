// PS3 DualShock 3 / Sixaxis input translator (PSL1GHT ioPad).
// See InputTranslatorPS3.h for the two-role model.

#ifdef FRUIT_PLATFORM_PS3

#include "platform/ps3/InputTranslatorPS3.h"
#include "input/Touch.h"
#include "render/Layout.h"
#include "debug/DebugFlags.h"

#include <io/pad.h>
#include <cstring>
#include <cmath>
#include <cstdio>

namespace {

// Analog centre is 0x80. Deadzone in stick units (0..128).
const int kStickDeadzone = 24;

// Convert raw stick byte (0..255) to normalized delta around centre [-1,1].
float StickAxis(u8 raw)
{
    int v = static_cast<int>(raw) - 0x80;
    if (v > -kStickDeadzone && v < kStickDeadzone)
        return 0.0f;
    // Scale so full throw maps to ~1.0 outside the deadzone.
    const float range = 128.0f - static_cast<float>(kStickDeadzone);
    float f = static_cast<float>(v);
    if (f > 0.0f)
        f = (f - static_cast<float>(kStickDeadzone)) / range;
    else
        f = (f + static_cast<float>(kStickDeadzone)) / range;
    if (f < -1.0f) f = -1.0f;
    if (f >  1.0f) f =  1.0f;
    return f;
}

}  // namespace

InputTranslatorPS3::InputTranslatorPS3()
    : motionModeWasOn_(false)
{
    memset(fingerX, 0, sizeof(fingerX));
    memset(fingerY, 0, sizeof(fingerY));
    memset(fingerActive, 0, sizeof(fingerActive));
    memset(prevCutHeld, 0, sizeof(prevCutHeld));
    memset(padPresent, 0, sizeof(padPresent));
    memset(m_AimGX, 0, sizeof(m_AimGX));
    memset(m_AimGY, 0, sizeof(m_AimGY));
    memset(m_AimValid, 0, sizeof(m_AimValid));
}

void InputTranslatorPS3::Init()
{
    // Max ports PSL1GHT supports; we only drive the first MAX_PADS.
    if (ioPadInit(7) != 0) {
        printf("[PS3] ioPadInit failed\n");
        return;
    }

    padInfo info;
    if (ioPadGetInfo(&info) == 0) {
        printf("[PS3] pads connected: %u / max %u\n", info.connected, info.max);
        for (int i = 0; i < MAX_PADS; ++i) {
            if (info.status[i]) {
                ioPadSetSensorMode(i, 1);  // PAD_SENSOR_MODE_ON
                padPresent[i] = true;
            }
        }
    }
}

void InputTranslatorPS3::SetMotionMode(bool enabled)
{
    FN::g_MotionMode = enabled;
}

void InputTranslatorPS3::TransformStickNormalized(float nx, float ny, float& gx, float& gy)
{
    // nx,ny in [0,1], top-left origin, y-down -- same convention as SDL/Wii.
    Layout::TouchToGame(nx, ny, &gx, &gy);
}

void InputTranslatorPS3::PointerPressChannel(int ch, float gx, float gy)
{
    if (fingerActive[ch]) return;
    fingerActive[ch] = true;
    fingerX[ch] = gx;
    fingerY[ch] = gy;
    Mortar::Touch::GetInstance().OnPressed(ch + 1, gx, gy);
}

void InputTranslatorPS3::PointerReleaseChannel(int ch)
{
    if (!fingerActive[ch]) return;
    fingerActive[ch] = false;
    Mortar::Touch::GetInstance().OnReleased(ch + 1);
}

void InputTranslatorPS3::DrainPad(int port, float nx, float ny, bool cutHeld, bool connected)
{
    if (port < 0 || port >= MAX_PADS) return;

    const int pressCh = port;
    const int hoverCh = HOVER_CHANNEL_FIRST + port;

    if (!connected) {
        // Pad gone -- release both roles for this port.
        if (fingerActive[pressCh]) PointerReleaseChannel(pressCh);
        if (fingerActive[hoverCh]) PointerReleaseChannel(hoverCh);
        prevCutHeld[port] = false;
        m_AimValid[port] = false;
        return;
    }

    float gx = 0.0f, gy = 0.0f;
    TransformStickNormalized(nx, ny, gx, gy);

    m_AimGX[port] = gx;
    m_AimGY[port] = gy;
    m_AimValid[port] = true;

    // ---- Role 1: cut-button press finger (channels 0..3) ----
    {
        const bool down    = cutHeld;
        const bool wasDown = fingerActive[pressCh];

        if (down && !wasDown) {
            fingerX[pressCh] = gx;
            fingerY[pressCh] = gy;
            fingerActive[pressCh] = true;
            Mortar::Touch::GetInstance().OnPressed(pressCh + 1, gx, gy);
        } else if (down && wasDown) {
            if (gx != fingerX[pressCh] || gy != fingerY[pressCh]) {
                fingerX[pressCh] = gx;
                fingerY[pressCh] = gy;
                Mortar::Touch::GetInstance().OnMoved(pressCh + 1, gx, gy);
            }
        } else if (!down && wasDown) {
            fingerX[pressCh] = gx;
            fingerY[pressCh] = gy;
            fingerActive[pressCh] = false;
            Mortar::Touch::GetInstance().OnReleased(pressCh + 1);
        }
    }

    // ---- Role 2: motion-mode hover blade (channels 12..15) ----
    if (FN::g_MotionMode) {
        const bool wasCut = prevCutHeld[port];

        // Cut down-edge: lift hover blade (menu click rides Role 1).
        if (cutHeld && !wasCut) {
            PointerReleaseChannel(hoverCh);
        }

        // Cut up-edge: re-press hover at current stick position.
        if (!cutHeld && wasCut) {
            PointerPressChannel(hoverCh, gx, gy);
        }

        // Continuous hover while cut button is up.
        if (!cutHeld) {
            PointerPressChannel(hoverCh, gx, gy);
            if (gx != fingerX[hoverCh] || gy != fingerY[hoverCh]) {
                fingerX[hoverCh] = gx;
                fingerY[hoverCh] = gy;
                Mortar::Touch::GetInstance().OnMoved(hoverCh + 1, gx, gy);
            }
        }
    } else if (fingerActive[hoverCh]) {
        PointerReleaseChannel(hoverCh);
    }

    prevCutHeld[port] = cutHeld;
}

void InputTranslatorPS3::Poll()
{
    padInfo info;
    memset(&info, 0, sizeof(info));
    ioPadGetInfo(&info);

    for (int port = 0; port < MAX_PADS; ++port) {
        const bool connected = (info.status[port] != 0);
        padPresent[port] = connected;

        if (!connected) {
            DrainPad(port, 0.5f, 0.5f, false, false);
            continue;
        }

        padData data;
        memset(&data, 0, sizeof(data));
        if (ioPadGetData(port, &data) != 0 || data.len == 0) {
            DrainPad(port, 0.5f, 0.5f, false, true);
            continue;
        }

        // Left stick -> normalized screen position.
        // Stick at rest = centre of screen. Full throw reaches the edges.
        const float sx = StickAxis(data.ANA_L_H);
        const float sy = StickAxis(data.ANA_L_V);
        float nx = 0.5f + sx * 0.5f;
        float ny = 0.5f + sy * 0.5f;
        if (nx < 0.0f) nx = 0.0f;
        if (nx > 1.0f) nx = 1.0f;
        if (ny < 0.0f) ny = 0.0f;
        if (ny > 1.0f) ny = 1.0f;

        // Cut / click: Cross or R2 (digital). Pressure-sensitive R2 can be
        // added later via PRESS_R2 if desired.
        const bool cutHeld = (data.BTN_CROSS != 0) || (data.BTN_R2 != 0);

        // Sixaxis values are available if we want tilt-aim later:
        // data.SENSOR_X / Y / Z / G  (0x0000..0x03FF, centre ~0x1FF)
        (void)data.SENSOR_X;

        DrainPad(port, nx, ny, cutHeld, true);
    }
}

void InputTranslatorPS3::DispatchForSimTick()
{
    Mortar::Touch::GetInstance().Update(0.0f);

    // If motion mode was toggled off, ensure hover blades are released.
    if (motionModeWasOn_ && !FN::g_MotionMode) {
        for (int port = 0; port < MAX_PADS; ++port) {
            PointerReleaseChannel(HOVER_CHANNEL_FIRST + port);
        }
    }
    motionModeWasOn_ = FN::g_MotionMode;
}

void InputTranslatorPS3::ReleaseAllFingers()
{
    for (int ch = 0; ch < CHANNEL_COUNT; ++ch) {
        if (!fingerActive[ch]) continue;
        Mortar::Touch::GetInstance().OnReleased(ch + 1);
        fingerActive[ch] = false;
    }
    Mortar::Touch::GetInstance().Update(0.0f);
    memset(prevCutHeld, 0, sizeof(prevCutHeld));
}

#endif // FRUIT_PLATFORM_PS3
