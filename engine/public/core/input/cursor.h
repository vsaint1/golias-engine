#pragma once

namespace golias {
    enum ECursorLockState {
        CURSOR_LOCKED,
        CURSOR_UNLOCKED
    };

    class Cursor {
    public:
    
    static void SetCursorEnabled(bool enabled);
    static bool IsCursorEnabled();
    
    static void SetCursorLockState(ECursorLockState state);
    static ECursorLockState GetCursorLockState();
    private:
        friend class Engine;
        static bool isCursorEnabled;
        static ECursorLockState cursorLockState;

    };

} // namespace golias
