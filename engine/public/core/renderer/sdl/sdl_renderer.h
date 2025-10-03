#pragma once

#include "core/renderer/renderer.h"
#include "core/renderer/sdl/sdl_struct.h"



/*!

    @file sdl_renderer.h
    @brief SDLRenderer class definition.

    This file contains the definition of the SDLRenderer class, which is a concrete implementation of the Renderer interface using the SDL library for rendering operations.

    @version 0.0.1

*/
class SDLRenderer final: public Renderer {
public:
    bool initialize(SDL_Window* window) override;

    void clear(glm::vec4 color) override;

    void present() override;

    void* get_context() override{
        return (void*)_renderer;
    }

    bool load_font(const std::string& name, const std::string& path, int size) override;
    
    std::shared_ptr<Texture> load_texture(const std::string& name, const std::string& path) override;

    void draw_texture(const Transform2D& transform, Texture* texture, const glm::vec4& dest, const glm::vec4& source,
                      bool flip_h, bool flip_v, const glm::vec4& color) override;


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
