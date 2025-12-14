#pragma once

#include "core/gstl/str.h"
#include "core/math/math.h"
#include "shader_preprocessor.h"
#include <SDL3_ttf/SDL_ttf.h>

#include <glm/gtx/string_cast.hpp>



using RID                 = uint32_t;
constexpr RID INVALID_RID = -1;

struct Shader {
 RID handle =  INVALID_RID;

};

struct Font {

    Font() = default;
    explicit Font(TTF_Font* ptr, int size = 24) : handle(ptr), size(size) {
    }

    TTF_Font* get_native_handle() const {
        return handle;
    }


    void get_text_size(const String& text, int* out_w, int* out_h) const {
        if (!TTF_GetStringSize(handle, text.c_str(), text.length(), out_w, out_h)) {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to get text size: %s", SDL_GetError());
        }
    }

    int get_font_size_internal() const {
        return TTF_GetFontHeight(handle);
    }

    void destroy()  {
        if (handle) {
            TTF_CloseFont(handle);
            handle = nullptr;
        }
    }

private:
    TTF_Font* handle = nullptr;
    int size = 16;

};


struct Color {
    float r, g, b, a;

    Color(float r = 1.0f, float g = 1.0f, float b = 1.0f, float a = 1.0f) : r(r), g(g), b(b), a(a) {
    }

    glm::vec4 to_vec4() const {
        return glm::vec4(r, g, b, a);
    }

    static const Color WHITE;
    static const Color BLACK;
    static const Color RED;
    static const Color GREEN;
    static const Color BLUE;
    static const Color YELLOW;
    static const Color CYAN;
    static const Color MAGENTA;
};


struct Rect {
    float x, y, width, height;
    Rect(float x = 0, float y = 0, float w = 0, float h = 0) : x(x), y(y), width(w), height(h) {
    }
};


enum class DataFormat {
    R8_UNORM,
    R8G8_UNORM,
    R8G8B8_UNORM,
    R8G8B8A8_UNORM,
    R16_SFLOAT,
    R16G16_SFLOAT,
    R16G16B16A16_SFLOAT,
    R32_SFLOAT,
    R32G32_SFLOAT,
    R32G32B32_SFLOAT,
    R32G32B32A32_SFLOAT,
    R32_UINT,
    R32G32_UINT,
    R32G32B32_UINT,
    R32G32B32A32_UINT,
    R32_SINT,
    R32G32_SINT,
    R32G32B32_SINT,
    R32G32B32A32_SINT,
    D24_UNORM_S8_UINT,
    D32_SFLOAT
};

enum class TextureType { TEXTURE_TYPE_2D, TEXTURE_TYPE_2D_ARRAY, TEXTURE_TYPE_3D, TEXTURE_TYPE_CUBEMAP, TEXTURE_TYPE_CUBEMAP_ARRAY };

enum class TextureFilter { NEAREST, LINEAR, NEAREST_MIPMAP_NEAREST, LINEAR_MIPMAP_NEAREST, NEAREST_MIPMAP_LINEAR, LINEAR_MIPMAP_LINEAR };

enum class TextureWrap { REPEAT, MIRRORED_REPEAT, CLAMP_TO_EDGE, CLAMP_TO_BORDER };

enum class CompareOp { NEVER, LESS, EQUAL, LESS_OR_EQUAL, GREATER, NOT_EQUAL, GREATER_OR_EQUAL, ALWAYS };

enum class StencilOp { KEEP, ZERO, REPLACE, INCREMENT_AND_CLAMP, DECREMENT_AND_CLAMP, INVERT, INCREMENT_AND_WRAP, DECREMENT_AND_WRAP };

enum class BlendFactor {
    ZERO,
    ONE,
    SRC_COLOR,
    ONE_MINUS_SRC_COLOR,
    DST_COLOR,
    ONE_MINUS_DST_COLOR,
    SRC_ALPHA,
    ONE_MINUS_SRC_ALPHA,
    DST_ALPHA,
    ONE_MINUS_DST_ALPHA
};

enum class BlendOp { ADD, SUBTRACT, REVERSE_SUBTRACT, MIN, MAX };

enum class CullMode { NONE, FRONT, BACK, FRONT_AND_BACK };

enum class PolygonMode { FILL, LINE, POINT };

enum class PrimitiveTopology { POINTS, LINES, LINE_STRIP, TRIANGLES, TRIANGLE_STRIP, TRIANGLE_FAN };

enum class ShaderStage { VERTEX = 1 << 0, FRAGMENT = 1 << 1, COMPUTE = 1 << 2 };

