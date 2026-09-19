"""Type stubs for the native `golias` module.
"""

from typing import Any
import enum

class Vector3:
    x: float
    y: float
    z: float
    def __init__(self, x: float = 0.0, y: float = 0.0, z: float = 0.0) -> None: ...

class Vector2:
    x: float
    y: float
    def __init__(self, x: float = 0.0, y: float = 0.0) -> None: ...

def deg_to_rad(degrees: float) -> float: ...

class Transform:
    def rotate_local(self, axis: Vector3, radians: float) -> None: ...
    def get_position(self) -> Vector3: ...
    def set_position(self, position: Vector3) -> None: ...
    def get_world_position(self) -> Vector3: ...
    def set_world_position(self, position: Vector3) -> None: ...
    def get_forward(self) -> Vector3: ...

class GameObject:
    def get_name(self) -> str: ...
    def get_transform(self) -> Transform: ...
    def is_active(self) -> bool: ...
    def set_active(self, active: bool) -> None: ...

class Time:
    @staticmethod
    def get_delta_time() -> float: ...
    @staticmethod
    def get_elapsed_time() -> float: ...

class Input:
    @staticmethod
    def is_key_pressed(key: KeyCode | str) -> bool: ...
    @staticmethod
    def get_mouse_position() -> Vector2: ...

class PythonBehavior:
    """Base class for golias scripts. `self` proxies the owning GameObject."""

    def start(self) -> None: ...
    def update(self, delta_time: float) -> None: ...
    def on_enable(self) -> None: ...
    def on_disable(self) -> None: ...
    def on_destroy(self) -> None: ...

    @property
    def transform(self) -> Transform: ...
    def get_name(self) -> str: ...
    def set_active(self, active: bool) -> None: ...
    def __getattr__(self, name: str) -> Any: ...


class KeyCode(enum.IntEnum):
    # Letters
    A = 1
    B = 2
    C = 3
    D = 4
    E = 5
    F = 6
    G = 7
    H = 8
    I = 9
    J = 10
    K = 11
    L = 12
    M = 13
    N = 14
    O = 15
    P = 16
    Q = 17
    R = 18
    S = 19
    T = 20
    U = 21
    V = 22
    W = 23
    X = 24
    Y = 25
    Z = 26

    # Digit row
    Num0 = 27
    Num1 = 28
    Num2 = 29
    Num3 = 30
    Num4 = 31
    Num5 = 32
    Num6 = 33
    Num7 = 34
    Num8 = 35
    Num9 = 36

    # Function keys
    F1 = 37
    F2 = 38
    F3 = 39
    F4 = 40
    F5 = 41
    F6 = 42
    F7 = 43
    F8 = 44
    F9 = 45
    F10 = 46
    F11 = 47
    F12 = 48
    F13 = 49
    F14 = 50
    F15 = 51
    F16 = 52
    F17 = 53
    F18 = 54
    F19 = 55
    F20 = 56
    F21 = 57
    F22 = 58
    F23 = 59
    F24 = 60
    F25 = 61

    # Modifiers
    LeftShift = 62
    RightShift = 63
    LeftControl = 64
    RightControl = 65
    LeftAlt = 66
    RightAlt = 67
    LeftSuper = 68
    RightSuper = 69

    # Navigation / misc
    Escape = 70
    Enter = 71
    Tab = 72
    Backspace = 73
    Insert = 74
    Delete = 75
    Home = 76
    End = 77
    PageUp = 78
    PageDown = 79
    Left = 80
    Right = 81
    Up = 82
    Down = 83
    CapsLock = 84
    NumLock = 85
    ScrollLock = 86
    PrintScreen = 87
    Pause = 88
    Menu = 89

    # Punctuation
    Space = 90
    Apostrophe = 91
    Comma = 92
    Minus = 93
    Period = 94
    Slash = 95
    Semicolon = 96
    Equal = 97
    LeftBracket = 98
    Backslash = 99
    RightBracket = 100
    GraveAccent = 101

    # Keypad
    KP0 = 102
    KP1 = 103
    KP2 = 104
    KP3 = 105
    KP4 = 106
    KP5 = 107
    KP6 = 108
    KP7 = 109
    KP8 = 110
    KP9 = 111
    KPDecimal = 112
    KPDivide = 113
    KPMultiply = 114
    KPSubtract = 115
    KPAdd = 116
    KPEnter = 117
    KPEqual = 118

    # Keypad navigation
    KPInsert = 119
    KPDelete = 120
    KPHome = 121
    KPEnd = 122
    KPPageUp = 123
    KPPageDown = 124
    KPLeft = 125
    KPRight = 126
    KPUp = 127
    KPDown = 128

    # Non-US / international
    World1 = 129
    World2 = 130
