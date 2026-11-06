#ifndef FN_PLATFORM_XBOX360_VIDEO_H
#define FN_PLATFORM_XBOX360_VIDEO_H

#include <stdint.h>

namespace fn {
namespace xbox360 {

void ConfigurePresentation(uint32_t width, uint32_t height);
bool BeginPresentFrame();
uint32_t CurrentFrameAddress();
uint32_t DisplayWidth();
uint32_t DisplayHeight();

} // namespace xbox360
} // namespace fn

#endif
