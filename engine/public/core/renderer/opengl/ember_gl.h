#pragma once

#include "core/ember_core.h"

// TODO: get this based on device?
constexpr int MAX_QUADS    = 10000;
constexpr int MAX_VERTICES = MAX_QUADS * 4;
constexpr int MAX_INDICES  = MAX_QUADS * 6;

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
    OpenglRenderer();
    ~OpenglRenderer() override;

    void setup_shaders(Shader* default_shader, Shader* framebuffer_shader);

    void initialize() override;

    void resize_viewport(int view_width, int view_height) override;

    void set_context(const void* ctx) override;

    void* get_context() override;

    void destroy() override;

    std::shared_ptr<Texture> load_texture(const std::string& file_path) override;

    std::shared_ptr<Texture> get_texture(const std::string& path) override;

    bool load_font(const std::string& file_path, const std::string& font_alias, int font_size) override;

    void set_default_font(const std::string& font_name) override;

    void unload_font(const Font& font) override;

    void unload_texture(Uint32 id) override;

    void draw_texture(const Texture* texture, const Rect2& dest_rect, float rotation, const glm::vec4& color,
                                   const Rect2& src_rect, int z_index,
                                   bool flip_h, bool flip_v, const UberShader& uber_shader) override;

    void draw_rect(Rect2 rect, float rotation, const glm::vec4& color, bool filled, int z_index) override;

    void draw_text(const std::string& text, float x, float y, float rotation, float scale, const glm::vec4& color,
                   const std::string& font_alias, int z_index, const UberShader& uber_shader, int ft_size) override;

    void draw_line(float x1, float y1, float x2, float y2, float width, float rotation, const glm::vec4& color, int z_index) override;

    void draw_triangle(float x1, float y1, float x2, float y2, float x3, float y3, float rotation, const glm::vec4& color, bool filled,
                       int z_index) override;

    void draw_circle(float center_x, float center_y, float rotation, float radius, const glm::vec4& color, bool filled, int segments,
                     int z_index) override;

    void draw_polygon(const std::vector<glm::vec2>& points, float rotation, const glm::vec4& color, bool filled, int z_index) override;

    void render_command(const DrawCommand& cmd) override;

    void flush() override;

    void present() override;

    void clear(glm::vec4 color) override;

private:
    OpenglShader* _default_shader = nullptr;
    OpenglShader* _fbo_shader    = nullptr;

    // DEFAULT shader (text/texture/primitives)
    GLuint vao, vbo, ebo;
    GLuint shader_program;

    // DEFAULT FBO shader
    GLuint _fbo_vao,_fbo_vbo,_frame_buffer_object, _fbo_texture;

    SDL_GLContext context = nullptr;

    void render_fbo() override;

    void set_effect_uniforms(const UberShader& uber_shader, const glm::vec2& texture_size = glm::vec2(1, 1)) override;

    [[nodiscard]] glm::vec2 get_texture_size(Uint32 texture_id) const  override;

};
