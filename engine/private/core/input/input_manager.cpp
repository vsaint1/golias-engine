#include "core/input/input_manager.h"


namespace golias {

    bool InputManager::IsKeyPressed(int key) const {
        if (key < 0 || key >= static_cast<int>(mKeyStates.size())) {
            return false;
        }

        return mKeyStates[key];
    }

    void InputManager::SetKeyPressed(int key, bool pressed) {
        if (key < 0 || key >= static_cast<int>(mKeyStates.size())) {
            return;
        }

        mKeyStates[key] = pressed;
    }
    
} // namespace golias
