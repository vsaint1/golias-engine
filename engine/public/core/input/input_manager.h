#pragma once

#include <array>

namespace golias {


    enum class KeyCode : uint16_t {
        Unknown = 0,

        // Letters
        A,
        B,
        C,
        D,
        E,
        F,
        G,
        H,
        I,
        J,
        K,
        L,
        M,
        N,
        O,
        P,
        Q,
        R,
        S,
        T,
        U,
        V,
        W,
        X,
        Y,
        Z,

        // Number row
        Num0,
        Num1,
        Num2,
        Num3,
        Num4,
        Num5,
        Num6,
        Num7,
        Num8,
        Num9,

        // Function keys
        F1,
        F2,
        F3,
        F4,
        F5,
        F6,
        F7,
        F8,
        F9,
        F10,
        F11,
        F12,
        F13,
        F14,
        F15,
        F16,
        F17,
        F18,
        F19,
        F20,
        F21,
        F22,
        F23,
        F24,
        F25,

        // Modifiers
        LeftShift,
        RightShift,
        LeftControl,
        RightControl,
        LeftAlt,
        RightAlt,
        LeftSuper,
        RightSuper,

        // Navigation / control
        Escape,
        Enter,
        Tab,
        Backspace,
        Insert,
        Delete,
        Home,
        End,
        PageUp,
        PageDown,

        // Arrow keys
        Left,
        Right,
        Up,
        Down,

        // Lock / system keys
        CapsLock,
        NumLock,
        ScrollLock,
        PrintScreen,
        Pause,
        Menu,

        // Punctuation
        Space,
        Apostrophe,
        Comma,
        Minus,
        Period,
        Slash,
        Semicolon,
        Equal,
        LeftBracket,
        Backslash,
        RightBracket,
        GraveAccent,

        // Keypad
        KP0,
        KP1,
        KP2,
        KP3,
        KP4,
        KP5,
        KP6,
        KP7,
        KP8,
        KP9,

        KPDecimal,
        KPDivide,
        KPMultiply,
        KPSubtract,
        KPAdd,
        KPEnter,
        KPEqual,

        // Keypad navigation
        KPInsert,
        KPDelete,
        KPHome,
        KPEnd,
        KPPageUp,
        KPPageDown,
        KPLeft,
        KPRight,
        KPUp,
        KPDown,

        // Non-US / international
        World1,
        World2,

        Count
    };


    class InputManager {

    public:
        void SetKeyPressed(KeyCode key, bool pressed);

        bool IsKeyPressed(KeyCode key) const;

        int GetKeyCount() const {
            return static_cast<int>(KeyCode::Count);
        }

        KeyCode Translate(int key) const;

    private:
        InputManager()                               = default;
        InputManager(const InputManager&)            = delete;
        InputManager& operator=(const InputManager&) = delete;
        InputManager(InputManager&&)                 = delete;
        InputManager& operator=(InputManager&&)      = delete;

    private:
        std::array<bool, static_cast<size_t>(KeyCode::Count)> mKeyStates{};

        friend class Engine;
    };
} // namespace golias
