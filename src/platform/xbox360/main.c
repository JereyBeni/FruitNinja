#include <stdbool.h>
#include <stdint.h>

#include "xsdk/gpu.h"
#include "xsdk/kernel.h"
#include "xsdk/log.h"
#include "xsdk/present.h"
#include "xsdk/splash.h"

enum {
    BUFFER_COUNT = 2,
    SWAP_INTERVAL = 1,
    PRESENT_TIMEOUT_MS = 5000,
    CLEAR_ARGB = 0xFF18243A
};

static int run(uint32_t width, uint32_t height)
{
    for (;;) {
        INPUT_STATE input = { 0 };
        if (XamInputGetState(0, 0, &input) == ERROR_SUCCESS &&
            (input.gamepad.buttons & INPUT_GAMEPAD_BACK)) {
            break;
        }

        uint32_t address = 0;
        NTSTATUS status = xsdk_present_begin(&address, PRESENT_TIMEOUT_MS);
        if (!NT_SUCCESS(status)) {
            xsdk_log_write("FruitNinja360: present_begin failed: 0x%08X",
                (unsigned)status);
            return 1;
        }

        xsdk_gpu_resolve(address, width, height, CLEAR_ARGB);
        xsdk_present_end();
    }

    return 0;
}

int main(void)
{
    xsdk_log_write("FruitNinja360: Xbox 360 platform bring-up");

    NTSTATUS status = xsdk_splash_show(1500);
    if (!NT_SUCCESS(status)) {
        xsdk_log_write("FruitNinja360: splash failed: 0x%08X", (unsigned)status);
    }

    status = xsdk_gpu_open();
    if (!NT_SUCCESS(status)) {
        xsdk_log_write("FruitNinja360: gpu_open failed: 0x%08X", (unsigned)status);
        return 1;
    }

    uint32_t width = 0;
    uint32_t height = 0;
    status = xsdk_present_open(
        XSDK_DISPLAY_720P,
        BUFFER_COUNT,
        SWAP_INTERVAL,
        &width,
        &height
    );
    if (!NT_SUCCESS(status)) {
        xsdk_log_write("FruitNinja360: present_open failed: 0x%08X",
            (unsigned)status);
        xsdk_gpu_close();
        return 1;
    }

    xsdk_log_write("FruitNinja360: display opened at %ux%u",
        (unsigned)width, (unsigned)height);

    const int result = run(width, height);

    xsdk_present_close();
    status = xsdk_gpu_close();
    if (!NT_SUCCESS(status)) {
        xsdk_log_write("FruitNinja360: gpu_close failed: 0x%08X",
            (unsigned)status);
        return 1;
    }

    xsdk_log_write("FruitNinja360: platform bring-up ended");
    return result;
}
