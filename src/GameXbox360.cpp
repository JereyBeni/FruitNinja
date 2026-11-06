#ifdef FRUIT_PLATFORM_XBOX360

#include "Game.h"
#include "game/GameWork.h"
#include "game/GameTaskState.h"
#include "render/DisplayManager.h"
#include "core/SystemManager.h"
#include "asset/TextureManager.h"
#include "debug/DebugFlags.h"
#include "debug/Logger.h"
#include "debug/OSD.h"
#include "config.h"
#include "render/gl_funcs.h"
#include "render/Layout.h"
#include "platform/RenderInterp.h"
#include "platform/xbox360/Xbox360Video.h"
#include "game/SettingsSave.h"

static float s_currentFps = 0.0f;

bool Game::init(void* win, void* gl)
{
    window = win;
    gl_context = gl;

    data_dir = FN_DATA_DIR;
    save_dir = FN_SAVE_DIR;
    Mortar::TextureManager::SetDataDir(data_dir.c_str());
    LoadSettings();

    const uint32_t width = fn::xbox360::DisplayWidth();
    const uint32_t height = fn::xbox360::DisplayHeight();
    if (!width || !height) {
        LOG_ERROR("GameXbox360", "XSDK presentation has no display dimensions");
        return false;
    }

    Mortar::DisplayManager::GetInstance().SetWindowSize(0, FN_SCREEN_H, 0, FN_SCREEN_W);
    if (!renderer.init()) {
        LOG_ERROR("GameXbox360", "Failed to initialise Xenos renderer");
        return false;
    }

    renderer.InitGL(static_cast<int>(width), static_cast<int>(height));
    GamePreInitialise();
    SetHardware("XBOX360", true);
    GameInitialise(nullptr, nullptr);

    if (!m_StartupTexture) {
        m_StartupTexture = Mortar::TextureManager::LoadLocalisedTexture("HB_logo.tex");
    }

    game_work.taskStateIndex = 0;
    running = true;
    return true;
}

void Game::pollInput() {}

void Game::stepUpdate()
{
    ++Debug::g_LogTick;
    game_work.dt = 0.0f;
    SystemManager::GetInstance().Update(&game_work.dt);
    game_work.dt *= FN::g_DebugTimeScale;
    GameTaskUpdate(game_work.dt);
}

void Game::setCurrentFps(float fps)
{
    s_currentFps = fps;
}

void Game::renderFrame(float alpha, int)
{
    const int width = static_cast<int>(fn::xbox360::DisplayWidth());
    const int height = static_cast<int>(fn::xbox360::DisplayHeight());
    if (width <= 0 || height <= 0 || !fn::xbox360::BeginPresentFrame()) {
        LOG_ERROR("GameXbox360", "Could not acquire an XSDK presentation buffer");
        running = false;
        return;
    }

    Layout::SetWindowAspect(16.0f, 9.0f);
    int vpX = 0, vpY = 0, vpW = width, vpH = height;
    Layout::ComputeViewportFitAlways(width, height, &vpX, &vpY, &vpW, &vpH);
    glViewport(vpX, vpY, vpW, vpH);
    Layout::SetActiveViewport(vpX, vpY, vpW, vpH, width, height);

    Mortar::DisplayManager::GetInstance().BeginFrame();

#if defined(FN_RENDER_INTERP) && FN_RENDER_INTERP
    fn::RenderInterp::Get().ApplyForDraw(alpha);
#endif
    const float savedDt = game_work.dt;
    GameTaskDraw(game_work.dt);
    game_work.dt = savedDt;
#if defined(FN_RENDER_INTERP) && FN_RENDER_INTERP
    fn::RenderInterp::Get().RestoreAfterDraw();
#endif

    FN::DebugFps_Draw(s_currentFps);
    OSD_Update(0.0f);
    OSD_Draw();

    Mortar::DisplayManager::GetInstance().SwapBuffers(window);
}

void Game::frameTick()
{
    pollInput();
    stepUpdate();
    renderFrame(0.0f, 1);
}

void Game::run()
{
    while (running) frameTick();
}

void Game::runFrames(int frameCount)
{
    for (int i = 0; i < frameCount && running; ++i) {
        stepUpdate();
        renderFrame(0.0f, 1);
    }
}

#endif
