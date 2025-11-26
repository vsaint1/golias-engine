#pragma once
#include "drivers/gles3/rendering_device_gles3.h"
#include <SDL3_ttf/SDL_ttf.h>
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

    void draw_line(float x1, float y1, float x2, float y2, const Color& color = Color::WHITE, float thickness = 1.0f);

    void draw_triangle(float x1, float y1, float x2, float y2, float x3, float y3, const Color& color = Color::WHITE);

    void draw_polygon(const std::vector<glm::vec2>& points, const Color& color = Color::WHITE);

    void draw_texture(RID texture, float x, float y, float width = 0, float height = 0, const Color& tint = Color::WHITE,
                      float rotation = 0.0f);

    void draw_texture_rect(RID texture, const Rect& dest, const Rect& source, const Color& tint = Color::WHITE, float rotation = 0.0f);

    void get_texture_size(RID texture, uint32_t& out_width, uint32_t& out_height);

    TTF_Font* load_font_from_file(const char* filepath, int size);

    RID load_texture_from_file(const char* filepath);
    RID load_texture_from_file(const char* filepath, const TextureDescription& desc);

    RID load_texture_from_memory(void* data, int width, int height, int channels = 4);
    void unload_texture(RID texture);

    template <typename... Args>
    void draw_text(TTF_Font* font, float x, float y, const Color& color, const char* format_str, Args&&... args);
    void draw_text(TTF_Font* font, float x, float y, const Color& color, const std::string& text);

    RID create_shader(const char* vertex_src, const char* fragment_src);
    RID create_shader_from_file(const char* filepath);
    RID create_shader_from_source(const char* source);
    void destroy_shader(RID shader);

    void draw_custom(RID shader, float x, float y, float width, float height, RID texture = INVALID_RID, const Color& color = Color::WHITE);

    void draw_texture_ex(float x, float y, float width, float height, RID texture = INVALID_RID, const Rect& source = {0, 0, 0, 0},
                         const Color& color = Color::WHITE, float rotation = 0.0f, bool flip_h = false, bool flip_v = false,
                         RID shader = INVALID_RID);


    void set_blend_mode(BlendMode mode);
    void set_line_width(float width);

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

    RenderingDevice* rd;

    RID shader;
    RID pipeline;
    RID vertex_buffer;
    RID index_buffer;
    RID white_texture;
    RID default_sampler;

    std::unordered_map<RID, RID> texture_samplers;

    std::vector<Vertex> vertices;
    std::vector<uint16_t> indices;
    std::vector<DrawCommand> draw_commands;

    glm::mat4 projection;
    glm::mat4 view;
    std::vector<glm::mat4> transform_stack;
    BlendMode current_blend_mode;
    ScaleMode scale_mode;
    float line_width;
    int window_width, window_height; /// Render resolution (internal)
    int viewport_width, viewport_height;
    bool is_drawing;

    std::unordered_map<RID, RID> custom_shader_pipelines; /// Shader RID -> pipeline RID


    TTF_Font* emoji_font = nullptr;
    std::unordered_map<std::string, TTF_Font*> loaded_fonts;

    void flush();
    void add_quad(const glm::mat4& transform, const Color& color, RID texture, bool use_texture);
    glm::mat4 get_current_transform() const;
    void calculate_viewport(int window_w, int window_h, int& out_x, int& out_y, int& out_w, int& out_h);
    void setup_pipeline_for_blend_mode(BlendMode mode);
    RID get_or_create_custom_pipeline(RID shader);
};


template <typename... Args>
inline void RenderingCanvas::draw_text(TTF_Font* font, float x, float y, const Color& color, const char* format_str, Args&&... args) {
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

inline void RenderingCanvas::draw_text(TTF_Font* font, float x, float y, const Color& color, const std::string& text) {
    if (!font) {
        spdlog::warn("DrawText called with null font!");
        return;
    }

    if (text.empty()) {
        return;
    }

    SDL_Color text_color;
    text_color.r = (uint8_t) (color.r * 255);
    text_color.g = (uint8_t) (color.g * 255);
    text_color.b = (uint8_t) (color.b * 255);
    text_color.a = (uint8_t) (color.a * 255);

    SDL_Surface* text_surface = TTF_RenderText_Blended(font, text.data(), text.length(), text_color);
    if (!text_surface) {
        spdlog::error("Failed to render Text '{}': {}", text, SDL_GetError());
        return;
    }

    SDL_Surface* rgba_surface = SDL_ConvertSurface(text_surface, SDL_PIXELFORMAT_RGBA32);
    SDL_DestroySurface(text_surface);

    if (!rgba_surface) {
        spdlog::error("Failed to convert Text surface to RGBA: {}", SDL_GetError());
        return;
    }


    RID text_texture = load_texture_from_memory(rgba_surface->pixels, rgba_surface->w, rgba_surface->h, 4);

    if (text_texture == INVALID_RID) {
        SDL_DestroySurface(rgba_surface);
        spdlog::error("Failed to create Texture from Text surface");
        return;
    }

    flush();

    std::vector<Vertex> text_vertices;
    std::vector<uint16_t> text_indices;

    glm::mat4 transform = glm::translate(glm::mat4(1.0f), glm::vec3(x, y, 0));
    transform           = get_current_transform() * transform;

    glm::vec4 positions[] = {glm::vec4(0, 0, 0, 1), glm::vec4(static_cast<float>(rgba_surface->w), 0, 0, 1),
                             glm::vec4(static_cast<float>(rgba_surface->w), static_cast<float>(rgba_surface->h), 0, 1),
                             glm::vec4(0, static_cast<float>(rgba_surface->h), 0, 1)};

    glm::vec2 texcoords[] = {{0, 0}, {1, 0}, {1, 1}, {0, 1}};
    glm::vec4 tint_color  = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);

    for (int i = 0; i < 4; i++) {
        glm::vec4 pos = transform * positions[i];
        text_vertices.push_back({glm::vec3(pos), tint_color, texcoords[i]});
    }

    text_indices = {0, 1, 2, 2, 3, 0};

    rd->buffer_update(vertex_buffer, 0, text_vertices.size() * sizeof(Vertex), text_vertices.data());
    rd->buffer_update(index_buffer, 0, text_indices.size() * sizeof(uint16_t), text_indices.data());

    rd->bind_pipeline(pipeline);
    rd->bind_vertex_buffers({vertex_buffer});
    rd->bind_index_buffer(index_buffer, IndexType::UINT16);

    glm::mat4 vp = projection * view;
    rd->push_constant("uViewProjection", glm::value_ptr(vp), sizeof(glm::mat4));

    rd->bind_texture(0, text_texture, default_sampler);
    int tex_unit = 0;
    rd->push_constant("uTexture", &tex_unit, sizeof(int));


    rd->draw_indexed(6, 1, 0, 0, 0);


    unload_texture(text_texture);
    SDL_DestroySurface(rgba_surface);
}
