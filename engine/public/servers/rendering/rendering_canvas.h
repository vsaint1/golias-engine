#pragma once

#include "drivers/gles3/rendering_device_gles3.h"
#include "servers/rendering/shader_material.h"
#include <spdlog/spdlog.h>


class RenderingCanvas {
public:
    explicit RenderingCanvas(RenderingDevice* device);
    ~RenderingCanvas();

    bool initialize(int window_width, int window_height);
    void shutdown();

    void begin(const Color& clear_color = Color::BLACK);
    void end();

    void set_camera(const glm::mat4& view_projection);
    void reset_camera();
    void set_viewport_size(int width, int height);
    void set_scale_mode(ScaleMode mode);
    void push_transform(const glm::mat4& transform);
    void pop_transform();

    void draw_rect(float x, float y, float width, float height, const Color& color = Color::WHITE, float rotation = 0.0f);

    void draw_rect_outlined(float x, float y, float width, float height, const Color& color = Color::WHITE, float thickness = 1.0f);

    void draw_circle(float x, float y, float radius, const Color& color = Color::WHITE, int segments = 32);

    void draw_circle_outlined(float x, float y, float radius, const Color& color = Color::WHITE, float thickness = 1.0f, int segments = 32);

    void draw_arc(float x, float y, float radius, float start_angle, float end_angle, const Color& color = Color::WHITE, int segments = 32);

    void draw_line(float x1, float y1, float x2, float y2, const Color& color = Color::WHITE, float thickness = 1.0f);

    void draw_triangle(float x1, float y1, float x2, float y2, float x3, float y3, const Color& color = Color::WHITE);

    void draw_polygon(const Vector<glm::vec2>& points, const Color& color = Color::WHITE);

    void draw_texture(RID texture, float x, float y, float width = 0, float height = 0, const Color& tint = Color::WHITE,
                      float rotation = 0.0f);

    /// @deprecated  Use draw_texture_ex with a CanvasMaterial (optional) instead
    void draw_custom(RID shader, float x, float y, float width, float height, RID texture = INVALID_RID, const Color& color = Color::WHITE);

    void draw_texture_ex(float x, float y, float width, float height, RID texture = INVALID_RID, const Rect& source = {0, 0, 0, 0},
                         const Color& color = Color::WHITE, float rotation = 0.0f, bool flip_h = false, bool flip_v = false,
                         const CanvasMaterial* material = nullptr);

    template <typename... Args>
    void draw_text(Font* font, float x, float y, const Color& color, const char* format_str, Args&&... args);

    void draw_text(Font* font, float x, float y, const Color& color, const String& text);
    
    void draw_text(float x, float y, const Color& color, const String& text);
    
    Font* load_font_from_file(const char* filepath, int size);

    RID load_texture(const char* filepath);
    RID load_texture(const char* filepath, const TextureDescription& desc);
    Texture load_texture_from_file(const char* filepath);
    Texture load_texture_from_file(const char* filepath, const TextureDescription& desc);
    void get_texture_size(RID texture, uint32_t& out_width, uint32_t& out_height);

    RID load_texture_from_memory(void* data, int width, int height, int channels = 4);
    void unload_texture(RID texture);

    RID create_shader(const char* vertex_src, const char* fragment_src);
    RID load_shader_from_file(const char* filepath);
    RID create_shader_from_source(const char* source);
    void destroy_shader(RID shader);

    void set_blend_mode(BlendMode mode);
    void set_line_width(float width);

    RenderingDevice* get_rendering_device() const { return rd; }

private:
    struct Vertex {
        glm::vec3 position;
        glm::vec4 color;
        glm::vec2 texcoord;
    };

    struct DrawCommand {
        RID texture;
        uint32_t index_start;
        uint32_t index_count;
        bool use_texture;
    };


    RID shader;
    RID pipeline;
    RID vertex_buffer;
    RID index_buffer;
    RID white_texture;
    RID default_sampler;

    std::unordered_map<RID, RID> texture_samplers;

    Vector<Vertex> vertices;
    Vector<uint16_t> indices;
    Vector<DrawCommand> draw_commands;

    glm::mat4 projection;
    glm::mat4 view;
    Vector<glm::mat4> transform_stack;
    BlendMode current_blend_mode;
    ScaleMode scale_mode;
    float line_width;

    int window_width, window_height; /// Render resolution (internal)
    int viewport_width, viewport_height;
    bool is_drawing;

    HashMap<RID, RID> custom_shader_pipelines; /// Shader RID -> pipeline RID

    Font* default_font = nullptr;
    Font* emoji_font = nullptr;
    HashMap<String, Font*> loaded_fonts;

    RenderingDevice* rd;

    void flush();
    void add_quad(const glm::mat4& transform, const Color& color, RID texture, bool use_texture);
    glm::mat4 get_current_transform() const;
    void calculate_viewport(int window_w, int window_h, int& out_x, int& out_y, int& out_w, int& out_h);
    void setup_pipeline_for_blend_mode(BlendMode mode);
    RID get_or_create_custom_pipeline(RID shader);
};


template <typename... Args>
void RenderingCanvas::draw_text(Font* font, float x, float y, const Color& color, const char* format_str, Args&&... args) {
    if (!font) {
        spdlog::warn("DrawText called with null font!");
        return;
    }

    std::string formatted_text;
    if constexpr (sizeof...(args) > 0) {
        formatted_text = std::vformat(format_str, std::make_format_args(args...));
    } else {
        formatted_text = format_str;
    }

    draw_text(font, x, y, color, formatted_text);
}
