#pragma once

#include "core/engine_config.h"
#include "core/io/file_system.h"
#include "systems/input_manager.h"
#include "systems/time_manager.h"
#include "systems/audio_manager.h"

#pragma region ENGINE_SYSTEMS
#include "core/systems/physics_sys.h"
#include "core/systems/thread_pool.h"
#pragma endregion
#include <random>

class Renderer;
class OpenglShader;
class OpenglRenderer;


constexpr float PIXELS_PER_METER = 32.0f;
constexpr float METERS_PER_PIXEL = 1.0f / PIXELS_PER_METER;

/**
 * @brief Core Engine singleton.
 *
 * Manages window, renderer, audio, input, and timing systems.
 */
class Engine {
public:
    Engine();

    struct {
        int width                   = 0;
        int height                  = 0;
        const SDL_DisplayMode* data = nullptr;
        int bbWidth = 0, bbHeight = 0; // backbuffer
        SDL_Window* handle{};
    } Window;

    EngineConfig Config{};

    struct {
        SDL_AudioDeviceID device_id = 0;
        SDL_AudioSpec spec{};
        float global_volume = 1.f;
    } Audio;

    Ember_VFS VirtualFileSystem{};

    bool is_running = false;

    /**
     * @brief Resize the SDL window.
     *
     * @param w New width.
     * @param h New height.
     */
    void resize_window(int w, int h);

    /**
     * @brief Get the renderer instance.
     *
     * @return Renderer* Renderer pointer.
     */
    [[nodiscard]] Renderer* get_renderer() const;

    /**
     * @brief Get the input manager.
     *
     * @return InputManager* Input manager pointer.
     */
    [[nodiscard]] InputManager* input_manager() const;

    /**
     * @brief Get the time manager.
     *
     * @return TimeManager* Time manager pointer.
     */
    [[nodiscard]] TimeManager* time_manager() const;

    /**
     * @brief Deinitialize window, renderer, and modules.
     *
     * @version 0.0.1
     */
    void shutdown();


    /**
     * @brief Disable or enable vertical synchronization (VSync).
     *
     * @details by default, VSync is disabled.
     *
     * @version 1.0.0
     */
    void set_vsync(bool enabled);

    /**
     * @brief Initialize the engine: SDL window, renderer, audio, fonts.
     *
     * - Creates Window
     * - Chooses Graphics Backend (`OpenGL` or `Metal`)
     * - Initializes Audio and Font subsystems
     *
     *
     * @param width Virtual Window width.
     * @param height Virtual Window height.
     * @param type Renderer type (`OPENGL` or `METAL`).
     * @param flags SDL window flags. See: https://wiki.libsdl.org/SDL_WindowFlags
     *
     * @return true on success, false on failure.
     *
     * @version 0.0.1
     */
    bool initialize(int width, int height, Backend type, Uint64 flags = 0);

    void update(double delta_time) const;

    b2WorldId get_physics_world() const;

    template <typename T>
    T* get_system();

private:
    std::vector<std::unique_ptr<EngineManager>> _systems{};

    Renderer* _renderer          = nullptr;
    InputManager* _input_manager = nullptr;
    TimeManager* _time_manager   = nullptr;

    b2WorldId _world;

    /**
     * @brief Create a renderer instance.
     *
     * @param window SDL window.
     * @param view_width Viewport width.
     * @param view_height Viewport height.
     * @param type Renderer backend (`OPENGL` or `METAL`).
     * @return Renderer* Created renderer instance.
     *
     * @version 0.0.1
     */
    Renderer* _create_renderer_internal(SDL_Window* window, int view_width, int view_height, Backend type);

    /**
     * @brief Create an OpenGL renderer internally.
     *
     * @param window SDL window.
     * @param view_width Viewport width.
     * @param view_height Viewport height.
     * @return Renderer* Created OpenGL renderer.
     *
     * @version 0.0.2
     */
    Renderer* _create_renderer_gl(SDL_Window* window, int view_width, int view_height);

    /**
     * @brief Create a Metal renderer internally.
     *
     * @note TODO: Implement Metal backend.
     *
     * @param window SDL window.
     * @param view_width Viewport width.
     * @param view_height Viewport height.
     * @return Renderer* Created Metal renderer.
     */
    Renderer* _create_renderer_metal(SDL_Window* window, int view_width, int view_height);
};

template <typename T>
T* Engine::get_system() {

    for (auto& sys : _systems) {
        if (T* casted = dynamic_cast<T*>(sys.get())) {
            return casted;
        }
    }

    return nullptr;
}

// Global engine instance
extern std::unique_ptr<Engine> GEngine;

// Global audio engine instance (Miniaudio)
extern ma_engine audio_engine;


b2Vec2 pixels_to_world(const glm::vec2& pixelPos);

glm::vec2 world_to_pixels(const b2Vec2& worldPos);

template <typename T>
T random(T min, T max) {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    if constexpr (std::is_integral_v<T>) {
        std::uniform_int_distribution<T> dist(min, max);
        return dist(gen);
    } else if constexpr (std::is_floating_point_v<T>) {
        std::uniform_real_distribution<T> dist(min, max);
        return dist(gen);
    } else {
        static_assert("Unsupported type for random function");
    }

    return T{0};
}
