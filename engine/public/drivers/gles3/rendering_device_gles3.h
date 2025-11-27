#pragma once

#include "servers/rendering/rendering_device.h"
#include <glad.h>

class RenderingDeviceGLES3 final : public RenderingDevice {
public:
    RenderingDeviceGLES3() = default;
    ~RenderingDeviceGLES3() override;

    bool initialize() override;
    void shutdown() override;

    RID shader_create_from_source(const String& vertex_src, const String& fragment_src) override;
    void shader_destroy(RID shader) override;

    RID buffer_create(size_t size, uint32_t usage_flags, const void* data = nullptr) override;
    void buffer_update(RID buffer, size_t offset, size_t size, const void* data) override;
    void buffer_destroy(RID buffer) override;

    RID texture_create(const TextureFormat& format, void* data = nullptr) override;
    void texture_update(RID texture, uint32_t mip_level, uint32_t layer, const void* data, size_t size) override;
    void texture_generate_mipmaps(RID texture) override;
    void texture_destroy(RID texture) override;
    void get_texture_size(RID texture, uint32_t& width, uint32_t& height) override;

    RID sampler_create(const SamplerState& state) override;
    void sampler_destroy(RID sampler) override;

    RID framebuffer_create(const Vector<RenderPassAttachment>& attachments) override;
    void framebuffer_destroy(RID framebuffer) override;

    RID pipeline_create(const PipelineState& state) override;
    void pipeline_destroy(RID pipeline) override;

    void begin_frame() override;
    void end_frame() override;

    void render_pass_begin(RID framebuffer, const Viewport& viewport, const Scissor& scissor) override;
    void render_pass_end() override;

    void bind_pipeline(RID pipeline) override;
    void bind_vertex_buffers(const Vector<RID>& buffers, const Vector<size_t>& offsets = {}) override;
    void bind_index_buffer(RID buffer, IndexType type, size_t offset = 0) override;
    void bind_uniform_buffer(uint32_t binding, RID buffer, size_t offset = 0, size_t size = 0) override;
    void bind_texture(uint32_t binding, RID texture, RID sampler) override;

    void push_constant(const String& name, const void* data, size_t size) override;

    void draw(uint32_t vertex_count, uint32_t instance_count = 1, uint32_t first_vertex = 0, uint32_t first_instance = 0) override;
    void draw_indexed(uint32_t index_count, uint32_t instance_count = 1, uint32_t first_index = 0, int32_t vertex_offset = 0,
                      uint32_t first_instance = 0) override;

    void set_viewport(const Viewport& viewport) override;
    void set_scissor(const Scissor& scissor) override;
    void clear_color(const glm::vec4& color) override;
    void clear_depth_stencil(float depth = 1.0f, uint32_t stencil = 0) override;

private:
    // TODO: move all this code to abstract and have only GLES-specific code here
    struct ShaderModule {
        GLuint program = 0;
        HashMap<String, GLint> uniform_locations;
    };

    struct Buffer {
        GLuint buffer        = 0;
        size_t size          = 0;
        uint32_t usage_flags = 0;
        GLenum target        = GL_ARRAY_BUFFER;
    };

    struct Texture {
        GLuint texture = 0;
        TextureFormat format;
    };

    struct Sampler {
        GLuint sampler = 0;
        SamplerState state;
    };

    struct Framebuffer {
        GLuint framebuffer = 0;
       Vector<RenderPassAttachment> attachments;
        uint32_t width = 0, height = 0;
    };

    struct Pipeline {
        PipelineState state;
        GLuint vao = 0;
    };

    HashMap<RID, ShaderModule> shaders;
    HashMap<RID, Buffer> buffers;
    HashMap<RID, Texture> textures;
    HashMap<RID, Sampler> samplers;
    HashMap<RID, Framebuffer> framebuffers;
    HashMap<RID, Pipeline> pipelines;

    RID current_pipeline         = INVALID_RID;
    RID current_framebuffer      = INVALID_RID;
    IndexType current_index_type = IndexType::UINT16;

    GLuint compile_shader(GLenum type, const String& source);
    GLenum to_gl_format(DataFormat format, bool* is_int = nullptr);
    GLenum to_gl_filter(TextureFilter filter);
    GLenum to_gl_wrap(TextureWrap wrap);
    GLenum to_gl_compare(CompareOp op);
    GLenum to_gl_stencil_op(StencilOp op);
    GLenum to_gl_blend_factor(BlendFactor factor);
    GLenum to_gl_blend_op(BlendOp op);
    GLenum to_gl_topology(PrimitiveTopology topology);
    void apply_rasterization_state(const RasterizationState& state);
    void apply_depth_stencil_state(const DepthStencilState& state);
    void apply_blend_state(const Vector<BlendState>& states);
};
