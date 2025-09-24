#pragma once

#include "core/system/logging.h"

struct Transform2D {
    glm::vec2 position = {0, 0};
    glm::vec2 scale    = {1, 1};
    float rotation     = 0;
};

class Renderer {
public:
    virtual bool initialize(SDL_Window* window) = 0;

    virtual void clear(glm::vec4 color = glm::vec4(0, 0, 0, 1)) = 0;

    virtual void present() = 0;

    virtual void draw_rect(const Transform2D& transform, float w, float h, glm::vec4 color = glm::vec4(1, 1, 1, 1), bool is_filled = false) = 0;

    virtual void draw_triangle(const Transform2D& transform, float size, glm::vec4 color = glm::vec4(1, 1, 1, 1), bool is_filled = false) = 0;
    
    virtual void draw_line(const Transform2D& transform, glm::vec2 end, glm::vec4 color = glm::vec4(1, 1, 1, 1)) = 0;

    virtual void draw_circle(const Transform2D& transform, float radius, glm::vec4 color = glm::vec4(1, 1, 1, 1), bool is_filled = false) = 0;

    virtual void draw_polygon(const Transform2D& transform, const std::vector<glm::vec2>& points, glm::vec4 color = glm::vec4(1, 1, 1, 1), bool is_filled = false) = 0;

    virtual void shutdown() = 0;

private:
    SDL_Window* _window = nullptr;
};
