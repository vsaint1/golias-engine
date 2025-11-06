#pragma once
#include "core/io/file_system.h"

enum class CubemapOrientation {
    DEFAULT,
    TOP,
    BOTTOM,
    FLIP_X,
    FLIP_Y
};

enum class TextureFormat {
    UNKNOWN,
    R8, RG8, RGB8, RGBA8,
    R16F, RG16F, RGB16F, RGBA16F,
    R32F, RG32F, RGB32F, RGBA32F,
    DEPTH24, DEPTH32F, DEPTH24_STENCIL8,
    SRGB8, SRGB8_ALPHA8
};

/**
 * @brief Texture filtering modes.
 *
 * Determines how texture pixels are sampled when scaled or transformed.
 *
 * @version  0.0.5
 */
enum class TextureFiltering {
    NEAREST,
    LINEAR,
    NEAREST_MIPMAP_NEAREST,
    LINEAR_MIPMAP_NEAREST,
    NEAREST_MIPMAP_LINEAR,
    LINEAR_MIPMAP_LINEAR
};

enum class TextureWrap {
    REPEAT,
    MIRRORED_REPEAT,
    CLAMP_TO_EDGE,
    CLAMP_TO_BORDER
};

enum class TextureUsage {
    STATIC,
    DYNAMIC,
    RENDER_TARGET
};

struct TextureDesc {
    uint32_t width              = 0;
    uint32_t height             = 0;
    TextureFormat format        = TextureFormat::RGBA8;
    TextureFiltering min_filter = TextureFiltering::LINEAR;
    TextureFiltering mag_filter = TextureFiltering::LINEAR;
    TextureWrap wrap_s          = TextureWrap::REPEAT;
    TextureWrap wrap_t          = TextureWrap::REPEAT;
    bool generate_mipmaps       = false;
    uint32_t mip_levels         = 0;
    TextureUsage usage          = TextureUsage::STATIC;
    float anisotropy            = 1.0f;
    glm::vec4 border_color      = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
};

/*!

    @brief Texture2D Abstract class
    - Create texture from file or memory
    - Bind the texture
    - Get texture properties

    @version  0.0.1

*/
class GpuImage {
public:

    explicit GpuImage(const TextureDesc& desc) : _width(desc.width),
                                                  _height(desc.height),
                                                  _format(desc.format),
                                                  _tex_desc(desc) {
    }

    virtual ~GpuImage() = default;

    virtual bool create(const void* p_data, const TextureDesc& desc) = 0;

    virtual void bind(Uint32 slot = 0) const = 0;

    virtual void unbind() =0;

    virtual void destroy() = 0;

    bool load_from_file(const std::string& path, const TextureDesc& desc);

    bool load_from_memory(const void* data, size_t size, const TextureDesc& desc);

    TextureDesc get_desc() const;

    TextureFormat get_format() const;

    Uint32 get_width() const;

    Uint32 get_height() const;

    Uint32 get_mip_levels() const;

    Uint32 get_id() const;

protected:
    Uint32 _width         = 0;
    Uint32 _height        = 0;
    Uint32 _mip_levels    = 0;
    TextureFormat _format = TextureFormat::RGBA8;
    TextureDesc _tex_desc;

    Uint32 _id = 0;

};


/*!

    @brief GpuBuffer Abstract class
    - Bind the buffer
    - Upload data to the buffer
    - Get buffer size

    @version  0.0.5

*/
enum class GpuBufferType {
    VERTEX, /// For Vertex Buffer Objects (VBOs)
    INDEX, /// For Element Buffer Objects (EBOs)
    UNIFORM, /// For Uniform Buffer Objects (UBOs)
    STORAGE /// For Shader Storage Buffer Objects (SSBOs)
};

/*!

    @brief GpuBuffer Abstract class
    - Bind the buffer
    - Upload data to the buffer
    - Get buffer size

    @version  0.0.5

*/
class GpuBuffer {
public:
    virtual ~GpuBuffer() = default;

    virtual void bind() const = 0;

    virtual void upload(const void* data, size_t size) = 0;

    virtual size_t size() const = 0;

    virtual GpuBufferType type() const = 0;

};

enum class DataType {
    USHORT,
    FLOAT,
    INT,
    UNSIGNED_INT,
};


struct VertexAttribute {
    uint32_t location;
    uint32_t components;
    DataType type;
    bool normalized;
    uint32_t offset;
};

class GpuVertexLayout {
public:
    virtual ~GpuVertexLayout() = default;
    virtual void bind() const = 0;
    virtual void unbind() const = 0;
};


enum class FramebufferTextureFormat {
    None = 0,
    RGBA8,
    RED_INTEGER,
    DEPTH24STENCIL8,
    DEPTH_COMPONENT
};

struct FramebufferTextureSpecification {
    FramebufferTextureFormat format = FramebufferTextureFormat::None;
};

struct FramebufferAttachmentSpecification {
    std::vector<FramebufferTextureSpecification> attachments;
    FramebufferAttachmentSpecification() = default;

    FramebufferAttachmentSpecification(const std::initializer_list<FramebufferTextureSpecification> list)
        : attachments(list) {
    }
};

struct FramebufferSpecification {
    unsigned int width  = 0;
    unsigned int height = 0;
    FramebufferAttachmentSpecification attachments;
    bool swap_chain_target = false;
};


