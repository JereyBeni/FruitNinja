#ifdef FRUIT_PLATFORM_XBOX360

#include "Game.h"
#include "config.h"
#include "platform/FixedStepDriver.h"
#include "platform/xbox360/InputTranslatorXbox360.h"
#include "platform/xbox360/Xbox360Video.h"
#include "game/SettingsSave.h"
#include "debug/Logger.h"

#include "xsdk/gpu.h"
#include "xsdk/kernel.h"
#include "xsdk/log.h"
#include "xsdk/present.h"
#include "xsdk/splash.h"
#include "xsdk/xam.h"

#include <chrono>
#include <cstdint>

namespace {
constexpr uint32_t kPresentBuffers = 3;
constexpr uint32_t kSwapInterval = 1;
constexpr uint32_t kTitleSplashMs = 1200;

Game g_game;
fn::FixedStepDriver g_driver;
InputTranslatorXbox360 g_input;

bool MountSave(XsdkSave& save)
{
    uint32_t userIndex = XSDK_XAM_USER_NONE;
    char gamertag[XSDK_XAM_GAMERTAG_BYTES] = {};
    const uint32_t userResult = xsdk_xam_signed_in_user(&userIndex, gamertag);
    if (userResult != ERROR_SUCCESS && userResult != ERROR_NO_SUCH_USER) {
        xsdk_log_write("FruitNinja360: XAM user lookup failed: 0x%08X", userResult);
        return false;
    }

    uint32_t deviceId = 0;
    const uint32_t deviceResult = xsdk_save_device(&deviceId);
    if (deviceResult != ERROR_SUCCESS) {
        xsdk_log_write("FruitNinja360: no HDD save device: 0x%08X", deviceResult);
        return false;
    }

    static const uint16_t displayName[] = {
        'F','r','u','i','t',' ','N','i','n','j','a',0
    };
    uint32_t result = xsdk_save_mount(&save, userIndex, "save", "fruitninja", displayName);
    if (result == ERROR_IO_PENDING) {
        result = xsdk_save_wait(&save, 5000);
    }
    if (result != ERROR_SUCCESS) {
        xsdk_log_write("FruitNinja360: save mount failed: 0x%08X", result);
        return false;
    }
    xsdk_log_write("FruitNinja360: save package mounted for user %u", userIndex);
    return true;
}

void UnmountSave(XsdkSave& save)
{
    uint32_t result = xsdk_save_unmount(&save);
    if (result == ERROR_IO_PENDING) result = xsdk_save_wait(&save, 5000);
    if (result != ERROR_SUCCESS)
        xsdk_log_write("FruitNinja360: save flush/unmount failed: 0x%08X", result);
}
}

int main()
{
    xsdk_log_write("FruitNinja360: starting native Xenos game port");
    const NTSTATUS splashStatus = xsdk_splash_show(kTitleSplashMs);
    if (!NT_SUCCESS(splashStatus))
        xsdk_log_write("FruitNinja360: splash skipped: 0x%08X", static_cast<unsigned>(splashStatus));

    NTSTATUS status = xsdk_gpu_open();
    if (!NT_SUCCESS(status)) {
        xsdk_log_write("FruitNinja360: GPU init failed: 0x%08X", static_cast<unsigned>(status));
        return 1;
    }

    uint32_t width = 0, height = 0;
    status = xsdk_present_open(XSDK_DISPLAY_720P, kPresentBuffers, kSwapInterval, &width, &height);
    if (!NT_SUCCESS(status)) {
        xsdk_log_write("FruitNinja360: display init failed: 0x%08X", static_cast<unsigned>(status));
        xsdk_gpu_close();
        return 1;
    }
    fn::xbox360::ConfigurePresentation(width, height);

    XsdkSave save = {};
    const bool saveMounted = MountSave(save);

    int exitCode = 0;
    if (!g_game.init(nullptr, nullptr)) {
        xsdk_log_write("FruitNinja360: Game::init failed");
        exitCode = 1;
    } else {
        g_input.Init();
        auto previous = std::chrono::steady_clock::now();

        while (g_game.running) {
            INPUT_STATE state = {};
            if (XamInputGetState(0, 0, &state) == ERROR_SUCCESS) {
                if (state.gamepad.buttons & INPUT_GAMEPAD_BACK) {
                    g_game.running = false;
                    break;
                }
                g_input.DrainController(state);
            } else {
                g_input.ReleaseAll();
            }

            const auto now = std::chrono::steady_clock::now();
            const double elapsedMs =
                std::chrono::duration<double, std::milli>(now - previous).count();
            previous = now;

            const int steps = g_driver.advance(elapsedMs);
            for (int i = 0; i < steps && g_game.running; ++i) {
                g_input.DispatchForSimTick();
                g_game.stepUpdate();
#if defined(FN_RENDER_INTERP) && FN_RENDER_INTERP
                fn::RenderInterp::Get().SnapshotAfterStep();
#endif
            }

            g_game.tickRealtimeUi(static_cast<float>(elapsedMs / 1000.0));
            g_game.renderFrame(static_cast<float>(g_driver.alpha()), steps);
        }

        g_input.ReleaseAll();
        SaveSettings();
        g_game.shutdown();
    }

    if (saveMounted) UnmountSave(save);
    xsdk_present_close();
    status = xsdk_gpu_close();
    if (!NT_SUCCESS(status)) {
        xsdk_log_write("FruitNinja360: GPU shutdown failed: 0x%08X", static_cast<unsigned>(status));
        exitCode = 1;
    }

    xsdk_log_write("FruitNinja360: exit %d", exitCode);
    return exitCode;
}

#endif
