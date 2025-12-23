#pragma once
#include <array>
#include <unordered_map>
#include <functional>
#include <vector>
#include <string>
#include <glm/glm.hpp>
#include <SDL3/SDL_events.h>

namespace golias {

class InputManager {
public:
    ~InputManager() = default;
    
    bool IsKeyPressed(int keycode) const;
    bool IsKeyJustPressed(int keycode) const;
    bool IsKeyJustReleased(int keycode) const;
    
    void BindAction(const std::string& action_name, int keycode);
    void UnbindAction(const std::string& action_name);
    bool IsActionPressed(const std::string& action_name) const;
    bool IsActionJustPressed(const std::string& action_name) const;
    bool IsActionJustReleased(const std::string& action_name) const;
    
    void BindAxis(const std::string& axis_name, int positive_key, int negative_key);
    float GetAxis(const std::string& axis_name) const;
    glm::vec2 GetAxis2D(const std::string& horizontal_axis, const std::string& vertical_axis) const;
    
    void Update();
    
    void ProcessEvent(const SDL_Event& event);
    
    void Reset();

private:
    InputManager() = default;
    InputManager(const InputManager&) = delete;
    InputManager& operator=(const InputManager&) = delete;
    InputManager(InputManager&&) = delete;
    InputManager& operator=(InputManager&&) = delete;

    enum class KeyState : uint8_t {
        UP = 0,
        JUST_PRESSED = 1,
        PRESSED = 2,
        JUST_RELEASED = 3
    };
    
    struct AxisBinding {
        int positive_key;
        int negative_key;
    };

    static constexpr size_t MAX_KEYS = 512;
    std::array<KeyState, MAX_KEYS> key_states_{};
    
    std::unordered_map<std::string, std::vector<int>> action_bindings_;
    
    std::unordered_map<std::string, AxisBinding> axis_bindings_;
    
    bool IsValidKeycode(int keycode) const { 
        return keycode >= 0 && keycode < static_cast<int>(MAX_KEYS); 
    }
    
    KeyState GetKeyState(int keycode) const;
    
    friend class Engine;
};

} // namespace golias