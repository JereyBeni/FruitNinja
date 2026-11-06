#include "platform/xbox360/InputTranslatorXbox360.h"

#include "input/Touch.h"
#include "render/Layout.h"

#include <algorithm>
#include <cmath>

namespace {
constexpr int kTouchExternalId = 1;
constexpr int16_t kDeadzone = 6000;
constexpr uint8_t kTriggerThreshold = 96;

float Axis(int16_t value)
{
    if (value > -kDeadzone && value < kDeadzone) return 0.0f;
    const float scaled = static_cast<float>(value) / 32767.0f;
    return std::max(-1.0f, std::min(1.0f, scaled));
}
}

InputTranslatorXbox360::InputTranslatorXbox360()
    : m_cursorX(0.5f), m_cursorY(0.5f), m_gameX(0.0f), m_gameY(0.0f), m_pressed(false)
{
}

void InputTranslatorXbox360::Init()
{
    Layout::TouchToGame(m_cursorX, m_cursorY, &m_gameX, &m_gameY);
}

void InputTranslatorXbox360::DrainController(const INPUT_STATE& state)
{
    const INPUT_GAMEPAD& pad = state.gamepad;
    const float dx = Axis(pad.thumb_lx);
    const float dy = Axis(pad.thumb_ly);

    m_cursorX = std::max(0.0f, std::min(1.0f, m_cursorX + dx * 0.025f));
    m_cursorY = std::max(0.0f, std::min(1.0f, m_cursorY - dy * 0.025f));
    Layout::TouchToGame(m_cursorX, m_cursorY, &m_gameX, &m_gameY);

    // A is the menu/click action; right trigger is the gameplay slice action.
    const bool pressed =
        (pad.buttons & INPUT_GAMEPAD_A) != 0 || pad.right_trigger >= kTriggerThreshold;

    Mortar::Touch& touch = Mortar::Touch::GetInstance();
    if (pressed && !m_pressed) {
        touch.OnPressed(kTouchExternalId, m_gameX, m_gameY);
    } else if (pressed && m_pressed) {
        touch.OnMoved(kTouchExternalId, m_gameX, m_gameY);
    } else if (!pressed && m_pressed) {
        touch.OnReleased(kTouchExternalId);
    }
    m_pressed = pressed;
}

void InputTranslatorXbox360::DispatchForSimTick()
{
    Mortar::Touch::GetInstance().Update(0.0f);
}

void InputTranslatorXbox360::ReleaseAll()
{
    if (m_pressed) {
        Mortar::Touch::GetInstance().OnReleased(kTouchExternalId);
        m_pressed = false;
    }
    Mortar::Touch::GetInstance().Update(0.0f);
}
