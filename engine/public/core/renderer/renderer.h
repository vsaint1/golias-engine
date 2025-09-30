#pragma once

#include "core/component/logic/system_logic.h"
#include "core/ember_utils.h"
#include "core/renderer/base_struct.h"


/*!

    @brief Abstract base class for different rendering backends.
    This class defines the interface for rendering operations such as initializing the renderer, clearing the screen, drawing shapes, textures, text and presenting the rendered content.

    @version 0.0.1
*/
class Renderer {
public:
    virtual bool initialize(SDL_Window* window) = 0;

    virtual void clear(glm::vec4 color = glm::vec4(0, 0, 0, 1)) = 0;

    virtual void present() = 0;

    void set_default_fonts(const std::string& text_font, const std::string& emoji_font);

    virtual bool load_font(const std::string& name, const std::string& path, int size = 16) = 0;

    virtual std::shared_ptr<Texture> load_texture(const std::string& name, const std::string& path = "") = 0;

    virtual void draw_texture(const Transform2D& transform, Texture* texture, const glm::vec4& dest, const glm::vec4& source,
                              bool flip_h = false, bool flip_v = false, const glm::vec4& color = glm::vec4(1, 1, 1, 1)) = 0;


    virtual void draw_text(const Transform2D& transform, const glm::vec4& color, const std::string& font_name, const char* fmt, ...) = 0;

    virtual void draw_rect(const Transform2D& transform, float w, float h, glm::vec4 color = glm::vec4(1, 1, 1, 1),
                           bool is_filled = false) = 0;


    virtual void draw_line_3d(const glm::vec3& from, const glm::vec3& to, const glm::vec4& color) {
        LOG_WARN("draw_line_3d not implemented for this renderer");
    }

    virtual void draw_triangle_3d(const glm::vec3& v1, const glm::vec3& v2, const glm::vec3& v3, const glm::vec4& color, bool is_filled) {
        LOG_WARN("draw_triangle_3d not implemented for this renderer");
    }

    virtual void draw_triangle(const Transform2D& transform, float size, glm::vec4 color = glm::vec4(1, 1, 1, 1),
                               bool is_filled = false) = 0;

    virtual void draw_line(const Transform2D& transform, glm::vec2 end, glm::vec4 color = glm::vec4(1, 1, 1, 1)) = 0;

    virtual void draw_circle(const Transform2D& transform, float radius, glm::vec4 color = glm::vec4(1, 1, 1, 1),
                             bool is_filled = false) = 0;

    virtual void draw_polygon(const Transform2D& transform, const std::vector<glm::vec2>& points, glm::vec4 color = glm::vec4(1, 1, 1, 1),
                              bool is_filled = false) = 0;

    virtual ~Renderer() = default;

    virtual void draw_model(const Transform3D& t, const Model* model, const glm::mat4& view, const glm::mat4& projection,
                            const glm::vec3& viewPos) {
        LOG_WARN("draw_model not implemented for this renderer");
    }

    virtual void draw_cube(const Transform3D& transform, const glm::mat4& view, const glm::mat4& proj, Uint32 shader) {
        LOG_WARN("draw_cube not implemented for this renderer");
    }

    virtual void draw_environment(const glm::mat4& view, const glm::mat4& projection) {
        LOG_WARN("draw_environment not implemented for this renderer");
    }

    virtual std::shared_ptr<Model> load_model(const char* path) {

        LOG_WARN("LoadModel not implemented for this renderer");
        return nullptr;
    }

protected:
    SDL_Window* _window = nullptr;

    std::string vformat(const char* fmt, va_list args);

    virtual std::vector<Tokens> parse_text(const std::string& text) = 0;

    virtual void draw_text_internal(const glm::vec2& pos, const glm::vec4& color, const std::string& font_name,
                                    const std::string& text) = 0;


    // TODO: consider using resource manager for models, textures, fonts
    std::unordered_map<std::string, std::shared_ptr<Model>> _models;

    std::unordered_map<std::string, std::shared_ptr<Texture>> _textures;

    std::unordered_map<std::string, std::shared_ptr<Font>> _fonts;

    std::string _default_font_name;
    std::string _emoji_font_name;
};