enum class BufferUsage {
    BUFFER_USAGE_VERTEX       = 1 << 0,
    BUFFER_USAGE_INDEX        = 1 << 1,
    BUFFER_USAGE_UNIFORM      = 1 << 2,
    BUFFER_USAGE_STORAGE      = 1 << 3,
    BUFFER_USAGE_TRANSFER_SRC = 1 << 4,
    BUFFER_USAGE_TRANSFER_DST = 1 << 5
};

enum class IndexType { UINT16, UINT32 };

class RIDAllocator {
public:
    RIDAllocator()          = default;
    virtual ~RIDAllocator() = default;

    RID allocate_rid();

protected:
    RID next_rid = 1;
};

struct VertexAttribute {
    uint32_t location;
    DataFormat format;
    uint32_t offset;
};

struct VertexFormat {
    uint32_t binding = 0;
    uint32_t stride  = 0;
    Vector<VertexAttribute> attributes;
};

struct TextureFormat {
    TextureType type      = TextureType::TEXTURE_TYPE_2D;
    DataFormat format     = DataFormat::R8G8B8A8_UNORM;
    uint32_t width        = 1;
    uint32_t height       = 1;
    uint32_t depth        = 1;
    uint32_t array_layers = 1;
    uint32_t mipmaps      = 1;
    uint32_t samples      = 1;
    bool depth_stencil    = false;
};

struct SamplerState {
    TextureFilter min_filter = TextureFilter::LINEAR;
    TextureFilter mag_filter = TextureFilter::LINEAR;
    TextureWrap wrap_u       = TextureWrap::REPEAT;
    TextureWrap wrap_v       = TextureWrap::REPEAT;
    TextureWrap wrap_w       = TextureWrap::REPEAT;
    float max_anisotropy     = 1.0f;
    bool compare_enabled     = false;
    CompareOp compare_op     = CompareOp::LESS;
    float lod_bias           = 0.0f;
    float min_lod            = 0.0f;
    float max_lod            = 1000.0f;
};

struct RasterizationState {
    CullMode cull_mode        = CullMode::BACK;
    bool front_face_ccw       = true;
    PolygonMode polygon_mode  = PolygonMode::FILL;
    float line_width          = 1.0f;
    bool depth_clamp_enable   = false;
    bool depth_bias_enable    = false;
    float depth_bias_constant = 0.0f;
    float depth_bias_slope    = 0.0f;
};

struct DepthStencilState {
    bool depth_test_enable          = true;
    bool depth_write_enable         = true;
    CompareOp depth_compare_op      = CompareOp::LESS;
    bool stencil_test_enable        = false;
    StencilOp stencil_fail_op       = StencilOp::KEEP;
    StencilOp stencil_depth_fail_op = StencilOp::KEEP;
    StencilOp stencil_pass_op       = StencilOp::KEEP;
    CompareOp stencil_compare_op    = CompareOp::ALWAYS;
    uint32_t stencil_compare_mask   = 0xFF;
    uint32_t stencil_write_mask     = 0xFF;
    uint32_t stencil_reference      = 0;
};

struct BlendState {
    bool enable           = false;
    BlendFactor src_color = BlendFactor::ONE;
    BlendFactor dst_color = BlendFactor::ZERO;
    BlendOp color_op      = BlendOp::ADD;
    BlendFactor src_alpha = BlendFactor::ONE;
    BlendFactor dst_alpha = BlendFactor::ZERO;
    BlendOp alpha_op      = BlendOp::ADD;
    bool write_r = true, write_g = true, write_b = true, write_a = true;
};

struct PipelineState {
    RID shader = INVALID_RID;
    VertexFormat vertex_format;
    PrimitiveTopology topology = PrimitiveTopology::TRIANGLES;
    RasterizationState rasterization;
    DepthStencilState depth_stencil;
    Vector<BlendState> blend_states;
    uint32_t color_attachment_count = 1;
};

struct Viewport {
    float x = 0, y = 0, width = 800, height = 600;
    float min_depth = 0.0f, max_depth = 1.0f;
};

struct Scissor {
    int32_t x = 0, y = 0;
    uint32_t width = 800, height = 600;
};

struct ClearValue {
    glm::vec4 color  = glm::vec4(0, 0, 0, 1);
    float depth      = 1.0f;
    uint32_t stencil = 0;
};

struct RenderPassAttachment {
    RID texture        = INVALID_RID;
    uint32_t mip_level = 0;
    uint32_t layer     = 0;
    bool clear         = true;
    ClearValue clear_value;
};


enum class BlendMode { NONE, ALPHA, ADD, MULTIPLY };

