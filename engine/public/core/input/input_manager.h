#pragma once

#include <array>

namespace golias {

    class InputManager {

    public:
        void SetKeyPressed(int key, bool pressed);

        bool IsKeyPressed(int key) const;

    private:
        InputManager()                               = default;
        InputManager(const InputManager&)            = delete;
        InputManager& operator=(const InputManager&) = delete;
        InputManager(InputManager&&)                 = delete;
        InputManager& operator=(InputManager&&)      = delete;

    private:
        std::array<bool, 1024> mKeyStates = {false};

        friend class Engine;
    };
} // namespace golias
