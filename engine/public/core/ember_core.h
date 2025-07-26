#pragma once

#include "core/component/camera.h"
#include "core/component/circle.h"
#include "core/component/label.h"
#include "core/component/node.h"
#include "core/component/polygon.h"
#include "core/component/sprite_node.h"
#include "core/text_parser.h"

#pragma region OPENGL/ES
#include "core/renderer/mesh.h"
#include "core/renderer/opengl/shader_gl.h"
#pragma endregion

#pragma region METAL
#pragma endregion

#include "core/audio/ember_audio.h"
#include "core/ember_utils.h"
#include "core/system_info.h"


enum class DrawCommandType {
    NONE,
    TEXTURE,
    TEXT,
    LINE,
    RECT,
    TRIANGLE,
    CIRCLE,
    POLYGON,
};


struct DrawCommand {
    DrawCommandType type = DrawCommandType::NONE;
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;
    Uint32 texture_id   = 0;
    float line_width    = 1.0f;
    int circle_segments = 32;
    int z_index         = 0;

    explicit DrawCommand(DrawCommandType t, int z = 0) : type(t), z_index(z) {
    }
};

struct BatchKey {
    Uint32 texture_id;
    int z_index;
    DrawCommandType type;
    bool operator==(const BatchKey& other) const {
        return texture_id == other.texture_id && z_index == other.z_index && type == other.type;
    }
};

namespace std {
    template <>
    struct hash<BatchKey> {
        std::size_t operator()(const BatchKey& k) const {
            const auto hash =
                (std::hash<Uint32>()(k.texture_id) ^ (std::hash<int>()(k.z_index) << 1)) >> 1 ^ std::hash<int>()(static_cast<int>(k.type));
            return hash;
        }
    };
} // namespace std


struct Batch {
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;
    Uint32 texture_id    = 0;
    DrawCommandType type = DrawCommandType::NONE;
    int z_index          = 0;
};


/**
 * @brief Renderer interface
 *
 * Defines the base API for rendering operations.
 *
 * @version 0.0.1
 */
class Renderer {
public:
    virtual ~Renderer() = default;

    int Viewport[2] = {800, 600};
    SDL_Window* Window = nullptr;
    RendererType Type = RendererType::OPENGL;

    virtual void initialize() = 0;

    virtual bool load_font(const std::string& font_path, const std::string& font_alias, int font_size = 48) = 0;

    virtual void set_current_font(const std::string& font_name) = 0;

    virtual Texture& load_texture(const std::string& path) = 0;

    virtual Texture& get_texture(const std::string& path) = 0;

    virtual void draw_texture(const Texture& texture, const Rect2& dest_rect, float rotation, const glm::vec4& color = glm::vec4(1.0f),
                              const Rect2& src_rect = {0, 0, 0, 0}, int z_index = 0) = 0;

    virtual void draw_rect(Rect2 rect, float rotation, const glm::vec4& color, bool filled = true, int z_index = 0) = 0;

    virtual void draw_text(const std::string& text, float x, float y, float rotation, float scale, const glm::vec4& color,
                           const std::string& font_alias = "", int z_index = 0) = 0;

    virtual void draw_line(float x1, float y1, float x2, float y2, float width, const glm::vec4& color, int z_index = 0, float rotation = 0.0f) = 0;

    virtual void draw_triangle(float x1, float y1, float x2, float y2, float x3, float y3,float rotation, const glm::vec4& color, bool filled = true,
                              int z_index = 0 ) = 0;

    virtual void draw_circle(float center_x, float center_y, float rotation, float radius, const glm::vec4& color, bool filled = true, int segments = 32,
                             int z_index = 0) = 0;

    virtual void draw_polygon(const std::vector<glm::vec2>& points, float rotation, const glm::vec4& color, bool filled = true, int z_index = 0) = 0;

    virtual void render_command(const DrawCommand& cmd) = 0;

    virtual void flush() = 0;
    virtual void clear(const glm::vec4& color = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f)) = 0;

    virtual void resize(int view_width, int view_height) = 0;
    virtual void set_context(const void* ctx) = 0;
    virtual void* get_context() = 0;
    virtual void destroy() = 0;
    virtual void unload_font(const Font& font) = 0;
    virtual void unload_texture(Uint32 id) = 0;
protected:
    std::unordered_map<BatchKey, Batch> batches;

    FT_Library ft = {};

    std::unordered_map<std::string, Font> fonts;
    std::string current_font_name;

    glm::mat4 projection = glm::mat4(1.f);
    std::vector<DrawCommand> commands;

    std::unordered_map<std::string, std::unique_ptr<Texture>> textures;

    glm::vec2 rotate_point(const glm::vec2& point, const glm::vec2& center, float rotation) {
        float s           = sin(rotation);
        float c           = cos(rotation);
        glm::vec2 rel     = point - center;
        glm::vec2 rot_rel = glm::vec2(rel.x * c - rel.y * s, rel.x * s + rel.y * c);
        return center + rot_rel;
    }

    void add_quad_to_batch(const BatchKey& key, float x, float y, float w, float h, float u0, float v0, float u1, float v1,
                           const glm::vec4& color, float rotation = 0.0f) {
        Batch& batch     = batches[key];
        batch.texture_id = key.texture_id;
        batch.type       = key.type;
        batch.z_index    = key.z_index;
        uint32_t base    = batch.vertices.size();

        glm::vec2 center = glm::vec2(x + w * 0.5f, y + h * 0.5f);

        batch.vertices.push_back({rotate_point({x, y + h}, center, rotation), {u0, v1}, color});
        batch.vertices.push_back({rotate_point({x + w, y + h}, center, rotation), {u1, v1}, color});
        batch.vertices.push_back({rotate_point({x + w, y}, center, rotation), {u1, v0}, color});
        batch.vertices.push_back({rotate_point({x, y}, center, rotation), {u0, v0}, color});
        batch.indices.insert(batch.indices.end(), {base, base + 1, base + 2, base + 2, base + 3, base});
    }
};
