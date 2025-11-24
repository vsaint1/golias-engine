#include "servers/rendering/rendering_canvas.h"
#include "stdafx.h"
#include <SDL3/SDL_main.h>


namespace golias {


    class Camera2D {
    public:
        glm::vec2 position;
        float rotation;
        float zoom;

        Camera2D() : position(0.0f, 0.0f), rotation(0.0f), zoom(1.0f) {
        }

        Camera2D(float x, float y, float zoom = 1.0f, float rotation = 0.0f) : position(x, y), rotation(rotation), zoom(zoom) {
        }

        glm::mat4 get_view_matrix() const {
            glm::mat4 view = glm::mat4(1.0f);
            view           = glm::translate(view, glm::vec3(-position.x, -position.y, 0.0f));
            view           = glm::rotate(view, -rotation, glm::vec3(0.0f, 0.0f, 1.0f));
            view           = glm::scale(view, glm::vec3(zoom, zoom, 1.0f));
            return view;
        }

        glm::mat4 get_view_projection_matrix(float viewport_width, float viewport_height) const {
            glm::mat4 projection = glm::ortho(0.0f, viewport_width, viewport_height, 0.0f, -1.0f, 1.0f);
            return projection * get_view_matrix();
        }

        glm::vec2 screen_to_world(const glm::vec2& screen_pos, float viewport_width, float viewport_height) const {
            glm::mat4 inv_vp    = glm::inverse(get_view_projection_matrix(viewport_width, viewport_height));
            glm::vec4 world_pos = inv_vp * glm::vec4(screen_pos.x, screen_pos.y, 0.0f, 1.0f);
            return glm::vec2(world_pos.x, world_pos.y);
        }

        glm::vec2 world_to_screen(const glm::vec2& world_pos, float viewport_width, float viewport_height) const {
            glm::mat4 vp         = get_view_projection_matrix(viewport_width, viewport_height);
            glm::vec4 screen_pos = vp * glm::vec4(world_pos.x, world_pos.y, 0.0f, 1.0f);
            return glm::vec2(screen_pos.x, screen_pos.y);
        }

        void move(float dx, float dy) {
            position.x += dx;
            position.y += dy;
        }

        void look_at(float x, float y, float viewport_width, float viewport_height) {
            position.x = x - viewport_width / (2.0f * zoom);
            position.y = y - viewport_height / (2.0f * zoom);
        }
    };


} // namespace golias


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

#if defined(NDEBUG)
    logger->set_level(spdlog::level::info);
#else
    logger->set_level(spdlog::level::debug);
