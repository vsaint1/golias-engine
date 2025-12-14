#pragma once
#include "servers/rendering/rendering_device.h"


class RenderingDeviceSDL_GPU final : public RenderingDevice {
public:
    RenderingDeviceSDL_GPU() = default;
    ~RenderingDeviceSDL_GPU() override;

    bool initialize(SDL_Window* sdl_window) override;
    void shutdown() override;

    RID shader_create_from_source(const String& vertex_src, const String& fragment_src) override;
    RID shader_create_from_bytecode(const void* vertex_bytecode, size_t vertex_size,
                                    const void* fragment_bytecode, size_t fragment_size,
                                    uint32_t num_samplers = 0, uint32_t num_uniform_buffers = 1);
    void shader_destroy(RID shader) override;

    RID buffer_create(size_t size, uint32_t usage_flags, const void* data = nullptr) override;
    void buffer_update(RID buffer, size_t offset, size_t size, const void* data) override;
    void buffer_destroy(RID buffer) override;

    RID texture_create(const TextureFormat& format, void* data = nullptr) override;
    void texture_update(RID texture, uint32_t mip_level, uint32_t layer, const void* data, size_t size) override;
    void texture_generate_mipmaps(RID texture) override;
    void texture_destroy(RID texture) override;
    void get_texture_size(RID texture, uint32_t& width, uint32_t& height) override;
    uint32_t texture_get_native_handle(RID texture) override;
    Texture get_texture(RID texture) override;

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
    void draw_indexed(uint32_t index_count, uint32_t instance_count = 1, uint32_t first_index = 0,
                     int32_t vertex_offset = 0, uint32_t first_instance = 0) override;

    void set_viewport(const Viewport& viewport) override;
    void set_scissor(const Scissor& scissor) override;
    void clear_color(const glm::vec4& color) override;
    void clear_depth_stencil(float depth = 1.0f, uint32_t stencil = 0) override;

    void swap_buffers() override;

private:
    SDL_GPUDevice* _device = nullptr;
    SDL_GPUCommandBuffer* _cmd_buffer = nullptr;
    SDL_GPURenderPass* _render_pass = nullptr;

    struct GPUShader {
        SDL_GPUShader* vertex = nullptr;
        SDL_GPUShader* fragment = nullptr;
    };

    struct GPUBuffer {
        SDL_GPUBuffer* handle = nullptr;
        size_t size = 0;
        uint32_t usage = 0;
    };

    struct GPUTexture {
        SDL_GPUTexture* handle = nullptr;
        TextureFormat format;
    };

    HashMap<RID, GPUShader> _shaders;
    HashMap<RID, GPUBuffer> _buffers;
    HashMap<RID, GPUTexture> _textures;
    HashMap<RID, SDL_GPUSampler*> _samplers;
    HashMap<RID, Vector<RenderPassAttachment>> _framebuffers;
    HashMap<RID, SDL_GPUGraphicsPipeline*> _pipelines;

    RID _current_pipeline = INVALID_RID;
    Viewport _current_viewport;
    glm::vec4 _clear_color = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);

    struct BoundVertexBuffer {
        SDL_GPUBuffer* buffer = nullptr;
        Uint32 offset = 0;
    };

    Vector<BoundVertexBuffer> _bound_vertex_buffers;

    SDL_GPUBuffer* _bound_index_buffer = nullptr;
    SDL_GPUIndexElementSize _bound_index_type = SDL_GPU_INDEXELEMENTSIZE_16BIT;
    Uint32 _bound_index_offset = 0;

    struct BoundTextureSampler {
        SDL_GPUTexture* texture = nullptr;
        SDL_GPUSampler* sampler = nullptr;
    };
    HashMap<uint32_t, BoundTextureSampler> _bound_textures;

    SDL_GPUTextureFormat sdl_format(DataFormat fmt);
    SDL_GPUVertexElementFormat sdl_vertex_format(DataFormat fmt);
    SDL_GPUFilter sdl_filter(TextureFilter filter);
    SDL_GPUSamplerMipmapMode sdl_mipmap_mode(TextureFilter filter);
    SDL_GPUSamplerAddressMode sdl_wrap(TextureWrap wrap);
    SDL_GPUCompareOp sdl_compare(CompareOp op);
    SDL_GPUStencilOp sdl_stencil(StencilOp op);
    SDL_GPUBlendFactor sdl_blend_factor(BlendFactor f);
    SDL_GPUBlendOp sdl_blend_op(BlendOp op);
    SDL_GPUCullMode sdl_cull(CullMode mode);
    SDL_GPUFillMode sdl_fill(PolygonMode mode);
    SDL_GPUPrimitiveType sdl_topology(PrimitiveTopology topo);
    SDL_GPUIndexElementSize sdl_index_type(IndexType type);
    SDL_GPUBufferUsageFlags sdl_buffer_usage(uint32_t flags);
};