enum class ScaleMode {
    NONE, /// No scaling - stretch to fit
    KEEP, /// Keep aspect ratio - letterbox/pillarbox
    EXPAND /// Expand to fill window - may stretch
};

struct TextureDescription {
    TextureFilter min_filter = TextureFilter::LINEAR;
    TextureFilter mag_filter = TextureFilter::LINEAR;
    TextureWrap wrap_u       = TextureWrap::REPEAT;
    TextureWrap wrap_v       = TextureWrap::REPEAT;
    bool generate_mipmaps    = false;
};

struct ShaderModule {
    uint32_t program = 0;
    HashMap<String, int32_t> uniform_locations;
};

struct Buffer {
    uint32_t handle      = 0;
    size_t size          = 0;
    uint32_t usage_flags = 0;
    uint32_t target      = 0;
};

struct Texture {
    RID rid         = INVALID_RID;
    uint32_t handle = 0;
    TextureFormat format;
};

struct Sampler {
    uint32_t handle = 0;
    SamplerState state;
};

struct Framebuffer {
    uint32_t handle = 0;
    Vector<RenderPassAttachment> attachments;
    uint32_t width  = 0;
    uint32_t height = 0;
};

struct Pipeline {
    PipelineState state;
    uint32_t handle = 0;
};

class RenderingDevice : public RIDAllocator {
public:
    virtual ~RenderingDevice() = default;

    virtual bool initialize(SDL_Window* sdl_window) = 0;
    virtual void shutdown()                         = 0;

    virtual RID shader_create_from_source(const String& vertex_src, const String& fragment_src) = 0;
    virtual void shader_destroy(RID shader)                                                     = 0;

    virtual RID buffer_create(size_t size, uint32_t usage_flags, const void* data = nullptr) = 0;
    virtual void buffer_update(RID buffer, size_t offset, size_t size, const void* data)     = 0;
    virtual void buffer_destroy(RID buffer)                                                  = 0;

    virtual RID texture_create(const TextureFormat& format, void* data = nullptr)                               = 0;
    virtual void texture_update(RID texture, uint32_t mip_level, uint32_t layer, const void* data, size_t size) = 0;
    virtual void texture_generate_mipmaps(RID texture)                                                          = 0;
    virtual void texture_destroy(RID texture)                                                                   = 0;
    virtual void get_texture_size(RID texture, uint32_t& width, uint32_t& height)                               = 0;
    virtual uint32_t texture_get_native_handle(RID texture)                                                     = 0;
    virtual Texture get_texture(RID texture)                                                                    = 0;

    virtual RID sampler_create(const SamplerState& state) = 0;
    virtual void sampler_destroy(RID sampler)             = 0;

    virtual RID framebuffer_create(const Vector<RenderPassAttachment>& attachments) = 0;
    virtual void framebuffer_destroy(RID framebuffer)                               = 0;

    virtual RID pipeline_create(const PipelineState& state) = 0;
    virtual void pipeline_destroy(RID pipeline)             = 0;

    virtual void begin_frame() = 0;
    virtual void end_frame()   = 0;

    virtual void render_pass_begin(RID framebuffer, const Viewport& viewport, const Scissor& scissor) = 0;
    virtual void render_pass_end()                                                                    = 0;

    virtual void bind_pipeline(RID pipeline)                                                           = 0;
    virtual void bind_vertex_buffers(const Vector<RID>& buffers, const Vector<size_t>& offsets = {})   = 0;
    virtual void bind_index_buffer(RID buffer, IndexType type, size_t offset = 0)                      = 0;
    virtual void bind_uniform_buffer(uint32_t binding, RID buffer, size_t offset = 0, size_t size = 0) = 0;
    virtual void bind_texture(uint32_t binding, RID texture, RID sampler)                              = 0;

    virtual void push_constant(const String& name, const void* data, size_t size) = 0;

    virtual void draw(uint32_t vertex_count, uint32_t instance_count = 1, uint32_t first_vertex = 0, uint32_t first_instance = 0) = 0;
    virtual void draw_indexed(uint32_t index_count, uint32_t instance_count = 1, uint32_t first_index = 0, int32_t vertex_offset = 0,
                              uint32_t first_instance = 0)                                                                        = 0;

    virtual void set_viewport(const Viewport& viewport)                        = 0;
    virtual void set_scissor(const Scissor& scissor)                           = 0;
    virtual void clear_color(const glm::vec4& color)                           = 0;
    virtual void clear_depth_stencil(float depth = 1.0f, uint32_t stencil = 0) = 0;

    virtual void swap_buffers() = 0;
protected:
    SDL_Window* _window = nullptr;
};
