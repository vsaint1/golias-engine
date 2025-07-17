#pragma once

#include "core/input/input_manager.h"
#include "core/io/file_system.h"
#include "core/time_manager.h"

/**
 * @brief Graphics backend type.
 *
 * Choose which backend to use.
 *
 * - Type: OpenGL
 *   - WEB: WebGL 3.0
 *   - DESKTOP: OpenGL 3.3
 *   - MOBILE: OpenGLES 3.0
 *
 * - Type: Metal
 *   - Apple devices only.
 *
 * @version 0.0.1
 */
enum RendererType {
    OPENGL,
    METAL
};

class Renderer;
class OpenglShader;
class OpenglRenderer;

/**
 * @brief Core Engine singleton.
 *
 * Manages window, renderer, audio, input, and timing systems.
 */
class Engine {
public:
    struct {
        int width = 0;
        int height = 0;
        const char* title = "[EMBER_ENGINE] - Window";
        const SDL_DisplayMode* data = nullptr;
        bool bFullscreen = false;
        int bbWidth = 0, bbHeight = 0; // backbuffer
        SDL_Window* handle{};
    } Window;

    struct {
        SDL_AudioDeviceID device_id = 0;
        SDL_AudioSpec spec{};
        float global_volume = 1.f;
    } Audio;

    Ember_VFS VirtualFileSystem{};

    bool bIsRunning = false;

    /**
     * @brief Resize the SDL window.
     *
     * @param w New width.
     * @param h New height.
     */
    void resize_window(int w, int h) const;

    /**
     * @brief Get the renderer instance.
     *
     * @return Renderer* Renderer pointer.
     */
    Renderer* get_renderer() const;

    /**
     * @brief Get the input manager.
     *
     * @return InputManager* Input manager pointer.
     */
    InputManager* input_manager() const;

    /**
     * @brief Get the time manager.
     *
     * @return TimeManager* Time manager pointer.
     */
    TimeManager* time_manager() const;

    /**
     * @brief Deinitialize window, renderer, and modules.
     *
     * @version 0.0.1
     */
    void shutdown();

    /**
     * @brief Initialize the engine: SDL window, renderer, audio, fonts.
     *
     * - Creates Window
     * - Chooses Graphics Backend (`OpenGL` or `Metal`)
     * - Initializes Audio and Font subsystems
     *
     * @param title Window title.
     * @param width Window width.
     * @param height Window height.
     * @param type Renderer type (`OPENGL` or `METAL`).
     * @param flags SDL window flags. See: https://wiki.libsdl.org/SDL_WindowFlags
     *
     * @return true on success, false on failure.
     *
     * @version 0.0.1
     */
    bool initialize(const char* title, int width, int height, RendererType type, Uint64 flags = 0);

private:
    Renderer* _renderer = nullptr;
    InputManager* _input_manager = nullptr;
    TimeManager* _time_manager = nullptr;

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
    Renderer* _create_renderer_internal(SDL_Window* window, int view_width, int view_height, RendererType type);

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

// Global engine instance
extern std::unique_ptr<Engine> GEngine;

// Global audio engine instance (Miniaudio)
extern ma_engine audio_engine;
