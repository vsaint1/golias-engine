#pragma once

#include "renderer.h"


class SDLRenderer : public Renderer {
public:
    bool initialize(SDL_Window* window) override;

    void clear(glm::vec4 color) override;

    void present() override;

    void draw_line(const Transform2D& transform, glm::vec2 end, glm::vec4 color) override;

    void draw_rect(const Transform2D& transform, float w, float h, glm::vec4 color, bool is_filled ) override;

    void draw_triangle(const Transform2D& transform, float size, glm::vec4 color, bool is_filled) override;
    
    void draw_circle(const Transform2D& transform,float radius, glm::vec4 color, bool is_filled) override;

    void draw_polygon(const Transform2D& transform, const std::vector<glm::vec2>& points, glm::vec4 color, bool is_filled) override;

    void shutdown() override;

private:
    SDL_Window* _window     = nullptr;
    SDL_Renderer* _renderer = nullptr;
};
