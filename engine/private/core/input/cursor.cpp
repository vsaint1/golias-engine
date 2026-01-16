#include "core/input/cursor.h"

#include "core/engine.h"

namespace golias {
    bool Cursor::isCursorEnabled             = true;
    ECursorLockState Cursor::cursorLockState = ECursorLockState::CURSOR_UNLOCKED;

    void Cursor::SetCursorEnabled(bool enabled) {
        isCursorEnabled = enabled;

        if (isCursorEnabled) {
            if (!SDL_ShowCursor()) {
                spdlog::error("Cursor::SetCursorEnabled Failed to show cursor: {}", SDL_GetError());
            }
        } else {
            if (!SDL_HideCursor()) {
                spdlog::error("Cursor::SetCursorEnabled Failed to hide cursor: {}", SDL_GetError());
            }
        }
    }

    bool Cursor::IsCursorEnabled() {
        return isCursorEnabled;
    }

    void Cursor::SetCursorLockState(ECursorLockState state) {
        cursorLockState = state;

        if (auto application = Engine::GetInstance().GetApplication()) {

            auto nativeWindow = application->GetWindowNativeHandle();
            
            if (state == ECursorLockState::CURSOR_LOCKED) {
                if (!SDL_SetWindowRelativeMouseMode(nativeWindow, true)) {
                    spdlog::error("Cursor::SetCursorLockState Failed to lock cursor: {}", SDL_GetError());
                }
            } else {
                if (!SDL_SetWindowRelativeMouseMode(nativeWindow, false)) {
                    spdlog::error("Cursor::SetCursorLockState Failed to unlock cursor: {}", SDL_GetError());
                }
            }
        }
    }

    ECursorLockState Cursor::GetCursorLockState() {
        return cursorLockState;
    }
} // namespace golias
