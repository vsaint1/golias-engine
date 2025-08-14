#pragma once
#include "core/ember_core.h"
// #include "core/renderer/shader.h"


class MetalRenderer final : public Renderer {
public:
    MetalRenderer();
    ~MetalRenderer() override;

    void initialize() override;
    void clear(glm::vec4 color) override;
    void present() override;
    void resize_viewport(int view_width, int view_height) override;
    void set_context(const void* ctx) override;
    void* get_context() override;
    void destroy() override;

    bool load_font(const std::string&, const std::string&, int) override;
    std::shared_ptr<Texture> load_texture(const std::string& path) override;
    std::shared_ptr<Texture> get_texture(const std::string& path) override;
    void unload_font(const Font& font) override;
    void unload_texture(Uint32 id) override;

    void draw_texture(const Texture* texture, const Rect2& dest_rect, float rotation,
                      const glm::vec4& color, const Rect2& src_rect, int z_index,
                      const UberShader& uber_shader) override;
    void draw_rect(Rect2 rect, float rotation, const glm::vec4& color, bool filled, int z_index) override;
    void draw_text(const std::string& text, float x, float y, float rotation, float scale,
                   const glm::vec4& color, const std::string& font_alias, int z_index,
                   const UberShader& uber_shader, int ft_size) override;
    void draw_line(float x1, float y1, float x2, float y2, float width,
                   float rotation, const glm::vec4& color, int z_index) override;
    void draw_triangle(float x1, float y1, float x2, float y2, float x3, float y3,
                       float rotation, const glm::vec4& color, bool filled, int z_index) override;
    void draw_circle(float cx, float cy, float rotation, float radius, const glm::vec4& color,
                     bool filled, int segments, int z_index) override;
    void draw_polygon(const std::vector<glm::vec2>& points, float rotation,
                      const glm::vec4& color, bool filled, int z_index) override;
    void flush() override;

protected:
    void _render_command(const DrawCommand& cmd) override;
    void _set_default_font(const std::string& font_name) override;
    void _render_fbo() override;
    void _set_effect_uniforms(const UberShader& uber_shader, const glm::vec2& texture_size) override;
    glm::vec2 _get_texture_size(Uint32 texture_id) const override;

private:
    void* device_ = nullptr;        // id<MTLDevice>
    void* queue_ = nullptr;         // id<MTLCommandQueue>
    void* pipeline_ = nullptr;      // id<MTLRenderPipelineState>
    void* metal_layer_ = nullptr;   // CAMetalLayer*
    void* sampler_ = nullptr;

    SDL_MetalView metal_view_ = 0;    // SDL_MetalView
    glm::vec4 clear_color_{0,0,0,1};
    

    
};
