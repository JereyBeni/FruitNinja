#ifndef FN_PLATFORM_XBOX360_INPUT_TRANSLATOR_H
#define FN_PLATFORM_XBOX360_INPUT_TRANSLATOR_H

#include "xsdk/kernel.h"

class InputTranslatorXbox360 {
public:
    InputTranslatorXbox360();

    void Init();
    void DrainController(const INPUT_STATE& state);
    void DispatchForSimTick();
    void ReleaseAll();

private:
    float m_cursorX;
    float m_cursorY;
    float m_gameX;
    float m_gameY;
    bool m_pressed;
};

#endif
