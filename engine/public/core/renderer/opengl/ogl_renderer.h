#pragma once

#include "core/renderer/renderer.h"


class OpenglRenderer final : public Renderer {
public:
    virtual bool initialize(SDL_Window* window);

    virtual void clear(glm::vec4 color = glm::vec4(0, 0, 0, 1));

    virtual void present();

    void set_default_fonts(const std::string& text_font, const std::string& emoji_font);

    virtual bool load_font(const std::string& name, const std::string& path, int size = 16);

    virtual std::shared_ptr<Texture> load_texture(const std::string& name, const std::string& path = "");

    virtual void draw_texture(const Transform2D& transform, Texture* texture, const glm::vec4& dest, const glm::vec4& source,
                              bool flip_h = false, bool flip_v = false, const glm::vec4& color = glm::vec4(1, 1, 1, 1));


    virtual void draw_text(const Transform2D& transform, const glm::vec4& color, const std::string& font_name, const char* fmt, ...);

    virtual void draw_rect(const Transform2D& transform, float w, float h, glm::vec4 color = glm::vec4(1, 1, 1, 1),
                           bool is_filled                                                  = false);

    virtual void draw_triangle(const Transform2D& transform, float size, glm::vec4 color = glm::vec4(1, 1, 1, 1),
                               bool is_filled                                            = false);

    virtual void draw_line(const Transform2D& transform, glm::vec2 end, glm::vec4 color = glm::vec4(1, 1, 1, 1));

    virtual void draw_circle(const Transform2D& transform, float radius, glm::vec4 color = glm::vec4(1, 1, 1, 1),
                             bool is_filled                                              = false);

    virtual void draw_polygon(const Transform2D& transform, const std::vector<glm::vec2>& points, glm::vec4 color = glm::vec4(1, 1, 1, 1),
                              bool is_filled                                                                      = false);

    virtual ~OpenglRenderer();

private:

    SDL_GLContext _context = nullptr;

protected:
    GLuint defaultShaderProgram = 0;
    GLint modelLoc, viewLoc, projLoc, viewPosLoc, lightPosLoc, lightColorLoc, materialDiffuseLoc, textureSamplerLoc, useTextureLoc;

    std::string vformat(const char* fmt, va_list args);

    virtual std::vector<Tokens> parse_text(const std::string& text);

    virtual void draw_text_internal(const glm::vec2& pos, const glm::vec4& color, const std::string& font_name,
                                    const std::string& text);


};
