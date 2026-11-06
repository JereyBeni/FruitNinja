#include "render/DisplayManager.h"
#include "platform/xbox360/Xbox360Video.h"

#include "xsdk/gpu.h"
#include "xsdk/present.h"

#include <cstdint>

namespace {
uint32_t s_width = 0;
uint32_t s_height = 0;
uint32_t s_frameAddress = 0;
bool s_frameReady = false;
constexpr uint32_t kBeginTimeoutMs = 5000;
constexpr uint32_t kClearArgb = 0xFF000000;
}

namespace fn {
namespace xbox360 {

void ConfigurePresentation(uint32_t width, uint32_t height)
{
    s_width = width;
    s_height = height;
    s_frameAddress = 0;
    s_frameReady = false;
}

bool BeginPresentFrame()
{
    s_frameReady = false;
    if (!s_width || !s_height) return false;

    const NTSTATUS status = xsdk_present_begin(&s_frameAddress, kBeginTimeoutMs);
    if (!NT_SUCCESS(status)) return false;

    xsdk_gpu_set_target(s_width, s_height);
    s_frameReady = true;
    return true;
}

uint32_t CurrentFrameAddress() { return s_frameAddress; }
uint32_t DisplayWidth() { return s_width; }
uint32_t DisplayHeight() { return s_height; }

} // namespace xbox360
} // namespace fn

namespace Mortar {

void DisplayManager::SwapBuffers(void*)
{
    if (!s_frameReady) return;

    xsdk_gpu_resolve(
        fn::xbox360::CurrentFrameAddress(),
        fn::xbox360::DisplayWidth(),
        fn::xbox360::DisplayHeight(),
        kClearArgb);
    xsdk_present_end();
    s_frameReady = false;
    m_bSwapPending = m_bSwapPending ? 0 : 1;
}

} // namespace Mortar
