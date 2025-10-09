#define NK_IMPLEMENTATION

#define NK_INCLUDE_FIXED_TYPES
#define NK_INCLUDE_STANDARD_IO
#define NK_INCLUDE_STANDARD_VARARGS
#define NK_INCLUDE_DEFAULT_ALLOCATOR
#define NK_INCLUDE_VERTEX_BUFFER_OUTPUT
#define NK_INCLUDE_FONT_BAKING
#define NK_INCLUDE_DEFAULT_FONT
#define NK_INCLUDE_STANDARD_BOOL
#define NK_INCLUDE_COMMAND_USERDATA

#include <nuklear.h>


#if defined(EMBER_2D)
    #define NK_SDL3_RENDERER_IMPLEMENTATION
    #include "nuklear_sdl3_renderer.h"
#elif defined(EMBER_3D)
    #define NK_SDL3_GL3_IMPLEMENTATION
    #include "nuklear_sdl3_gl3.h"
#else
#error "Either EMBER_2D or EMBER_3D must be defined"
#endif