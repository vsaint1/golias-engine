#pragma once

#include "core/component/camera.h"
#include "core/engine_structs.h"

#pragma region OPENGL/ES
#include "core/renderer/mesh.h"
#include "core/renderer/opengl/shader_gl.h"
#pragma endregion

#pragma region METAL
#pragma endregion

#include "core/audio/ember_audio.h"
#include "core/ember_utils.h"
#include "core/system_info.h"

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
    Renderer()          = default;

    SDL_Window* Window = nullptr;
    RendererType Type  = OPENGL;
    int Viewport[2]    = {480, 270};

    virtual void setup_shaders(Shader* default_shader,Shader* text_shader) = 0;

    virtual void initialize() = 0;

    virtual void flush() = 0;

    virtual void flush_text() = 0;

    /**
     * @brief Load a texture to the GPU.
     *
     * @param file_path Path to the texture file in the assets folder.
     * @return Texture Loaded texture handle.
     */
    virtual Texture load_texture(const std::string& file_path) = 0;

    /**
     * @brief Load a TTF font to the GPU.
     *
     * @param file_path Path to the TTF file in the assets folder.
     * @param font_size Desired font size.
     * @return Font Loaded font handle.
     */
    virtual Font load_font(const std::string& file_path, int font_size) = 0;

    /**
     * @brief Unload a font from CPU/GPU.
     *
     * @param font Font to unload.
     */
    virtual void unload_font(const Font& font) = 0;

    /**
     * @brief Unload a texture from the GPU.
     *
     * @param texture Texture to unload.
     */
    virtual void unload_texture(const Texture& texture) = 0;

    /**
     * @brief Clear the window background with a color.
     *
     * @param color Color in RGBA.
     */
    virtual void clear_background(const Color& color = {120, 100, 100, 255}) = 0;

    /**
     * @brief Begin the drawing procedure.
     *
     * Clears the background and sets up drawing with an orthographic projection.
     *
     * Example:
     * @code
     * renderer::begin_drawing();
     *  // Here goes your draw calls ordered
     * renderer::end_drawing();
     * @endcode
     *
     * @param view_projection The combined view + projection matrix.
     */
    virtual void begin_drawing(const glm::mat4& view_projection = glm::mat4(1.f)) = 0;

    /**
     * @brief End the drawing procedure and swap buffers.
     */
    virtual void end_drawing() = 0;

    /**
     * @brief Draw text using a loaded font.
     *
     * @param font The loaded TTF font.
     * @param text The text to render.
     * @param transform Position, rotation, and scale.
     * @param color RGBA color.
     * @param font_size Size of the text.
     * @param shader_effect Shader effect (e.g., glow, outline).
     * @param kerning Spacing between characters.
     */
    virtual void draw_text(const Font& font, const std::string& text, const Transform& transform, Color color, float font_size,
                           const ShaderEffect& shader_effect = {}, float kerning = 0.0f) = 0;

    /**
     * @brief Draw a texture quad.
     *
     * @param texture The loaded texture.
     * @param transform Transform of the quad.
     * @param size Size of the quad.
     * @param color RGBA color tint.
     */
    virtual void draw_texture(const Texture& texture, const Transform& transform, glm::vec2 size,
                              const Color& color = {255, 255, 255, 255}) = 0;

    /**
     * @brief Draw an extended texture quad (e.g., for spritesheets).
     *
     * @param texture The loaded texture.
     * @param source Source rectangle.
     * @param dest Destination rectangle.
     * @param origin Origin point (e.g., center).
     * @param rotation Rotation in radians.
     * @param color RGBA color tint.
     */
    virtual void draw_texture_ex(const Texture& texture, const ember::Rectangle& source, const ember::Rectangle& dest, glm::vec2 origin,
                                 float rotation, const Color& color = {255, 255, 255, 255}) = 0;

    /**
     * @brief Draw a line between two 3D points.
     *
     * @param start Start point.
     * @param end End point.
     * @param color RGBA color.
     * @param thickness Line thickness.
     */
    virtual void draw_line(glm::vec3 start, glm::vec3 end, const Color& color = {255, 255, 255, 255}, float thickness = 1.f) = 0;

    /**
     * @brief Draw a filled triangle.
     *
     * @param p0 First vertex.
     * @param p1 Second vertex.
     * @param p2 Third vertex.
     * @param color RGBA color.
     */
    virtual void draw_triangle_filled(glm::vec3 p0, glm::vec3 p1, glm::vec3 p2, const Color& color = {255, 255, 255, 255}) = 0;

    /**
     * @brief Draw a triangle outline.
     *
     * @param p0 First vertex.
     * @param p1 Second vertex.
     * @param p2 Third vertex.
     * @param color RGBA color.
     */
    virtual void draw_triangle(glm::vec3 p0, glm::vec3 p1, glm::vec3 p2, const Color& color = {255, 255, 255, 255}) = 0;

    /**
     * @brief Draw a circle outline.
     *
     * @param position Center position.
     * @param radius Circle radius.
     * @param color RGBA color.
     * @param segments Number of segments.
     */
    virtual void draw_circle(glm::vec3 position, float radius, const Color& color = {255, 255, 255, 255}, int segments = 32) = 0;

    /**
     * @brief Draw a filled circle.
     *
     * @param position Center position.
     * @param radius Circle radius.
     * @param color RGBA color.
     * @param segments Number of segments.
     */
    virtual void draw_circle_filled(glm::vec3 position, float radius, const Color& color = {255, 255, 255, 255}, int segments = 32) = 0;

    /**
     * @brief Draw a rectangle outline.
     *
     * @param transform Transform of the rectangle.
     * @param size Rectangle size.
     * @param color RGBA color.
     * @param thickness Line thickness.
     */
    virtual void draw_rect(const Transform& transform, glm::vec2 size, const Color& color = {255, 255, 255, 255},
                           float thickness = 1.f) = 0;

    /**
     * @brief Draw a filled rectangle.
     *
     * @param transform Transform of the rectangle.
     * @param size Rectangle size.
     * @param color RGBA color.
     * @param thickness Optional thickness (may not be used).
     */
    virtual void draw_rect_filled(const Transform& transform, glm::vec2 size, const Color& color = {255, 255, 255, 255},
                                float thickness = 1.f) = 0;


    /**
     * @brief Begin a static UI canvas.
     * @deprecated
     */
    virtual void BeginCanvas() = 0;

    /**
     * @brief End the UI canvas and restore state.
     * @deprecated
     */
    virtual void EndCanvas() = 0;

    /**
     * @brief Resize the rendering viewport.
     *
     * @param view_width New width.
     * @param view_height New height.
     */
    virtual void resize(int view_width, int view_height) = 0;

    /**
     * @brief Set the current rendering context.
     *
     * @param ctx Context pointer (e.g., SDL_GLContext, MTLDevice).
     */
    virtual void set_context(const void* ctx) = 0;

    /**
     * @brief Get the current rendering context.
     *
     * @return Context pointer.
     */
    virtual void* get_context() = 0;

    /**
     * @brief Destroy the renderer and free resources.
     */
    virtual void destroy() = 0;

private:
    /**
     * @brief Bind to texture slot before drawing.
     */
    virtual float _bind_texture(Uint32 slot = 0) = 0;

    /**
     * @brief Submit a quad to the `CPU` queue to be flushed
     */
    virtual void _submit(const Transform& transform, glm::vec2 size, glm::vec4 color, Uint32 slot = UINT32_MAX) = 0;
};
