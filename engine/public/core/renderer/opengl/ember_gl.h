#pragma once

#include "core/ember_core.h"

// TODO: get this based on device?
constexpr int MAX_QUADS         = 10000;
constexpr int MAX_VERTICES      = MAX_QUADS * 4;
constexpr int MAX_INDICES       = MAX_QUADS * 6;

/*!
 *  @brief sampler2D (array) supported by platform
 *  DESKTOP - 32
 *  WEBGL - 16
 *  ANDROID - 32
 *  IOS - 32
 */

constexpr int MAX_TEXTURE_SLOTS = 16;

/*!
   @brief Opengl Renderer implementation
   - Opengl 3.3
   - Opengl ES 3.0
*/
class OpenglRenderer final : public Renderer {
public:
    OpenglRenderer() = default;


    void setup_shaders(Shader* default_shader, Shader* text_shader) override;

    void initialize() override;

    void flush() override;

    void flush_text() override;

    void resize(int view_width, int view_height) override;

    void set_context(const void* ctx) override;

    void* get_context() override;

    void destroy() override;

    Texture load_texture(const std::string& file_path) override;

    Font load_font(const std::string& file_path, int font_size) override;

    void unload_font(const Font& font) override;

    void unload_texture(const Texture& texture) override;

    void clear_background(const Color& color) override;

    void begin_drawing(const glm::mat4& view_projection) override;

    void end_drawing() override;

    void draw_text(const Font& font, const std::string& text, const Transform& transform, Color color, float font_size,
                  const ShaderEffect& shader_effect, float kerning) override;

    void draw_texture(const Texture& texture, const Transform& transform, glm::vec2 size,
                     const Color& color) override;

    void draw_texture_ex(const Texture& texture, const ember::Rectangle& source, const ember::Rectangle& dest,
                       glm::vec2 origin, float rotation,float zIndex, const Color& color) override;

    void draw_line(glm::vec3 start, glm::vec3 end, const Color& color, float thickness) override;

    void draw_rect(const Transform& transform, glm::vec2 size, const Color& color, float thickness) override;

    void draw_rect_filled(const Transform& transform, glm::vec2 size, const Color& color, float thickness) override;

    void draw_triangle(glm::vec3 p0, glm::vec3 p1, glm::vec3 p2, const Color& color) override;

    void draw_triangle_filled(glm::vec3 p0, glm::vec3 p1, glm::vec3 p2, const Color& color) override;

    void draw_circle(glm::vec3 position, float radius, const Color& color, int segments) override;

    void draw_circle_filled(glm::vec3 position, float radius, const Color& color, int segments) override;

    void BeginCanvas() override;

    void EndCanvas() override;


    glm::mat4 Projection = glm::mat4(1.f);

private:

    OpenglShader* _default_shader = nullptr;
    OpenglShader* _text_shader    = nullptr;

    float _bind_texture(Uint32 slot = 0) override ;

    void _submit(const Transform& transform, glm::vec2 size, glm::vec4 color, Uint32 slot = UINT32_MAX) override;


    GLuint _textVAO = 0, _textVBO = 0, _textEBO =0;
    Vertex* _textBuffer = nullptr;
    int _textQuadCount = 0;
    int _textIndexCount = 0;
    unsigned int _currentFontTextureID = 0;


    GLuint VAO = 0, VBO = 0, EBO = 0;
    Vertex* _buffer                                       = nullptr;

    // OPENGL/ES HACK FIX
    Uint32 _textureArrayBuffer = 0;

    std::array<unsigned int, MAX_TEXTURE_SLOTS> _textures = std::array<unsigned int, MAX_TEXTURE_SLOTS>();
    int _textureCount                                     = 0;
    int _quadCount                                        = 0;
    int _indexCount                                       = 0;


    SDL_GLContext context = nullptr;
};