#endif

    logger->set_level(LOG_LEVEL);
    logger->flush_on(LOG_LEVEL);

    spdlog::set_default_logger(logger);


    SDL_Init(SDL_INIT_VIDEO);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);

    SDL_Window* window       = SDL_CreateWindow("Golias Engine", 1280, 720, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
    SDL_GLContext gl_context = SDL_GL_CreateContext(window);

    if (!gladLoadGLES2Loader(reinterpret_cast<GLADloadproc>(SDL_GL_GetProcAddress))) {
        spdlog::error("Failed to initialize OpenGL/ES Loader (GLAD)");
        return -1;
    }

    SDL_GL_SetSwapInterval(0);

    if (!TTF_Init()) {
        spdlog::error("Failed to initialize SDL_TTF: {}", SDL_GetError());
        return -1;
    }

    golias::RenderingDeviceGLES3 rd;
    rd.initialize();


    RenderingCanvas renderer(&rd);
    renderer.initialize(1280, 720);
    renderer.set_viewport_size(1280, 720);
    renderer.set_scale_mode(ScaleMode::KEEP);

    RID my_texture  = renderer.load_texture_from_file("res/icon.png");
    RID my_texture2 = renderer.load_texture_from_file("res/test/sprites/monsters.png");

    TextureDescription pixel_art_desc;
    pixel_art_desc.min_filter = TextureFilter::NEAREST;
    pixel_art_desc.mag_filter = TextureFilter::NEAREST;
    pixel_art_desc.wrap_u     = TextureWrap::CLAMP_TO_EDGE;
    pixel_art_desc.wrap_v     = TextureWrap::CLAMP_TO_EDGE;
    RID pixel_texture         = renderer.load_texture_from_file("res/test/sprites/monsters.png", pixel_art_desc);

    TextureDescription smooth_desc;
    smooth_desc.min_filter = TextureFilter::LINEAR;
    smooth_desc.mag_filter = TextureFilter::LINEAR;
    smooth_desc.wrap_u     = TextureWrap::REPEAT;
    smooth_desc.wrap_v     = TextureWrap::REPEAT;
    RID smooth_texture     = renderer.load_texture_from_file("res/icon.png", smooth_desc);

    RID wavy_shader    = renderer.create_shader_from_file("res/test/shaders/wave.glsl");
    RID rainbow_shader = renderer.create_shader_from_file("res/test/shaders/rainbow.glsl");
    RID retro_shader   = renderer.create_shader_from_file("res/test/shaders/retro.glsl");

    golias::Camera2D camera(0.0f, 0.0f, 1.0f);
    bool use_camera = false; // Toggle with C key

    bool running = true;
    SDL_Event event;
    float time = 0.0f;

    TTF_Font* mine = renderer.load_font_from_file("res/test/fonts/Minecraft.ttf", 24.0f);
    TTF_Font* font = renderer.load_font_from_file("res/test/fonts/Default.ttf", 24.0f);

    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT || (event.type == SDL_EVENT_KEY_DOWN && event.key.key == SDLK_ESCAPE)) {
                running = false;
            }

            if (event.type == SDL_EVENT_WINDOW_RESIZED) {
                int new_width  = event.window.data1;
                int new_height = event.window.data2;
                renderer.set_viewport_size(new_width, new_height);
            }

            if (event.type == SDL_EVENT_KEY_DOWN) {
                if (event.key.key == SDLK_1) {
                    renderer.set_scale_mode(ScaleMode::NONE);
                    printf("Scale Mode: NONE (no scaling, centered)\n");
                } else if (event.key.key == SDLK_2) {
                    renderer.set_scale_mode(ScaleMode::KEEP);
                    printf("Scale Mode: KEEP (aspect ratio preserved)\n");
                } else if (event.key.key == SDLK_3) {
                    renderer.set_scale_mode(ScaleMode::EXPAND);
                    printf("Scale Mode: EXPAND (fill window, may stretch)\n");
                } else if (event.key.key == SDLK_C) {
                    use_camera = !use_camera;
                    printf("Camera %s\n", use_camera ? "ENABLED" : "DISABLED");
                }
            }
        }


        const bool* keys = SDL_GetKeyboardState(nullptr);
        if (use_camera) {
            float camera_speed = 200.0f * 0.016f;
            if (keys[SDL_SCANCODE_LEFT]) {
                camera.move(-camera_speed, 0);
            }
            if (keys[SDL_SCANCODE_RIGHT]) {
                camera.move(camera_speed, 0);
            }
            if (keys[SDL_SCANCODE_UP]) {
                camera.move(0, -camera_speed);
            }
            if (keys[SDL_SCANCODE_DOWN]) {
                camera.move(0, camera_speed);
            }

            if (keys[SDL_SCANCODE_EQUALS] || keys[SDL_SCANCODE_KP_PLUS]) {
                camera.zoom += 0.5f * 0.016f;
                if (camera.zoom > 3.0f) {
                    camera.zoom = 3.0f;
                }
            }
            if (keys[SDL_SCANCODE_MINUS] || keys[SDL_SCANCODE_KP_MINUS]) {
                camera.zoom -= 0.5f * 0.016f;
                if (camera.zoom < 0.1f) {
                    camera.zoom = 0.1f;
                }
            }

            if (keys[SDL_SCANCODE_R]) {
                camera.rotation += 1.0f * 0.016f;
            }
            if (keys[SDL_SCANCODE_T]) {
                camera.rotation -= 1.0f * 0.016f;
            }
        }

        time += 0.016f;

        renderer.begin(Color(0.1f, 0.1f, 0.15f, 1.0f));


        if (use_camera) {
            renderer.set_camera(camera.get_view_projection_matrix(1280, 720));
        } else {
            renderer.reset_camera();
        }

        renderer.draw_rect(50, 50, 150, 100, Color::RED);

        renderer.draw_circle(400, 150, 60, Color::GREEN);

        renderer.draw_triangle(550, 50, 650, 50, 600, 150, Color::BLUE);

        renderer.draw_line(50, 250, 200, 250, Color::YELLOW, 3.0f);
        renderer.draw_line(50, 280, 200, 320, Color::CYAN, 2.0f);

        renderer.draw_rect_outlined(250, 250, 120, 80, Color::MAGENTA, 2.0f);
        renderer.draw_circle(450, 300, 50, Color::YELLOW);

        float rotation = time * 2.0f;
        renderer.draw_rect(550, 250, 100, 100, Color::CYAN, rotation);

        renderer.draw_texture(my_texture2, 50, 400, 150, 150);

        renderer.draw_texture(my_texture, 250, 400, 120, 120, Color::WHITE, time * 3.0f);

        renderer.draw_texture(my_texture, 50, 50, 100, 100);

        renderer.draw_texture(pixel_texture, 900, 50, 128, 128);
        renderer.draw_text(font, 900, 15, Color::WHITE, "NEAREST Filter");


        if (wavy_shader != INVALID_RID) {
            renderer.draw_custom(wavy_shader, 200, 50, 100, 100, my_texture, Color::WHITE);
        }

        if (rainbow_shader != INVALID_RID) {
            renderer.draw_custom(rainbow_shader, 200, 170, 100, 100, INVALID_RID, Color::WHITE);
        }

        if (retro_shader != INVALID_RID) {
            renderer.draw_custom(retro_shader, 200, 290, 100, 100, my_texture, Color::WHITE);
        }

        renderer.draw_texture_ex(700, 50, 80, 80, my_texture);

        renderer.draw_texture_ex(800, 50, 80, 80, my_texture, {0, 0, 0, 0}, Color::WHITE, 0.0f, true, false);

        renderer.draw_texture_ex(700, 150, 80, 80, my_texture, {0, 0, 0, 0}, Color::WHITE, 0.0f, false, true);

        renderer.draw_texture_ex(800, 150, 80, 80, my_texture, {0, 0, 0, 0}, Color::WHITE, 0.0f, true, true);

        renderer.draw_texture_ex(750, 270, 80, 80, my_texture, {0, 0, 0, 0}, Color::WHITE, time * 2.0f, false, false, wavy_shader);


        renderer.draw_texture_ex(700, 380, 64, 64, pixel_texture, {0, 0, 32, 32}, Color::WHITE);

        renderer.draw_text(mine, 400, 50, Color::WHITE, "Hello World! 👋 Welcome!");
        renderer.draw_text(font, 400, 80, Color::WHITE, "Frame: {} 🎮", static_cast<int>(time * 60));
        renderer.draw_text(font, 400, 110, Color::WHITE, "Position: ({}, {}) 📍", 123, 456);
        renderer.draw_text(font, 400, 140, Color::WHITE, "Time: {:.2f}s ⏱️", time);
        renderer.draw_text(font, 400, 170, Color::WHITE, "FPS: {} 🚀", 60);
        renderer.draw_text(font, 400, 200, Color::WHITE, "Mixed: ABC 123 😀 🎉 🔥 ⭐ XYZ");


        renderer.draw_text(font, 400, 240, Color::GREEN, "Health: {}", 100);
        renderer.draw_text(font, 400, 270, Color::RED, "Score: 999,999 🏆");

        renderer.end();

        SDL_GL_SwapWindow(window);

        SDL_Delay(16);
    }


    renderer.shutdown();
    rd.shutdown();

    TTF_Quit();

    SDL_GL_DestroyContext(gl_context);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
