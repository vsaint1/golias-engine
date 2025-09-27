#include "core/renderer/opengl/ogl_renderer.h"

#include "core/engine.h"

bool OpenglRenderer::initialize(SDL_Window* window) {

#if defined(SDL_PLATFORM_IOS) || defined(SDL_PLATFORM_ANDROID) || defined(SDL_PLATFORM_EMSCRIPTEN)

    /* GLES 3.0 -> GLSL: 300 */
    const char* glsl_version = "#version 300 es";
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);

#elif defined(SDL_PLATFORM_WINDOWS) || defined(SDL_PLATFORM_LINUX) || defined(SDL_PLATFORM_MACOS)

    /* OPENGL 3.3 -> GLSL: 330*/
    const char* glsl_version = "#version 330";
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);


#endif

    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);

    SDL_GLContext glContext = SDL_GL_CreateContext(window);

    if (!glContext) {
        LOG_CRITICAL("Failed to create GL context, %s", SDL_GetError());
        return false;
    }

#if defined(SDL_PLATFORM_IOS) || defined(SDL_PLATFORM_ANDROID) || defined(SDL_PLATFORM_EMSCRIPTEN)

    if (!gladLoadGLES2Loader((GLADloadproc) SDL_GL_GetProcAddress)) {
        LOG_CRITICAL("Failed to initialize GLAD (GLES_FUNCTIONS)");
        return nullptr;
    }

#else

    if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(SDL_GL_GetProcAddress))) {
        LOG_CRITICAL("Failed to initialize GLAD (GL_FUNCTIONS)");
        return false;
    }

#endif

    _context = glContext;
    _window  = window;

    const auto& viewport = GEngine->get_config().get_viewport();
    LOG_INFO("Using backend: %s, Viewport: %dx%d", "Opengl/ES 3.X", viewport.width, viewport.height);

    glEnable(GL_DEPTH_TEST);
    glViewport(0, 0, viewport.width, viewport.height);

    return true;
}

void OpenglRenderer::clear(glm::vec4 color) {

    glClearColor(color.r, color.g, color.b, color.a);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void OpenglRenderer::present() {

    SDL_GL_SwapWindow(_window);
}

void OpenglRenderer::set_default_fonts(const std::string& text_font, const std::string& emoji_font) {
}

bool OpenglRenderer::load_font(const std::string& name, const std::string& path, int size) {
    return true;
}

std::shared_ptr<Texture> OpenglRenderer::load_texture(const std::string& name, const std::string& path) {
    return nullptr;
}

void OpenglRenderer::draw_texture(const Transform2D& transform, Texture* texture, const glm::vec4& dest, const glm::vec4& source,
                                  bool flip_h, bool flip_v, const glm::vec4& color) {
}

void OpenglRenderer::draw_text(const Transform2D& transform, const glm::vec4& color, const std::string& font_name, const char* fmt, ...) {
}

void OpenglRenderer::draw_rect(const Transform2D& transform, float w, float h, glm::vec4 color, bool is_filled) {
}

void OpenglRenderer::draw_triangle(const Transform2D& transform, float size, glm::vec4 color, bool is_filled) {
}

void OpenglRenderer::draw_line(const Transform2D& transform, glm::vec2 end, glm::vec4 color) {
}

void OpenglRenderer::draw_circle(const Transform2D& transform, float radius, glm::vec4 color, bool is_filled) {
}

void OpenglRenderer::draw_polygon(const Transform2D& transform, const std::vector<glm::vec2>& points, glm::vec4 color, bool is_filled) {
}


OpenglRenderer::~OpenglRenderer() {

    SDL_GL_DestroyContext(_context);
}

std::string OpenglRenderer::vformat(const char* fmt, va_list args) {

    return "Hello";
}


std::vector<Tokens> OpenglRenderer::parse_text(const std::string& text) {

    return {};
}

void OpenglRenderer::draw_text_internal(const glm::vec2& pos, const glm::vec4& color, const std::string& font_name,
                                        const std::string& text) {
}
