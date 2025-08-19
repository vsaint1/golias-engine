#pragma once

#include "core/component/components.h"
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

/**
 * @brief Mode of drawing commands.
 *
 */
enum class DrawCommandMode { LINES, TRIANGLES };

/**
 * @brief Types of draw commands supported by the renderer.
 */
enum class DrawCommandType {
    NONE, ///< No command.
    TEXTURE, ///< Draw textured quad.
    TEXT, ///< Draw text.
    LINE, ///< Draw line.
    RECT, ///< Draw rectangle.
    TRIANGLE, ///< Draw triangle.
    CIRCLE, ///< Draw circle.
    POLYGON ///< Draw polygon.
};

/**
 * @brief Represents a single rendering command.
 */
struct DrawCommand {
    DrawCommandType type = DrawCommandType::NONE; ///< Type of draw command.
    std::vector<Vertex> vertices; ///< Vertex data.
    std::vector<uint32_t> indices; ///< Index buffer.
    Uint32 texture_id   = 0; ///< Texture used (if any).
    float line_width    = 1.0f; ///< Line thickness (if applicable).
    int circle_segments = 32; ///< Number of segments for circles.
    int z_index         = 0; ///< Z-index (render order).

    /**
     * @brief Constructs a DrawCommand with type and optional z-index.
     * @param t Type of command.
     * @param z Z-index (default = 0).
     */
    explicit DrawCommand(DrawCommandType t, int z = 0) : type(t), z_index(z) {
    }
};

/**
 * @brief Used to batch draw commands by shared properties.
 */
struct BatchKey {
    Uint32 texture_id; ///< Texture used.
    int z_index; ///< Z-index.
    DrawCommandType type; ///< Type of draw command.
    UberShader uber_shader = UberShader::none(); ///< Shader used for this batch.

    /**
     * @brief Equality operator.
     */
    bool operator==(const BatchKey& other) const {
        return texture_id == other.texture_id && z_index == other.z_index && type == other.type;
    }
};

/**
 * @brief Hash function for BatchKey to use in unordered_map.
 */
template <>
struct std::hash<BatchKey> {
    std::size_t operator()(const BatchKey& k) const {
        std::size_t h1       = std::hash<Uint32>()(k.texture_id);
        std::size_t h2       = std::hash<int>()(k.z_index);
        std::size_t h3       = std::hash<int>()(static_cast<int>(k.type));
        std::size_t combined = h1 ^ (h2 << 1);
        return (combined >> 1) ^ h3;
    }
};

/**
 * @brief A group of vertices and indices that can be rendered together.
 */
struct Batch {
    std::vector<Vertex> vertices; ///< Batched vertex data.
    std::vector<uint32_t> indices; ///< Batched index data.
    Uint32 texture_id      = 0; ///< Texture used in this batch.
    DrawCommandType type   = DrawCommandType::NONE; ///< Type of draw command.
    int z_index            = 0; ///< Render order.
    UberShader uber_shader = UberShader::none(); ///< Shader effects applied.
    DrawCommandMode mode   = DrawCommandMode::TRIANGLES;
    float thickness = 1.0f; ///< Line thickness for lines.
};


/**
 * @brief Base class for all renderers.
 */
class Renderer {
public:
    virtual ~Renderer() = default;

    int Viewport[2]    = {800, 600}; ///< Viewport size.
    SDL_Window* Window = nullptr; ///< SDL Window.
    Backend Type  = Backend::OPENGL; ///< Default Type of renderer.

    /** @brief Initialize the renderer and its resources. */
    virtual void initialize() = 0;

    /**
     * @brief Load a font.
     * @param font_path Path to TTF file.
     * @param font_alias Alias name to refer to this font.
     * @param font_size Size of font in pixels.
     * @return true if loaded successfully.
     */
    virtual bool load_font(const std::string& font_path, const std::string& font_alias, int font_size = 48) = 0;


    /**
     * @brief Load a texture from disk.
     * @param path Path to image file.
     * @return Reference to the loaded Texture.
     */
    virtual std::shared_ptr<Texture> load_texture(const std::string& path) = 0;

    /**
     * @brief Get a previously loaded texture.
     * @param path Path or alias of the texture.
     * @return Reference to the Texture.
     */
    virtual std::shared_ptr<Texture> get_texture(const std::string& path) = 0;

    /**
     * @brief Draw a textured quad.
     */
    virtual void draw_texture(const Texture* texture, const Rect2& dest_rect, float rotation, const glm::vec4& color = glm::vec4(1.0f),
                              const Rect2& src_rect = Rect2(), int z_index = 0,
                              const UberShader& uber_shader = UberShader::none()) = 0;

