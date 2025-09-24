#pragma once

#include "core/component/components.h"

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

    virtual void draw_rect(const Transform2D& transform, float w, float h, glm::vec4 color = glm::vec4(1, 1, 1, 1),
                           bool is_filled = false) = 0;

    virtual void draw_triangle(const Transform2D& transform, float size, glm::vec4 color = glm::vec4(1, 1, 1, 1),
                               bool is_filled = false) = 0;

    virtual void draw_line(const Transform2D& transform, glm::vec2 end, glm::vec4 color = glm::vec4(1, 1, 1, 1)) = 0;

    virtual void draw_circle(const Transform2D& transform, float radius, glm::vec4 color = glm::vec4(1, 1, 1, 1),
                             bool is_filled = false) = 0;

    virtual void draw_polygon(const Transform2D& transform, const std::vector<glm::vec2>& points, glm::vec4 color = glm::vec4(1, 1, 1, 1),
                              bool is_filled = false) = 0;

    virtual ~Renderer() = default;

private:
    SDL_Window* _window = nullptr;
};
