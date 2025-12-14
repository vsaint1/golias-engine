#include "servers/rendering/rendering_device.h"


const Color Color::WHITE   = Color(1, 1, 1, 1);
const Color Color::BLACK   = Color(0, 0, 0, 1);
const Color Color::RED     = Color(1, 0, 0, 1);
const Color Color::GREEN   = Color(0, 1, 0, 1);
const Color Color::BLUE    = Color(0, 0, 1, 1);
const Color Color::YELLOW  = Color(1, 1, 0, 1);
const Color Color::CYAN    = Color(0, 1, 1, 1);
const Color Color::MAGENTA = Color(1, 0, 1, 1);


RID RIDAllocator::allocate_rid() {
    return next_rid++;
}

void RenderingDevice::get_drawable_size(int& width, int& height) {
    if (_window) {
        SDL_GetWindowSizeInPixels(_window, &width, &height);
    } else {
        width  = 0;
        height = 0;
    }
}
