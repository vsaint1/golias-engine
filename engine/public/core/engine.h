#pragma once

#include "core/engine_config.h"
#include "core/io/file_system.h"
#include "systems/audio_manager.h"
#include "systems/input_manager.h"
#include "systems/time_manager.h"

#pragma region ENGINE_SYSTEMS
#include "core/systems/physics_sys.h"
#include "core/systems/scene_manager.h"
#include "core/systems/thread_pool.h"
#pragma endregion
#include <random>
#include "core/network/enet_client.h"

class Renderer;
class OpenglShader;
class OpenglRenderer;

constexpr double FIXED_TIMESTEP = 1.0 / 60.0;
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

    bool is_multiplayer = false;

    struct {
        float width                 = 0;
        float height                = 0;
        const SDL_DisplayMode* data = nullptr;
        int bbWidth = 0, bbHeight = 0; /// backbuffer
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
     * @brief Main engine loop.
     *
     * @details  Processes input, updates systems, and renders each frame.
     * - Windows, Linux, macOS, iOS, Android and Web
     *
     * @version 0.0.1
     */
    void run();

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


    /*!
     * @brief Update all engine systems.
     *
     * @param delta_time Time elapsed since last frame (in seconds).
     *
     * @version 0.0.1
     */
    void update_systems(double delta_time = 0);

    /*! @brief Get the created physics world.
     *
     * @return b2WorldId Physics world ID.
     *
     */
    [[nodiscard]] b2WorldId get_physics_world() const;

    /*! Get a specific engine system by type.
     *
     * @param T System type to retrieve.
     * @return T* Pointer to the requested system, or nullptr if not found.
     *
     * @example  PhysicsSystem* physics = GEngine->get_system<PhysicsSystem>();
     */
    template <typename T>
    T* get_system();


    // -------------------- SERVER CODE --------------------
    bool initialize_server(const char* host,int port = ENET_PORT_ANY);

    void shutdown_server();

    void update_server(double delta_time = 0);

    void broadcast(uint8_t type, const std::string& message);

    template <typename T>
    void broadcast(uint8_t type, const T& data);

    void on_message(uint8_t type, std::function<void(ENetPeer*, const Packet&)> handler);
private:

    std::vector<std::unique_ptr<EngineManager>> _systems{};

    Renderer* _renderer          = nullptr;
    InputManager* _input_manager = nullptr;
    TimeManager* _time_manager   = nullptr;

    b2WorldId _world;

    struct {
        ENetHost* host      = nullptr;
        ENetAddress address = {};
    } Server;

    HashMap<uint8_t, std::function<void(ENetPeer*, const Packet&)>> handlers;

    void handle_packet(ENetPeer* peer, ENetPacket* packet);

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

/*!
 * @brief Convert pixel coordinates to Physics world coordinates.
 *
 * @param pixel_pos Position in pixels.
 * @return b2Vec2 Position in Physics world units.
 */
b2Vec2 pixels_to_world(const glm::vec2& pixel_pos);

/*!
 * @brief Convert pixel coordinates to Physics world coordinates.
 *
 * @return b2Vec2 Position in Physics world units.
 */
glm::vec2 world_to_pixels(const b2Vec2& world_pos);

/*!
 * @brief Generate a random number between min and max.
 */
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



template <typename T>
void Engine::broadcast(uint8_t type, const T& data) {
    std::vector<uint8_t> payload(sizeof(T));
    SDL_memcpy(payload.data(), &data, sizeof(T));

    uint16_t size = static_cast<uint16_t>(payload.size());
    uint8_t header[3] = { type, static_cast<uint8_t>((size >> 8) & 0xFF), static_cast<uint8_t>(size & 0xFF) };

    std::vector<uint8_t> packet;
    packet.insert(packet.end(), header, header + 3);
    packet.insert(packet.end(), payload.begin(), payload.end());

    ENetPacket* p = enet_packet_create(packet.data(), packet.size(), ENET_PACKET_FLAG_RELIABLE);
    enet_host_broadcast(Server.host, 0, p);
    enet_host_flush(Server.host);
}


void engine_core_loop();