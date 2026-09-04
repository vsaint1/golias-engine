#pragma once

#include "core/input/input.h"
#include "stdafx.h"

namespace golias {


enum class InputMode { 
        Cursor,  // Standard cursor visible
        Hidden,  // Cursor hidden but still functional
        Disabled // Cursor disabled (mouse-look FPS like)
    };

    class Window {
    public:
        Window(int width, int height, CString title);

        virtual ~Window();

        virtual void PollEvents() = 0;

        virtual bool ShouldClose() const = 0;

        virtual void Close() = 0;

        virtual const String& GetTitle() const = 0;
        virtual void SetTitle(CString title)   = 0;

        virtual InputMode GetInputMode() const = 0;
        virtual void SetInputMode(InputMode mode) = 0;

        virtual void GetDrawableSize(int* width, int* height) const = 0;

        virtual void* GetNativeHandle() const = 0;

        virtual void* GetNativeViewHandle() const = 0;

        virtual void* GetHandle() const = 0;

        virtual void WaitForEvents() = 0;

        int GetWidth() const;

        int GetHeight() const;

        virtual void SwapBuffers() = 0;

        std::function<void(int, int)> OnResize;
        std::function<void(KeyCode, KeyAction, int)> OnKey;
        std::function<void(MouseButton, bool, int)> OnMouseButton;
        std::function<void(double, double)> OnCursorPos;
        std::function<void(double, double)> OnScroll;

    protected:
        void* mWindow = nullptr; // Native window handle (HWND, NSWindow*, etc.)

        int mWidth  = 0;
        int mHeight = 0;

        String mTitle;

        InputMode mInputMode = InputMode::Cursor;
    };

} // namespace golias