class Framebuffer {
public:
    virtual ~Framebuffer() = default;

    virtual void bind() = 0;
    virtual void unbind() = 0;
    virtual void invalidate() = 0;

    virtual void resize(unsigned int width, unsigned int height) = 0;

    virtual Uint32 get_fbo_id() const {
        return 0;
    }

    virtual uint32_t get_color_attachment_id(size_t index = 0) const = 0;
    virtual uint32_t get_depth_attachment_id() const = 0;

    virtual const FramebufferSpecification& get_specification() const = 0;
};


/*!

    @brief Vertex structure
    - Position
    - Normal
    - UV coordinates

    @version  0.0.1

*/
struct Vertex {
    glm::vec3 position; /// 3D position
    glm::vec3 normal; /// 3D normal
    glm::vec2 uv; /// 2D texture coordinates
};

constexpr int MAX_BONES = 250;


/*!

    @brief Shader Abstract class
    - Compile the shader
    - Bind the shader
    - Send uniforms

    @version  0.0.2
    @param string vertex The shader source
    @param string fragment The shader source
*/
class Shader {
public:
    Shader() = default;

    virtual ~Shader() = default;

    virtual void activate() const = 0;

    virtual void set_value(const char* name, float value) = 0;

    virtual void set_value(const char* name, int value) = 0;

    virtual void set_value(const char* name, Uint32 value) = 0;

    virtual void set_value(const char* name, glm::mat4 value, Uint32 count = 1) =0;

    virtual void set_value(const char* name, const int* value, Uint32 count = 1) =0;

    virtual void set_value(const char* name, const float* value, Uint32 count = 1) =0;

    virtual void set_value(const char* name, glm::vec2 value, Uint32 count = 1) =0;

    virtual void set_value(const char* name, glm::vec3 value, Uint32 count = 1) =0;

    virtual void set_value(const char* name, glm::vec4 value, Uint32 count = 1) =0;

    virtual void set_value(const char* name, const glm::mat4* value, Uint32 count = 1) =0;

    virtual void destroy() = 0;

    virtual Uint32 get_id() const = 0;

    virtual bool is_valid() const = 0;

protected:
    Uint32 id = 0;

    std::unordered_map<std::string, Uint32> _uniforms;
};

// Forward declaration
class Renderer;
struct MeshInstance3D;
struct Material;

struct RenderBatch {
    const MeshInstance3D* mesh;
    const Material* material;
    std::vector<glm::mat4> model_matrices;

    void clear() {
        model_matrices.clear();
    }
};

struct MeshMaterialKey {
    const MeshInstance3D* mesh;
    const Material* material;

    bool operator==(const MeshMaterialKey& other) const {
        return mesh == other.mesh && material == other.material;
    }
};

struct MeshMaterialKeyHash {
    std::size_t operator()(const MeshMaterialKey& key) const {
        std::size_t h1 = std::hash<const void*>{}(key.mesh);
        std::size_t h2 = std::hash<const void*>{}(key.material);
        return h1 ^ (h2 << 1);
    }
};

constexpr Uint32 MAX_VERTICES_2D = 65536;
constexpr Uint32 MAX_INDICES_2D  = MAX_VERTICES_2D * 3;

// ============================================================================
// 2D Rendering Structures
// ============================================================================


/*!
 * @brief Rectangle structure for 2D rendering.
 * @ingroup Core
 */
struct Rect2D {
    float x      = 0.0f;
    float y      = 0.0f;
    float width  = 0.0f;
    float height = 0.0f;

    Rect2D() = default;

    bool is_zero() const {
        return x == 0.0f && y == 0.0f && width == 0.0f && height == 0.0f;
    }

    Rect2D(float w, float h) : width(w), height(h) {
    }

    Rect2D(float x_, float y_, float w, float h) : x(x_), y(y_), width(w), height(h) {
    }
};

enum class FlipMode {
    NONE = 0,
    HORIZONTAL = 1,
    VERTICAL = 2,
    BOTH = 3
};

struct Vertex2D {
    glm::vec2 position;
    glm::vec2 tex_coord;
    glm::vec4 color;
};

enum class DrawMode2D {
    FILLED = 0, /// Filled shapes (triangles, rects)
    LINE = 1, /// Lines
    TEXT = 2, /// Text rendering
    CIRCLE_FILLED = 3, /// Filled circles
    CIRCLE_OUTLINE = 4 /// Circle outlines
};

enum class DrawType2D {
    RECTANGLE,
    CIRCLE,
    LINE,
    TRIANGLE,
    TEXT
};

struct DrawCommand2D {
    DrawType2D type;
    DrawMode2D mode;

    std::vector<Vertex2D> vertices;
    std::vector<uint32_t> indices;

    glm::vec4 color;
    Uint32 texture_id;
    bool use_texture;

    // Shape-specific parameters
    glm::vec2 position;
    glm::vec2 size; // for rectangles
    float radius; // for circles
    float thickness; // for lines and circle outlines
    int segments; // for circles
    bool filled;

    // Text-specific
    std::string text;
    float text_scale;
};


struct TextMesh {
    std::string text;
    TTF_Font* font;
    size_t start_pos;
};
