#include "servers/rendering/rendering_canvas.h"
#include "stdafx.h"
#include <SDL3/SDL_main.h>


enum class RenderingDeviceType { COMPATIBILITY, FORWARD_PLUS };


int main(int argc, char* argv[]) {

#if defined(NDEBUG)
    constexpr auto LOG_LEVEL = spdlog::level::info;
#else
    constexpr auto LOG_LEVEL = spdlog::level::debug;
#endif

#if defined(__ANDROID__)
    auto android_sink = std::make_shared<spdlog::sinks::android_sink_mt>("GoliasEngine");
    std::vector<spdlog::sink_ptr> sinks{android_sink};
#else
    auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    auto file_sink    = std::make_shared<spdlog::sinks::basic_file_sink_mt>("logs/output.log", true);
    std::vector<spdlog::sink_ptr> sinks{console_sink, file_sink};
#endif

    auto logger = std::make_shared<spdlog::logger>("GoliasEngine", sinks.begin(), sinks.end());
    logger->set_level(LOG_LEVEL);
    logger->flush_on(LOG_LEVEL);
    spdlog::set_default_logger(logger);

    if (!SDL_Init(SDL_INIT_VIDEO)) {
        SDL_Log("Failed to initialize SDL");
        return -1;
    }

    SDL_Window* window = SDL_CreateWindow("Golias Engine", 1280, 720, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);

    if (!window) {
        SDL_Log("Failed to create SDL window");
        return -1;
    }

    TTF_Init();
    RenderingDeviceType device_type = RenderingDeviceType::FORWARD_PLUS;

    std::unique_ptr<RenderingDevice> rd = nullptr;
    if (device_type == RenderingDeviceType::COMPATIBILITY) {
        rd = std::make_unique<RenderingDeviceGLES3>();
        spdlog::info("Using Compatibility Rendering Device");
    } else if (device_type == RenderingDeviceType::FORWARD_PLUS) {
        rd = std::make_unique<RenderingDeviceSDL_GPU>();
        spdlog::info("Using Forward+ Rendering Device");
    } else {
        spdlog::error("Unknown Rendering Device Type");
        return -1;
    }


    if (!rd->initialize(window)) {
        SDL_Log("Failed to initialize Rendering Device");
        return -1;
    }

    RenderingCanvas renderer(rd.get());
    int drawable_w, drawable_h;
    SDL_GetWindowSizeInPixels(window, &drawable_w, &drawable_h);
    renderer.initialize(drawable_w, drawable_h);
    renderer.set_viewport_size(drawable_w, drawable_h);
    renderer.set_scale_mode(ScaleMode::EXPAND);


    Texture icon_tex     = renderer.load_texture_from_file("res/icon.png");
    Texture monsters_tex = renderer.load_texture_from_file("res/test/sprites/monsters.png");

    TextureDescription pixel_desc;
    pixel_desc.min_filter = TextureFilter::NEAREST;
    pixel_desc.mag_filter = TextureFilter::NEAREST;
    pixel_desc.wrap_u     = TextureWrap::CLAMP_TO_EDGE;
    pixel_desc.wrap_v     = TextureWrap::CLAMP_TO_EDGE;
    Texture pixel_tex     = renderer.load_texture_from_file("res/test/sprites/monsters.png", pixel_desc);

    /// NOTE: Shader loading/compilation is not yet implemented for SDL_GPU
    // RID wavy_shader    = renderer.load_shader_from_file("res/test/shaders/wave.glsl");
    // RID rainbow_shader = renderer.load_shader_from_file("res/test/shaders/rainbow.glsl");
    // RID retro_shader   = renderer.load_shader_from_file("res/test/shaders/retro.glsl");
    //
    // auto wavy_material    = CanvasMaterial(wavy_shader).set_shader_param("amplitude", 10.0f).set_shader_param("frequency", 5.0f);
    // auto rainbow_material = CanvasMaterial(rainbow_shader).set_shader_param("speed", 2.0f);
    // auto retro_material   = CanvasMaterial(retro_shader).set_shader_param("pixel_size", 16.0f);

    Font mine_font = renderer.load_font_from_file("res/test/fonts/Minecraft.ttf", 24.0f);


    bool running = true;
    SDL_Event event;
    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT) {
                running = false;
            }
        }

        renderer.begin();

        renderer.draw_rect(100, 50, 30, 50, Color::WHITE);
        renderer.draw_circle(300, 600, 75, Color::YELLOW);
        renderer.draw_triangle(200, 400, 250, 300, 300, 400, Color::BLUE);
        renderer.draw_arc(600, 400, 100, 0.0f, 3.14f, Color::RED, 32);
        renderer.draw_circle_outlined(800, 200, 50, Color::MAGENTA, 3.0f, 32);
        renderer.draw_line(400, 400, 600, 450, Color::GREEN, 5.0f);
        renderer.draw_texture(monsters_tex.rid, 500, 50, 128, 128);
        renderer.draw_texture(pixel_tex.rid, 650, 50, 128, 128);
        renderer.draw_text(mine_font, 50, 200, Color::WHITE, "We Love Minecraft ❤️");

        renderer.draw_text(150, 50, Color::WHITE, "Hello, Golias Engine!");

        renderer.end();

        renderer.present();

        SDL_Delay(16);
    }

    renderer.shutdown();
    TTF_Quit();
    SDL_Quit();
    return 0;
}
