#pragma once

#include "helpers/logging.h"



/**
 * @brief Supported rendering backends for the engine.
 */
enum class Backend {
    /**
     * @brief OpenGL-based compatibility profile.
     *
     * Desktop: OpenGL Core (3.3+)
     * Mobile: OpenGL ES (3.0+)
     * Web: WebGL 2/3
     */
    GL_COMPATIBILITY,

    /**
     * @brief Vulkan-based high-performance profile.
     *
     * Desktop and Android (if supported).
     */
    VK_FORWARD,

    /**
     * @brief DirectX 12 backend (Windows + Xbox).
     */
    DIRECTX12,

    /**
     * @brief Metal backend (macOS + iOS).
     */
    METAL,

    /**
     * @brief Automatically choose the best backend for the platform.
     */
    AUTO
};

enum class Orientation {
    LANDSCAPE_LEFT,
    LANDSCAPE_RIGHT,
    PORTRAIT,
    PORTRAIT_UPSIDE_DOWN
};

enum class AspectRatio {
    NONE,
    KEEP,
    EXPAND,
};

enum class ViewportMode {
    VIEWPORT,
    CANVAS
};

/**
 * @brief Texture filtering modes.
 *
 * Determines how texture pixels are sampled when scaled or transformed.
 */
enum class TextureFiltering {
    LINEAR,  ///< Smooth filtering.
    NEAREST  ///< Pixelated filtering.
};


struct Viewport {
    int width = 320;
    int height = 180;
    float scale = 1.0f;

    ViewportMode mode = ViewportMode::VIEWPORT;
    AspectRatio aspect_ratio  = AspectRatio::KEEP;

    bool load(const tinyxml2::XMLElement* root);

};

struct Environment {
    glm::vec4 clear_color = {0.0f, 0.0f, 0.0f, 1.0f};

    bool load(const tinyxml2::XMLElement* root);
};

struct Application {
    const char* name = "Window - Ember Engine";
    const char* version = "1.0";
    const char* package_name = "br.com.vsaint1.ember_engine"; // identifier
    const char* icon_path = "res/icon.png";
    const char* description = "EEngine";
    int max_fps = 60;

    bool is_fullscreen = false;
    bool is_resizable = true;

    bool load(const tinyxml2::XMLElement* root);
};

struct Threading {
    bool is_multithreaded = false;
    int thread_count = -1; // Default to -1 (auto-detect based on CPU cores)
};

struct RendererDevice {
    Backend backend = Backend::GL_COMPATIBILITY;
    TextureFiltering texture_filtering = TextureFiltering::NEAREST;

    bool load(const tinyxml2::XMLElement* root);

    const char* get_backend_str() const;

};

enum class WindowMode {
    WINDOWED,
    MAXIMIZED,
    MINIMIZED,
    FULLSCREEN
};

struct Window {
    glm::ivec2 size = glm::ivec2(1280,720);

    WindowMode window_mode = WindowMode::WINDOWED;

    float dpi_scale = 1.0f;

    bool load (const tinyxml2::XMLElement* root);
};

struct EngineConfig {

    Orientation orientation = Orientation::LANDSCAPE_LEFT;

    TextureFiltering texture_filtering = TextureFiltering::NEAREST;

    bool load();

    const char* get_orientation_str() const;

    RendererDevice get_renderer_device() const;

    Threading get_threading() const;

    Application get_application() const;

    Environment get_environment() const;

    Viewport get_viewport() const;

    Window get_window() const;

    bool is_vsync() const;

    void set_vsync(bool enabled);

private:

    Application _app;

    Environment _environment;

    Viewport _viewport;

    RendererDevice _renderer_device;

    Threading _threading;

    Window _window;

    bool _is_vsync_enabled = true;

    tinyxml2::XMLDocument _doc = {};
};