    /**
     * @brief Draw a rectangle (filled or outlined).
     */
    virtual void draw_rect(Rect2 rect, float rotation, const glm::vec4& color, bool filled = true, int z_index = 0) = 0;

    /**
     * @brief Draw text to screen.
     */
    virtual void draw_text(const std::string& text, float x, float y, float rotation, float scale, const glm::vec4& color,
                           const std::string& font_alias = "", int z_index = 0,
                           const UberShader& uber_shader = UberShader::none(),int ft_size = 0) = 0;

    /**
     * @brief Draw a line.
     */
    virtual void draw_line(float x1, float y1, float x2, float y2, float thickness, float rotation, const glm::vec4& color,
                           int z_index = 0) = 0;

    /**
     * @brief Draw a triangle.
     */
    virtual void draw_triangle(float x1, float y1, float x2, float y2, float x3, float y3, float rotation, const glm::vec4& color,
                               bool filled = true, int z_index = 0) = 0;

    /**
     * @brief Draw a circle.
     */
    virtual void draw_circle(float center_x, float center_y, float rotation, float radius, const glm::vec4& color, bool filled = true,
                             int segments = 32, int z_index = 0) = 0;

    /**
     * @brief Draw a polygon.
     */
    virtual void draw_polygon(const std::vector<glm::vec2>& points, float rotation, const glm::vec4& color, bool filled = true,
                              int z_index = 0) = 0;


    /**
     * @brief Submit all batched draw calls.
     */
    virtual void flush() = 0;

    /**
     * @brief Swap buffers to present the rendered frame.
     *
     * @details  This function is called to display the rendered content on the screen.
     *
     */
    virtual void present() = 0;

    /**
     * @brief Clear the screen to the given color.
     * @param  color Color to clear the screen with (default is black).
     */
    virtual void clear(glm::vec4 color = glm::vec4(0.0f, 0.0f, 0.0f, 0.0f)) = 0;

    /**
     * @brief Resize the rendering context.
     * @param  view_width Width of the view.
     * @param  view_height Height of the view.
     */
    virtual void resize_viewport(int view_width, int view_height) = 0;

    /**
     * @brief Set the platform-specific rendering context.
     */
    virtual void set_context(const void* ctx) = 0;

    /**
     * @brief Get the platform-specific rendering context.
     */
    virtual void* get_context() = 0;

    /**
     * @brief Destroy rendering resources.
     */
    virtual void destroy() = 0;

    /**
     * @brief Unload a loaded font.
     */
    virtual void unload_font(const Font& font) = 0;

    /**
     * @brief Unload a texture by its ID.
     */
    virtual void unload_texture(Uint32 id) = 0;

protected:
    HashMap<BatchKey, Batch> batches; ///< All batches by key.

    FT_Library ft = {}; ///< FreeType library instance.

    HashMap<std::string, Font> fonts; ///< Loaded fonts.
    std::string current_font_name; ///< Currently selected font alias.

    glm::mat4 projection = glm::mat4(1.f); ///< Projection matrix.
    std::vector<DrawCommand> commands; ///< Commands to render this frame.

    HashMap<std::string, std::shared_ptr<Texture>> textures; ///< Cached textures.

    HashMap<Uint32, glm::vec2> _texture_sizes; ///< HACK_FIX: Cached texture sizes by ID.

    /**
     * @brief Process a draw command.
     */
    virtual void _render_command(const DrawCommand& cmd) = 0;

    /**
    * @brief Set the current font to use for rendering.
    * @param font_name Alias of the font to use.
    */
    virtual void _set_default_font(const std::string& font_name) = 0;

    /**
     * @brief Rotate a point around a center point.
     */
    glm::vec2 _rotate_point(const glm::vec2& point, const glm::vec2& center, float radians);

    /**
     * @brief Calculate the display rectangle for rendering based on the viewport and window size.
     *
     * @details This function calculates the rectangle to draw the viewport in a way that maintains the aspect ratio (letterbox or pillarbox).
     *
     * @version  1.0.0
     */
    Recti _calc_display();

    /**
    * @brief Render the current frame buffer.
    */
    virtual void _render_fbo() = 0;

    /**
     * @brief Add a quad (textured or untextured) to the appropriate batch.
     */
    void _add_quad_to_batch(const BatchKey& key, float x, float y, float w, float h, float u0, float v0, float u1, float v1,
                            const glm::vec4& color, float rotation = 0.0f, bool is_filled = true);

    virtual void _set_effect_uniforms(const UberShader& uber_shader, const glm::vec2& texture_size = glm::vec2(1, 1)) = 0;

    [[nodiscard]] virtual glm::vec2 _get_texture_size(Uint32 texture_id) const = 0;
};
