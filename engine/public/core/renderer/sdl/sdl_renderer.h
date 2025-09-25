#pragma once

#include "core/renderer/sdl/sdl_struct.h"
#include "core/renderer/renderer.h"


/*!

    @file sdl_renderer.h
    @brief SDLRenderer class definition.

    This file contains the definition of the SDLRenderer class, which is a concrete implementation of the Renderer interface using the SDL library for rendering operations.

    @version 0.0.1

*/
class SDLRenderer : public Renderer {
public:
    bool initialize(SDL_Window* window) override;

    void clear(glm::vec4 color) override;

    void present() override;

    bool load_font(const std::string& name, const std::string& path, int size) override;

    void draw_text(const Transform2D& transform, const glm::vec4& color, const std::string& font_name, const char* fmt, ...) override;

    void draw_line(const Transform2D& transform, glm::vec2 end, glm::vec4 color) override;

    void draw_rect(const Transform2D& transform, float w, float h, glm::vec4 color, bool is_filled) override;

    void draw_triangle(const Transform2D& transform, float size, glm::vec4 color, bool is_filled) override;

    void draw_circle(const Transform2D& transform, float radius, glm::vec4 color, bool is_filled) override;

    void draw_polygon(const Transform2D& transform, const std::vector<glm::vec2>& points, glm::vec4 color, bool is_filled) override;

    ~SDLRenderer() override;


private:
    SDL_Window* _window     = nullptr;
    SDL_Renderer* _renderer = nullptr;


    std::vector<Tokens> parse_text(const std::string& text) override;

    void draw_text_internal(const glm::vec2& pos, const glm::vec4& color, const std::string& font_name, const std::string& text) override;
};
