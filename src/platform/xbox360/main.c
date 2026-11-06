#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "fruit-ninja-xbox360_shaders.h"
#include "xsdk/gpu.h"
#include "xsdk/kernel.h"
#include "xsdk/log.h"
#include "xsdk/present.h"
#include "xsdk/shader.h"
#include "xsdk/splash.h"

enum {
    BUFFER_COUNT = 2,
    SWAP_INTERVAL = 1,
    PRESENT_TIMEOUT_MS = 5000,
    CLEAR_ARGB = 0xFF18243A,
    VERTEX_COUNT = 3,
    FLOATS_PER_VERTEX = 5,
    VERTEX_BYTES = VERTEX_COUNT * FLOATS_PER_VERTEX * sizeof(float),
    POSITION_OFFSET = 0,
    COLOR_OFFSET = 2 * sizeof(float),
    VERTEX_STRIDE = FLOATS_PER_VERTEX * sizeof(float)
};

static const XsdkVertexElement vertex_layout[] = {
    { .usage = XSDK_USAGE_POSITION, .format = XSDK_VERTEX_FLOAT2, .offset_bytes = POSITION_OFFSET },
    { .usage = XSDK_USAGE_COLOR, .format = XSDK_VERTEX_FLOAT3, .offset_bytes = COLOR_OFFSET }
};

static void write_vertices(volatile uint32_t *gpu_vertices)
{
    static const float data[VERTEX_COUNT][FLOATS_PER_VERTEX] = {
        {  0.0f,  0.72f, 1.0f, 0.15f, 0.12f },
        { -0.72f, -0.62f, 0.12f, 1.0f, 0.25f },
        {  0.72f, -0.62f, 0.12f, 0.35f, 1.0f }
    };
    uint32_t words[VERTEX_BYTES / sizeof(uint32_t)];
    memcpy(words, data, sizeof(data));
    for (uint32_t i = 0; i < VERTEX_BYTES / sizeof(uint32_t); ++i)
        gpu_vertices[i] = words[i];
    xsdk_gpu_invalidate(xsdk_gpu_address(gpu_vertices), VERTEX_BYTES);
}

static void draw_frame(
    const XsdkShader *vertex_shader,
    const XsdkShader *pixel_shader,
    uint32_t vertex_address,
    uint32_t width,
    uint32_t height,
    uint32_t frame_address)
{
    xsdk_gpu_set_target(width, height);
    xsdk_gpu_set_shaders(vertex_shader, pixel_shader);
    xsdk_built_shader_set_literals(&triangle_vert);
    xsdk_built_shader_set_literals(&triangle_frag);
    xsdk_gpu_set_vertices(0, vertex_address, VERTEX_BYTES);
    xsdk_gpu_draw(XSDK_PRIMITIVE_TRIANGLE_LIST, VERTEX_COUNT);
    xsdk_gpu_resolve(frame_address, width, height, CLEAR_ARGB);
}

int main(void)
{
    NTSTATUS status;
    XsdkShader vertex_shader = { 0 };
    XsdkShader pixel_shader = { 0 };
    volatile uint32_t *gpu_vertices = NULL;
    bool gpu_open = false;
    bool display_open = false;
    int result = 1;

    xsdk_log_write("FruitNinja360: Xenos renderer bring-up");

    status = xsdk_gpu_open();
    if (!NT_SUCCESS(status)) {
        xsdk_log_write("FruitNinja360: xsdk_gpu_open failed: 0x%08X", (unsigned)status);
        goto cleanup;
    }
    gpu_open = true;

    status = xsdk_splash_show(1000);
    if (!NT_SUCCESS(status))
        xsdk_log_write("FruitNinja360: splash unavailable: 0x%08X", (unsigned)status);

    status = xsdk_shader_load(&vertex_shader, triangle_vert.container, triangle_vert.container_bytes);
    if (NT_SUCCESS(status))
        status = xsdk_shader_load(&pixel_shader, triangle_frag.container, triangle_frag.container_bytes);
    if (NT_SUCCESS(status))
        status = xsdk_shader_set_vertex_layout(
            &vertex_shader,
            vertex_layout,
            sizeof(vertex_layout) / sizeof(vertex_layout[0]),
            (const uint32_t[]){ VERTEX_STRIDE });
    if (!NT_SUCCESS(status)) {
        xsdk_log_write("FruitNinja360: shader setup failed: 0x%08X", (unsigned)status);
        goto cleanup;
    }

    gpu_vertices = xsdk_gpu_alloc(VERTEX_BYTES);
    if (!gpu_vertices) {
        xsdk_log_write("FruitNinja360: vertex allocation failed");
        goto cleanup;
    }
    write_vertices(gpu_vertices);

    uint32_t width = 0;
    uint32_t height = 0;
    status = xsdk_present_open(XSDK_DISPLAY_720P, BUFFER_COUNT, SWAP_INTERVAL, &width, &height);
    if (!NT_SUCCESS(status)) {
        xsdk_log_write("FruitNinja360: xsdk_present_open failed: 0x%08X", (unsigned)status);
        goto cleanup;
    }
    display_open = true;
    xsdk_log_write("FruitNinja360: display %ux%u; press Back to exit",
        (unsigned)width, (unsigned)height);

    result = 0;
    for (;;) {
        INPUT_STATE input = { 0 };
        if (XamInputGetState(0, 0, &input) == ERROR_SUCCESS &&
            (input.gamepad.buttons & INPUT_GAMEPAD_BACK))
            break;

        uint32_t frame_address = 0;
        status = xsdk_present_begin(&frame_address, PRESENT_TIMEOUT_MS);
        if (!NT_SUCCESS(status)) {
            xsdk_log_write("FruitNinja360: present_begin failed: 0x%08X", (unsigned)status);
            result = 1;
            break;
        }

        draw_frame(
            &vertex_shader,
            &pixel_shader,
            xsdk_gpu_address(gpu_vertices),
            width,
            height,
            frame_address);
        xsdk_present_end();
    }

cleanup:
    if (display_open)
        xsdk_present_close();
    if (gpu_vertices)
        xsdk_gpu_free(gpu_vertices);
    xsdk_shader_free(&pixel_shader);
    xsdk_shader_free(&vertex_shader);
    if (gpu_open) {
        status = xsdk_gpu_close();
        if (!NT_SUCCESS(status)) {
            xsdk_log_write("FruitNinja360: xsdk_gpu_close failed: 0x%08X", (unsigned)status);
            result = 1;
        }
    }

    xsdk_log_write("FruitNinja360: renderer bring-up %s", result == 0 ? "finished" : "failed");
    return result;
}
