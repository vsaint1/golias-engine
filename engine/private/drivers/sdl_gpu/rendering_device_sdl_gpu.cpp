#include "drivers/sdl_gpu/rendering_device_sdl_gpu.h"

#include "drivers/sdl_gpu/shaders/default_dxd12.h"
#include "drivers/sdl_gpu/shaders/default_vulkan.h"
#include "drivers/sdl_gpu/shaders/default_metal.h"

RenderingDeviceSDL_GPU::~RenderingDeviceSDL_GPU() = default;

bool RenderingDeviceSDL_GPU::initialize(SDL_Window* sdl_window) {


    const int drivers = SDL_GetNumGPUDrivers();

    for (int i = 0; i < drivers; ++i) {
        const char* name = SDL_GetGPUDriver(i);
        SDL_Log("Available GPU Driver %d: %s", i, name);
    }

    _device = SDL_CreateGPUDevice(SDL_GPU_SHADERFORMAT_SPIRV | SDL_GPU_SHADERFORMAT_MSL | SDL_GPU_SHADERFORMAT_DXIL, true, nullptr);

    if (!_device) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to create GPU device: %s", SDL_GetError());
        return false;
    }

    const char* device_name = SDL_GetGPUDeviceDriver(_device);
    SDL_Log("Using GPU Device Driver: %s", device_name);
    if (!SDL_ClaimWindowForGPUDevice(_device, sdl_window)) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to claim window: %s", SDL_GetError());
        SDL_DestroyGPUDevice(_device);
        _device = nullptr;
        return false;
    }


    _window = sdl_window;

    return true;
}

void RenderingDeviceSDL_GPU::shutdown() {
    if (!_device) {
        return;
    }

    SDL_WaitForGPUIdle(_device);

    for (auto& [rid, shader] : _shaders) {
        if (shader.vertex) {
            SDL_ReleaseGPUShader(_device, shader.vertex);
        }
        if (shader.fragment) {
            SDL_ReleaseGPUShader(_device, shader.fragment);
        }
    }
    _shaders.clear();

    for (auto& [rid, buffer] : _buffers) {
        if (buffer.handle) {
            SDL_ReleaseGPUBuffer(_device, buffer.handle);
        }
    }
    _buffers.clear();

    for (auto& [rid, texture] : _textures) {
        if (texture.handle) {
            SDL_ReleaseGPUTexture(_device, texture.handle);
        }
    }
    _textures.clear();

    for (auto& [rid, sampler] : _samplers) {
        if (sampler) {
            SDL_ReleaseGPUSampler(_device, sampler);
        }
    }
    _samplers.clear();

    _framebuffers.clear();

    for (auto& [rid, pipeline] : _pipelines) {
        if (pipeline) {
            SDL_ReleaseGPUGraphicsPipeline(_device, pipeline);
        }
    }
    _pipelines.clear();

    SDL_ReleaseWindowFromGPUDevice(_device, _window);
    SDL_DestroyGPUDevice(_device);
    _device = nullptr;
}

// TODO: Implement shader compilation from source
RID RenderingDeviceSDL_GPU::shader_create_from_source(const String& vertex_src, const String& fragment_src) {

    if (!_device) {
        return INVALID_RID;
    }

    const SDL_GPUShaderFormat format = SDL_GetGPUShaderFormats(_device);
    spdlog::info("SDL_GPU Shader Format: {} (SPIRV={}, DXIL={}, MSL={})", format, (format & SDL_GPU_SHADERFORMAT_SPIRV) != 0,
                 (format & SDL_GPU_SHADERFORMAT_DXIL) != 0, (format & SDL_GPU_SHADERFORMAT_MSL) != 0);

    if (format & SDL_GPU_SHADERFORMAT_SPIRV) {
        return shader_create_from_bytecode(default_vertex_vulkan, sizeof(default_vertex_vulkan), default_fragment_vulkan,
                                           sizeof(default_fragment_vulkan), 1, 1);
    }

    if (format & SDL_GPU_SHADERFORMAT_MSL) {
        return shader_create_from_bytecode(default_vertex_metal, sizeof(default_vertex_metal), default_fragment_metal,
                                           sizeof(default_fragment_metal), 1, 1);
    }

    if (format & SDL_GPU_SHADERFORMAT_DXIL) {
        return shader_create_from_bytecode(default_vertex_dxd12, sizeof(default_vertex_dxd12), default_fragment_dxd12,
                                           sizeof(default_fragment_dxd12), 1, 1);
    }

    spdlog::error("No supported shader format found");
    return INVALID_RID;
}

RID RenderingDeviceSDL_GPU::shader_create_from_bytecode(const void* vertex_bytecode, size_t vertex_size, const void* fragment_bytecode,
                                                        size_t fragment_size, uint32_t num_samplers, uint32_t num_uniform_buffers) {
    if (!_device || !vertex_bytecode || !fragment_bytecode) {
        return INVALID_RID;
    }

    const SDL_GPUShaderFormat format = SDL_GetGPUShaderFormats(_device);


    SDL_GPUShaderCreateInfo vertex_info = {};
    vertex_info.code                    = (const Uint8*) vertex_bytecode;
    vertex_info.code_size               = vertex_size;
    vertex_info.entrypoint              = "main";
    vertex_info.format                  = format;
    vertex_info.stage                   = SDL_GPU_SHADERSTAGE_VERTEX;
    vertex_info.num_samplers            = 0;
    vertex_info.num_storage_buffers     = 0;
    vertex_info.num_storage_textures    = 0;
    vertex_info.num_uniform_buffers     = num_uniform_buffers;

    SDL_GPUShader* vertex_shader = SDL_CreateGPUShader(_device, &vertex_info);
    if (!vertex_shader) {
        spdlog::error("Failed to create vertex shader: {}", SDL_GetError());
        return INVALID_RID;
    }

    SDL_GPUShaderCreateInfo fragment_info = {};
    fragment_info.code                    = (const Uint8*) fragment_bytecode;
    fragment_info.code_size               = fragment_size;
    fragment_info.entrypoint              = "main";
    fragment_info.format                  = format;
    fragment_info.stage                   = SDL_GPU_SHADERSTAGE_FRAGMENT;
    fragment_info.num_samplers            = num_samplers;
    fragment_info.num_storage_buffers     = 0;
    fragment_info.num_storage_textures    = 0;
    fragment_info.num_uniform_buffers     = num_uniform_buffers;

    SDL_GPUShader* fragment_shader = SDL_CreateGPUShader(_device, &fragment_info);
    if (!fragment_shader) {
        spdlog::error("Failed to create fragment shader: {}", SDL_GetError());
        SDL_ReleaseGPUShader(_device, vertex_shader);
        return INVALID_RID;
    }

    RID rid = allocate_rid();
    GPUShader shader;
    shader.vertex   = vertex_shader;
    shader.fragment = fragment_shader;
    _shaders[rid]   = shader;

    return rid;
}

void RenderingDeviceSDL_GPU::shader_destroy(RID shader) {
    auto it = _shaders.find(shader);

    if (it != _shaders.end()) {
        if (it->second.vertex) {
            SDL_ReleaseGPUShader(_device, it->second.vertex);
        }
        if (it->second.fragment) {
            SDL_ReleaseGPUShader(_device, it->second.fragment);
        }
        _shaders.erase(it);
    }
}

RID RenderingDeviceSDL_GPU::buffer_create(size_t size, uint32_t usage_flags, const void* data) {
    SDL_GPUBufferCreateInfo create_info = {};
    create_info.usage                   = sdl_buffer_usage(usage_flags);
    create_info.size                    = (Uint32) size;

    SDL_GPUBuffer* buffer = SDL_CreateGPUBuffer(_device, &create_info);
    if (!buffer) {
        spdlog::error("Failed to create buffer: {}", SDL_GetError());
        return INVALID_RID;
    }

    if (data) {
        SDL_GPUTransferBufferCreateInfo transfer_info = {};
        transfer_info.usage                           = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD;
        transfer_info.size                            = (Uint32) size;

        SDL_GPUTransferBuffer* transfer = SDL_CreateGPUTransferBuffer(_device, &transfer_info);
        void* mapped                    = SDL_MapGPUTransferBuffer(_device, transfer, false);
        memcpy(mapped, data, size);
        SDL_UnmapGPUTransferBuffer(_device, transfer);

        SDL_GPUCommandBuffer* cmd  = SDL_AcquireGPUCommandBuffer(_device);
        SDL_GPUCopyPass* copy_pass = SDL_BeginGPUCopyPass(cmd);

        SDL_GPUTransferBufferLocation src_loc = {};
        src_loc.transfer_buffer               = transfer;
        src_loc.offset                        = 0;

        SDL_GPUBufferRegion dst_region = {};
        dst_region.buffer              = buffer;
        dst_region.offset              = 0;
        dst_region.size                = (Uint32) size;

        SDL_UploadToGPUBuffer(copy_pass, &src_loc, &dst_region, false);
        SDL_EndGPUCopyPass(copy_pass);
        SDL_SubmitGPUCommandBuffer(cmd);

        SDL_ReleaseGPUTransferBuffer(_device, transfer);
    }

    RID rid       = allocate_rid();
    _buffers[rid] = {buffer, size, usage_flags};
    return rid;
}

void RenderingDeviceSDL_GPU::buffer_update(RID buffer, size_t offset, size_t size, const void* data) {
    auto it = _buffers.find(buffer);
    if (it == _buffers.end() || !data || size == 0) {
        return;
    }

    SDL_GPUTransferBufferCreateInfo transfer_info = {};
    transfer_info.usage                           = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD;
    transfer_info.size                            = (Uint32) size;

    SDL_GPUTransferBuffer* transfer = SDL_CreateGPUTransferBuffer(_device, &transfer_info);
    if (!transfer) {
        spdlog::error("Failed to create transfer buffer: {}", SDL_GetError());
        return;
    }

    void* mapped = SDL_MapGPUTransferBuffer(_device, transfer, false);
    if (!mapped) {
        spdlog::error("Failed to map transfer buffer: {}", SDL_GetError());
        SDL_ReleaseGPUTransferBuffer(_device, transfer);
        return;
    }

    memcpy(mapped, data, size);
    SDL_UnmapGPUTransferBuffer(_device, transfer);

    SDL_GPUCommandBuffer* cmd = SDL_AcquireGPUCommandBuffer(_device);
    if (!cmd) {
        spdlog::error("Failed to acquire command buffer for buffer update: {}", SDL_GetError());
        SDL_ReleaseGPUTransferBuffer(_device, transfer);
        return;
    }

    SDL_GPUCopyPass* copy_pass = SDL_BeginGPUCopyPass(cmd);

    SDL_GPUTransferBufferLocation src_loc = {};
    src_loc.transfer_buffer               = transfer;
    src_loc.offset                        = 0;

    SDL_GPUBufferRegion dst_region = {};
    dst_region.buffer              = it->second.handle;
    dst_region.offset              = (Uint32) offset;
    dst_region.size                = (Uint32) size;

    SDL_UploadToGPUBuffer(copy_pass, &src_loc, &dst_region, false);
    SDL_EndGPUCopyPass(copy_pass);

    SDL_GPUFence* fence = SDL_SubmitGPUCommandBufferAndAcquireFence(cmd);
    if (fence) {
        SDL_WaitForGPUFences(_device, true, &fence, 1);
        SDL_ReleaseGPUFence(_device, fence);
    }

    SDL_ReleaseGPUTransferBuffer(_device, transfer);
}


void RenderingDeviceSDL_GPU::buffer_destroy(RID buffer) {
    auto it = _buffers.find(buffer);

    if (it != _buffers.end()) {
        if (it->second.handle) {
            SDL_ReleaseGPUBuffer(_device, it->second.handle);
        }
        _buffers.erase(it);
    }
}

RID RenderingDeviceSDL_GPU::texture_create(const TextureFormat& format, void* data) {
    SDL_GPUTextureCreateInfo create_info = {};

    if (format.type == TextureType::TEXTURE_TYPE_3D) {
        create_info.type = SDL_GPU_TEXTURETYPE_3D;
    } else if (format.type == TextureType::TEXTURE_TYPE_CUBEMAP) {
        create_info.type = SDL_GPU_TEXTURETYPE_CUBE;
    } else {
        create_info.type = SDL_GPU_TEXTURETYPE_2D;
    }

    create_info.format               = sdl_format(format.format);
    create_info.width                = format.width;
    create_info.height               = format.height;
    create_info.layer_count_or_depth = (format.type == TextureType::TEXTURE_TYPE_3D) ? format.depth : format.array_layers;
    create_info.num_levels           = format.mipmaps == 0 ? 1 : format.mipmaps;

    switch (format.samples) {
    case 2:
        create_info.sample_count = SDL_GPU_SAMPLECOUNT_2;
        break;
    case 4:
        create_info.sample_count = SDL_GPU_SAMPLECOUNT_4;
        break;
    case 8:
        create_info.sample_count = SDL_GPU_SAMPLECOUNT_8;
        break;
    default:
        create_info.sample_count = SDL_GPU_SAMPLECOUNT_1;
        break;
    }

    const bool is_multisample = (create_info.sample_count > SDL_GPU_SAMPLECOUNT_1);

    create_info.usage = 0;
    if (!is_multisample) {
        create_info.usage = SDL_GPU_TEXTUREUSAGE_SAMPLER;
    }

    if (format.depth_stencil) {
        create_info.usage |= SDL_GPU_TEXTUREUSAGE_DEPTH_STENCIL_TARGET;
    } else {
        create_info.usage |= SDL_GPU_TEXTUREUSAGE_COLOR_TARGET;
    }

    SDL_GPUTexture* texture = SDL_CreateGPUTexture(_device, &create_info);
    if (!texture) {
        spdlog::error("Failed to create texture: {}", SDL_GetError());
        return INVALID_RID;
    }

    RID rid = allocate_rid();
    GPUTexture gpu_tex;
    gpu_tex.handle = texture;
    gpu_tex.format = format;
    _textures[rid] = gpu_tex;

    if (data) {
        uint32_t bytes_per_pixel = 4;
        size_t data_size         = format.width * format.height * bytes_per_pixel;
        texture_update(rid, 0, 0, data, data_size);
    }

    return rid;
}


void RenderingDeviceSDL_GPU::texture_update(RID texture, uint32_t mip_level, uint32_t layer, const void* data, size_t size) {


    auto it = _textures.find(texture);
    if (it == _textures.end() || !data) {
        return;
    }

    SDL_GPUTransferBufferCreateInfo transfer_info = {};
    transfer_info.usage                           = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD;
    transfer_info.size                            = (Uint32) size;

    SDL_GPUTransferBuffer* transfer = SDL_CreateGPUTransferBuffer(_device, &transfer_info);
    void* mapped                    = SDL_MapGPUTransferBuffer(_device, transfer, false);
    memcpy(mapped, data, size);
    SDL_UnmapGPUTransferBuffer(_device, transfer);

    SDL_GPUCommandBuffer* cmd  = SDL_AcquireGPUCommandBuffer(_device);
    SDL_GPUCopyPass* copy_pass = SDL_BeginGPUCopyPass(cmd);

    SDL_GPUTextureTransferInfo src_info = {};
    src_info.transfer_buffer            = transfer;
    src_info.offset                     = 0;

    SDL_GPUTextureRegion dst_region = {};
    dst_region.texture              = it->second.handle;
    dst_region.mip_level            = mip_level;
    dst_region.layer                = layer;
    dst_region.x                    = 0;
    dst_region.y                    = 0;
    dst_region.z                    = 0;
    dst_region.w                    = it->second.format.width >> mip_level;
    dst_region.h                    = it->second.format.height >> mip_level;
    dst_region.d                    = 1;

    SDL_UploadToGPUTexture(copy_pass, &src_info, &dst_region, false);
    SDL_EndGPUCopyPass(copy_pass);
    SDL_SubmitGPUCommandBuffer(cmd);

    SDL_ReleaseGPUTransferBuffer(_device, transfer);
}

void RenderingDeviceSDL_GPU::texture_generate_mipmaps(RID texture) {
    spdlog::error("texture_generate_mipmaps not implemented yet (SDL_GPU)");
}

void RenderingDeviceSDL_GPU::texture_destroy(RID texture) {
    auto it = _textures.find(texture);

    if (it != _textures.end()) {
        if (it->second.handle) {
            SDL_ReleaseGPUTexture(_device, it->second.handle);
        }
        _textures.erase(it);
    }
}

void RenderingDeviceSDL_GPU::get_texture_size(RID texture, uint32_t& width, uint32_t& height) {
    auto it = _textures.find(texture);

    if (it != _textures.end()) {
        width  = it->second.format.width;
        height = it->second.format.height;
    }
}

uint32_t RenderingDeviceSDL_GPU::texture_get_native_handle(RID texture) {
    auto it = _textures.find(texture);
    return (it != _textures.end()) ? (uint32_t) (uintptr_t) it->second.handle : 0;
}

Texture RenderingDeviceSDL_GPU::get_texture(RID texture) {
    Texture result;
    auto it = _textures.find(texture);

    if (it != _textures.end()) {
        result.rid    = texture;
        result.handle = (uint32_t) (uintptr_t) it->second.handle;
        result.format = it->second.format;
    }

    return result;
}

RID RenderingDeviceSDL_GPU::sampler_create(const SamplerState& state) {
    SDL_GPUSamplerCreateInfo create_info = {};
    create_info.min_filter               = sdl_filter(state.min_filter);
    create_info.mag_filter               = sdl_filter(state.mag_filter);
    create_info.mipmap_mode              = SDL_GPU_SAMPLERMIPMAPMODE_LINEAR;
    create_info.address_mode_u           = sdl_wrap(state.wrap_u);
    create_info.address_mode_v           = sdl_wrap(state.wrap_v);
    create_info.address_mode_w           = sdl_wrap(state.wrap_w);
    create_info.mip_lod_bias             = state.lod_bias;
    create_info.max_anisotropy           = state.max_anisotropy;
    create_info.compare_op               = sdl_compare(state.compare_op);
    create_info.min_lod                  = state.min_lod;
    create_info.max_lod                  = state.max_lod;
    create_info.enable_anisotropy        = state.max_anisotropy > 1.0f;
    create_info.enable_compare           = state.compare_enabled;

    SDL_GPUSampler* sampler = SDL_CreateGPUSampler(_device, &create_info);
    if (!sampler) {
        spdlog::error("Failed to create sampler: {}", SDL_GetError());
        return INVALID_RID;
    }

    RID rid        = allocate_rid();
    _samplers[rid] = sampler;
    return rid;
}

void RenderingDeviceSDL_GPU::sampler_destroy(RID sampler) {
    auto it = _samplers.find(sampler);

    if (it != _samplers.end()) {
        if (it->second) {
            SDL_ReleaseGPUSampler(_device, it->second);
        }
        _samplers.erase(it);
    }
}

RID RenderingDeviceSDL_GPU::framebuffer_create(const Vector<RenderPassAttachment>& attachments) {
    RID rid            = allocate_rid();
    _framebuffers[rid] = attachments;
    return rid;
}

void RenderingDeviceSDL_GPU::framebuffer_destroy(RID framebuffer) {
    _framebuffers.erase(framebuffer);
}

RID RenderingDeviceSDL_GPU::pipeline_create(const PipelineState& state) {
    auto shader_it = _shaders.find(state.shader);
    if (shader_it == _shaders.end()) {
        spdlog::error("pipeline_create: Shader {} not found", state.shader);
        return INVALID_RID;
    }

    SDL_GPUGraphicsPipelineCreateInfo create_info = {};
    create_info.vertex_shader                     = shader_it->second.vertex;
    create_info.fragment_shader                   = shader_it->second.fragment;

    Vector<SDL_GPUVertexBufferDescription> vertex_buffers;
    Vector<SDL_GPUVertexAttribute> vertex_attributes;

    SDL_GPUVertexBufferDescription vb_desc = {};
    vb_desc.slot                           = state.vertex_format.binding;
    vb_desc.pitch                          = state.vertex_format.stride;
    vb_desc.input_rate                     = SDL_GPU_VERTEXINPUTRATE_VERTEX;
    vertex_buffers.push_back(vb_desc);

    for (const auto& attr : state.vertex_format.attributes) {
        SDL_GPUVertexAttribute va = {};
        va.location               = attr.location;
        va.buffer_slot            = state.vertex_format.binding;
        va.format                 = sdl_vertex_format(attr.format);
        va.offset                 = attr.offset;
        vertex_attributes.push_back(va);
    }

    SDL_GPUVertexInputState vertex_input    = {};
    vertex_input.vertex_buffer_descriptions = vertex_buffers.data();
    vertex_input.num_vertex_buffers         = (Uint32) vertex_buffers.size();
    vertex_input.vertex_attributes          = vertex_attributes.data();
    vertex_input.num_vertex_attributes      = (Uint32) vertex_attributes.size();
    create_info.vertex_input_state          = vertex_input;

    create_info.primitive_type = sdl_topology(state.topology);

    SDL_GPURasterizerState rasterizer = {};
    rasterizer.fill_mode              = sdl_fill(state.rasterization.polygon_mode);
    rasterizer.cull_mode              = sdl_cull(state.rasterization.cull_mode);
    rasterizer.front_face        = state.rasterization.front_face_ccw ? SDL_GPU_FRONTFACE_COUNTER_CLOCKWISE : SDL_GPU_FRONTFACE_CLOCKWISE;
    rasterizer.enable_depth_bias = state.rasterization.depth_bias_enable;
    rasterizer.depth_bias_constant_factor = state.rasterization.depth_bias_constant;
    rasterizer.depth_bias_slope_factor    = state.rasterization.depth_bias_slope;
    create_info.rasterizer_state          = rasterizer;

    SDL_GPUDepthStencilState depth_stencil = {};
    depth_stencil.enable_depth_test        = state.depth_stencil.depth_test_enable;
    depth_stencil.enable_depth_write       = state.depth_stencil.depth_write_enable;
    depth_stencil.compare_op               = sdl_compare(state.depth_stencil.depth_compare_op);
    depth_stencil.enable_stencil_test      = state.depth_stencil.stencil_test_enable;

    depth_stencil.back_stencil_state.fail_op       = sdl_stencil(state.depth_stencil.stencil_fail_op);
    depth_stencil.back_stencil_state.depth_fail_op = sdl_stencil(state.depth_stencil.stencil_depth_fail_op);
    depth_stencil.back_stencil_state.pass_op       = sdl_stencil(state.depth_stencil.stencil_pass_op);
    depth_stencil.back_stencil_state.compare_op    = sdl_compare(state.depth_stencil.stencil_compare_op);
    depth_stencil.compare_mask                     = (Uint8) state.depth_stencil.stencil_compare_mask;
    depth_stencil.write_mask                       = (Uint8) state.depth_stencil.stencil_write_mask;
    depth_stencil.front_stencil_state = depth_stencil.back_stencil_state;

    create_info.depth_stencil_state = depth_stencil;

    Vector<SDL_GPUColorTargetDescription> color_targets;
    for (size_t i = 0; i < state.blend_states.size(); ++i) {
        const auto& blend = state.blend_states[i];

        SDL_GPUColorTargetDescription desc = {};
        desc.format = SDL_GetGPUSwapchainTextureFormat(_device, _window);

        SDL_GPUColorTargetBlendState bs = {};
        bs.enable_blend                 = blend.enable;
        bs.src_color_blendfactor        = sdl_blend_factor(blend.src_color);
        bs.dst_color_blendfactor        = sdl_blend_factor(blend.dst_color);
        bs.color_blend_op               = sdl_blend_op(blend.color_op);
        bs.src_alpha_blendfactor        = sdl_blend_factor(blend.src_alpha);
        bs.dst_alpha_blendfactor        = sdl_blend_factor(blend.dst_alpha);
        bs.alpha_blend_op               = sdl_blend_op(blend.alpha_op);
        bs.color_write_mask             = 0;

        if (blend.write_r) {
            bs.color_write_mask |= SDL_GPU_COLORCOMPONENT_R;
        }
        if (blend.write_g) {
            bs.color_write_mask |= SDL_GPU_COLORCOMPONENT_G;
        }
        if (blend.write_b) {
            bs.color_write_mask |= SDL_GPU_COLORCOMPONENT_B;
        }
        if (blend.write_a) {
            bs.color_write_mask |= SDL_GPU_COLORCOMPONENT_A;
        }

        desc.blend_state = bs;
        color_targets.push_back(desc);
    }

    SDL_GPUGraphicsPipelineTargetInfo target_info = {};
    target_info.color_target_descriptions         = color_targets.data();
    target_info.num_color_targets                 = (Uint32) color_targets.size();
    target_info.depth_stencil_format              = SDL_GPU_TEXTUREFORMAT_D24_UNORM_S8_UINT;
    target_info.has_depth_stencil_target          = state.depth_stencil.depth_test_enable;

    create_info.target_info = target_info;

    SDL_GPUGraphicsPipeline* pipeline = SDL_CreateGPUGraphicsPipeline(_device, &create_info);
    if (!pipeline) {
        spdlog::error("Failed to create pipeline: {}", SDL_GetError());
        return INVALID_RID;
    }

    RID rid         = allocate_rid();
    _pipelines[rid] = pipeline;
    return rid;
}

void RenderingDeviceSDL_GPU::pipeline_destroy(RID pipeline) {
    auto it = _pipelines.find(pipeline);

    if (it != _pipelines.end()) {
        if (it->second) {
            SDL_ReleaseGPUGraphicsPipeline(_device, it->second);
        }
        _pipelines.erase(it);
    }
}

void RenderingDeviceSDL_GPU::begin_frame() {
    _cmd_buffer = SDL_AcquireGPUCommandBuffer(_device);
    if (!_cmd_buffer) {
        spdlog::error("Failed to acquire command buffer: {}", SDL_GetError());
    }
}

void RenderingDeviceSDL_GPU::end_frame() {
    if (_render_pass) {
        SDL_EndGPURenderPass(_render_pass);
        _render_pass = nullptr;
    }

    if (_cmd_buffer) {
        if (!SDL_SubmitGPUCommandBuffer(_cmd_buffer)) {
            spdlog::error("Failed to submit command buffer: {}", SDL_GetError());
        }
        _cmd_buffer = nullptr;
    }
}


void RenderingDeviceSDL_GPU::render_pass_begin(RID framebuffer, const Viewport& viewport, const Scissor& scissor) {
    if (!_cmd_buffer) {
        return;
    }

    SDL_GPUColorTargetInfo color_targets[8]    = {};
    Uint32 num_color_targets                   = 0;
    SDL_GPUDepthStencilTargetInfo depth_target = {};
    bool has_depth                             = false;

    auto fb_it = _framebuffers.find(framebuffer);
    if (fb_it != _framebuffers.end()) {
        for (const auto& attachment : fb_it->second) {
            auto tex_it = _textures.find(attachment.texture);
            if (tex_it == _textures.end()) {
                continue;
            }

            if (tex_it->second.format.depth_stencil) {
                depth_target.texture       = tex_it->second.handle;
                depth_target.load_op       = attachment.clear ? SDL_GPU_LOADOP_CLEAR : SDL_GPU_LOADOP_LOAD;
                depth_target.store_op      = SDL_GPU_STOREOP_STORE;
                depth_target.clear_depth   = attachment.clear_value.depth;
                depth_target.clear_stencil = (Uint8) attachment.clear_value.stencil;
                depth_target.cycle         = false;
                has_depth                  = true;
            } else {
                if (num_color_targets < 8) {
                    color_targets[num_color_targets].texture              = tex_it->second.handle;
                    color_targets[num_color_targets].mip_level            = attachment.mip_level;
                    color_targets[num_color_targets].layer_or_depth_plane = attachment.layer;
                    color_targets[num_color_targets].load_op              = attachment.clear ? SDL_GPU_LOADOP_CLEAR : SDL_GPU_LOADOP_LOAD;
                    color_targets[num_color_targets].store_op             = SDL_GPU_STOREOP_STORE;
                    color_targets[num_color_targets].clear_color.r        = attachment.clear_value.color.r;
                    color_targets[num_color_targets].clear_color.g        = attachment.clear_value.color.g;
                    color_targets[num_color_targets].clear_color.b        = attachment.clear_value.color.b;
                    color_targets[num_color_targets].clear_color.a        = attachment.clear_value.color.a;
                    color_targets[num_color_targets].cycle                = false;
                    num_color_targets++;
                }
            }
        }
    } else {
        SDL_GPUTexture* swapchain = nullptr;
        Uint32 width = 0, height = 0;
        if (!SDL_AcquireGPUSwapchainTexture(_cmd_buffer, _window, &swapchain, &width, &height)) {
            spdlog::error("Failed to acquire swapchain texture: {}", SDL_GetError());
            return;
        }

        if (!swapchain) {
            return;
        }

        color_targets[0].texture       = swapchain;
        color_targets[0].load_op       = SDL_GPU_LOADOP_CLEAR;
        color_targets[0].store_op      = SDL_GPU_STOREOP_STORE;
        color_targets[0].clear_color.r = 0.1f;
        color_targets[0].clear_color.g = 0.1f;
        color_targets[0].clear_color.b = 0.1f;
        color_targets[0].clear_color.a = 1.0f;
        color_targets[0].cycle         = true;
        num_color_targets              = 1;
    }

    if (num_color_targets == 0 && !has_depth) {
        return;
    }

    _render_pass = SDL_BeginGPURenderPass(_cmd_buffer, color_targets, num_color_targets, has_depth ? &depth_target : nullptr);

    if (!_render_pass) {
        spdlog::error("Failed to begin render pass: {}", SDL_GetError());
        return;
    }

    _current_viewport = viewport;
    set_viewport(viewport);
    set_scissor(scissor);
}

void RenderingDeviceSDL_GPU::render_pass_end() {
    if (_render_pass) {
        SDL_EndGPURenderPass(_render_pass);
        _render_pass = nullptr;
    }
}


void RenderingDeviceSDL_GPU::bind_pipeline(RID pipeline) {
    if (!_render_pass) {
        return;
    }

    auto it = _pipelines.find(pipeline);
    if (it == _pipelines.end()) {
        return;
    }

    SDL_BindGPUGraphicsPipeline(_render_pass, it->second);
    _current_pipeline = pipeline;
}

void RenderingDeviceSDL_GPU::bind_vertex_buffers(const Vector<RID>& buffers, const Vector<size_t>& offsets) {
    if (!_render_pass || buffers.empty()) {
        return;
    }

    Vector<SDL_GPUBufferBinding> bindings;
    bindings.reserve(buffers.size());

    _bound_vertex_buffers.clear();
    _bound_vertex_buffers.reserve(buffers.size());

    for (size_t i = 0; i < buffers.size(); ++i) {
        auto it = _buffers.find(buffers[i]);
        if (it == _buffers.end()) {
            continue;
        }

        SDL_GPUBufferBinding binding = {};
        binding.buffer               = it->second.handle;
        binding.offset               = (i < offsets.size()) ? (Uint32) offsets[i] : 0;
        bindings.push_back(binding);

        BoundVertexBuffer bvb;
        bvb.buffer = it->second.handle;
        bvb.offset = binding.offset;
        _bound_vertex_buffers.push_back(bvb);
    }

    if (!bindings.empty()) {
        SDL_BindGPUVertexBuffers(_render_pass, 0, bindings.data(), (Uint32) bindings.size());
    }
}

void RenderingDeviceSDL_GPU::bind_index_buffer(RID buffer, IndexType type, size_t offset) {
    if (!_render_pass) {
        return;
    }

    auto it = _buffers.find(buffer);
    if (it == _buffers.end()) {
        return;
    }

    SDL_GPUBufferBinding binding = {};
    binding.buffer               = it->second.handle;
    binding.offset               = (Uint32) offset;

    _bound_index_buffer = it->second.handle;
    _bound_index_type   = sdl_index_type(type);
    _bound_index_offset = (Uint32) offset;

    SDL_BindGPUIndexBuffer(_render_pass, &binding, sdl_index_type(type));
}

void RenderingDeviceSDL_GPU::bind_uniform_buffer(uint32_t binding, RID buffer, size_t offset, size_t size) {
    // SDL GPU doesn't have traditional uniform buffer binding like OpenGL/Vulkan
    // Instead it uses push constants or storage buffer binding
    // For now, we'll skip this as uniform data will be handled via push_constant
    // TODO: Implement storage buffer binding if needed
}

void RenderingDeviceSDL_GPU::bind_texture(uint32_t binding, RID texture, RID sampler) {
    if (!_render_pass) {
        return;
    }

    auto tex_it = _textures.find(texture);
    auto sam_it = _samplers.find(sampler);

    if (tex_it == _textures.end() || sam_it == _samplers.end()) {
        return;
    }

    SDL_GPUTextureSamplerBinding tex_sampler_binding = {};
    tex_sampler_binding.texture                      = tex_it->second.handle;
    tex_sampler_binding.sampler                      = sam_it->second;

    BoundTextureSampler bts;
    bts.texture              = tex_it->second.handle;
    bts.sampler              = sam_it->second;
    _bound_textures[binding] = bts;

    SDL_BindGPUFragmentSamplers(_render_pass, binding, &tex_sampler_binding, 1);
}

void RenderingDeviceSDL_GPU::push_constant(const String& name, const void* data, size_t size) {
    if (!_render_pass || !data || size == 0) {
        return;
    }

    SDL_PushGPUVertexUniformData(_cmd_buffer, 0, data, (Uint32) size);
    SDL_PushGPUFragmentUniformData(_cmd_buffer, 0, data, (Uint32) size);
}

void RenderingDeviceSDL_GPU::draw(uint32_t vertex_count, uint32_t instance_count, uint32_t first_vertex, uint32_t first_instance) {
    if (!_render_pass) {
        return;
    }

    SDL_DrawGPUPrimitives(_render_pass, vertex_count, instance_count, first_vertex, first_instance);
}

void RenderingDeviceSDL_GPU::draw_indexed(uint32_t index_count, uint32_t instance_count, uint32_t first_index, int32_t vertex_offset,
                                          uint32_t first_instance) {
    if (!_render_pass) {
        return;
    }

    SDL_DrawGPUIndexedPrimitives(_render_pass, index_count, instance_count, first_index, vertex_offset, first_instance);
}

void RenderingDeviceSDL_GPU::set_viewport(const Viewport& viewport) {
    if (!_render_pass) {
        return;
    }

    SDL_GPUViewport sdl_viewport = {};
    sdl_viewport.x               = viewport.x;
    sdl_viewport.y               = viewport.y;
    sdl_viewport.w               = viewport.width;
    sdl_viewport.h               = viewport.height;
    sdl_viewport.min_depth       = viewport.min_depth;
    sdl_viewport.max_depth       = viewport.max_depth;

    SDL_SetGPUViewport(_render_pass, &sdl_viewport);
    _current_viewport = viewport;
}

void RenderingDeviceSDL_GPU::set_scissor(const Scissor& scissor) {
    if (!_render_pass) {
        return;
    }

    SDL_Rect sdl_scissor = {};
    sdl_scissor.x        = scissor.x;
    sdl_scissor.y        = scissor.y;
    sdl_scissor.w        = (int) scissor.width;
    sdl_scissor.h        = (int) scissor.height;

    SDL_SetGPUScissor(_render_pass, &sdl_scissor);
}

void RenderingDeviceSDL_GPU::clear_color(const glm::vec4& color) {
    // In SDL GPU, clearing is handled at render pass begin via load_op = SDL_GPU_LOADOP_CLEAR
    // This function is kept for API compatibility but the actual clear happens in render_pass_begin
    // If we need to clear mid-pass, we would need to end and restart the render pass
}
void RenderingDeviceSDL_GPU::clear_depth_stencil(float depth, uint32_t stencil) {
    // In SDL GPU, clearing is handled at render pass begin via load_op = SDL_GPU_LOADOP_CLEAR
    // This function is kept for API compatibility but the actual clear happens in render_pass_begin
}

void RenderingDeviceSDL_GPU::swap_buffers() {
    // In SDL GPU, swapchain presentation is handled automatically when the command buffer
    // containing the swapchain texture is submitted via SDL_SubmitGPUCommandBuffer in end_frame()
    // No explicit swap is needed
}

SDL_GPUSamplerMipmapMode RenderingDeviceSDL_GPU::sdl_mipmap_mode(TextureFilter filter) {
    switch (filter) {
    case TextureFilter::NEAREST_MIPMAP_NEAREST:
    case TextureFilter::LINEAR_MIPMAP_NEAREST:
        return SDL_GPU_SAMPLERMIPMAPMODE_NEAREST;
    case TextureFilter::NEAREST_MIPMAP_LINEAR:
    case TextureFilter::LINEAR_MIPMAP_LINEAR:
        return SDL_GPU_SAMPLERMIPMAPMODE_LINEAR;
    default:
        return SDL_GPU_SAMPLERMIPMAPMODE_LINEAR;
    }
}


SDL_GPUTextureFormat RenderingDeviceSDL_GPU::sdl_format(DataFormat fmt) {
    switch (fmt) {
    case DataFormat::R8_UNORM:
        return SDL_GPU_TEXTUREFORMAT_R8_UNORM;
    case DataFormat::R8G8_UNORM:
        return SDL_GPU_TEXTUREFORMAT_R8G8_UNORM;
    case DataFormat::R8G8B8A8_UNORM:
        return SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM;
    case DataFormat::D24_UNORM_S8_UINT:
        return SDL_GPU_TEXTUREFORMAT_D24_UNORM_S8_UINT;
    default:
        return SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM;
    }
}

SDL_GPUVertexElementFormat RenderingDeviceSDL_GPU::sdl_vertex_format(DataFormat fmt) {
    switch (fmt) {
    case DataFormat::R32_SFLOAT:
        return SDL_GPU_VERTEXELEMENTFORMAT_FLOAT;
    case DataFormat::R32G32_SFLOAT:
        return SDL_GPU_VERTEXELEMENTFORMAT_FLOAT2;
    case DataFormat::R32G32B32_SFLOAT:
        return SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3;
    case DataFormat::R32G32B32A32_SFLOAT:
        return SDL_GPU_VERTEXELEMENTFORMAT_FLOAT4;
    case DataFormat::R32_UINT:
        return SDL_GPU_VERTEXELEMENTFORMAT_UINT;
    case DataFormat::R32G32_UINT:
        return SDL_GPU_VERTEXELEMENTFORMAT_UINT2;
    case DataFormat::R32G32B32_UINT:
        return SDL_GPU_VERTEXELEMENTFORMAT_UINT3;
    case DataFormat::R32G32B32A32_UINT:
        return SDL_GPU_VERTEXELEMENTFORMAT_UINT4;
    default:
        return SDL_GPU_VERTEXELEMENTFORMAT_FLOAT;
    }
}

SDL_GPUFilter RenderingDeviceSDL_GPU::sdl_filter(TextureFilter filter) {
    return (filter == TextureFilter::NEAREST) ? SDL_GPU_FILTER_NEAREST : SDL_GPU_FILTER_LINEAR;
}

SDL_GPUSamplerAddressMode RenderingDeviceSDL_GPU::sdl_wrap(TextureWrap wrap) {
    switch (wrap) {
    case TextureWrap::REPEAT:
        return SDL_GPU_SAMPLERADDRESSMODE_REPEAT;
    case TextureWrap::MIRRORED_REPEAT:
        return SDL_GPU_SAMPLERADDRESSMODE_MIRRORED_REPEAT;
    case TextureWrap::CLAMP_TO_EDGE:
        return SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE;
    default:
        return SDL_GPU_SAMPLERADDRESSMODE_REPEAT;
    }
}

SDL_GPUCompareOp RenderingDeviceSDL_GPU::sdl_compare(CompareOp op) {
    switch (op) {
    case CompareOp::NEVER:
        return SDL_GPU_COMPAREOP_NEVER;
    case CompareOp::LESS:
        return SDL_GPU_COMPAREOP_LESS;
    case CompareOp::EQUAL:
        return SDL_GPU_COMPAREOP_EQUAL;
    case CompareOp::LESS_OR_EQUAL:
        return SDL_GPU_COMPAREOP_LESS_OR_EQUAL;
    case CompareOp::GREATER:
        return SDL_GPU_COMPAREOP_GREATER;
    case CompareOp::NOT_EQUAL:
        return SDL_GPU_COMPAREOP_NOT_EQUAL;
    case CompareOp::GREATER_OR_EQUAL:
        return SDL_GPU_COMPAREOP_GREATER_OR_EQUAL;
    case CompareOp::ALWAYS:
        return SDL_GPU_COMPAREOP_ALWAYS;
    default:
        return SDL_GPU_COMPAREOP_LESS;
    }
}

SDL_GPUStencilOp RenderingDeviceSDL_GPU::sdl_stencil(StencilOp op) {
    switch (op) {
    case StencilOp::KEEP:
        return SDL_GPU_STENCILOP_KEEP;
    case StencilOp::ZERO:
        return SDL_GPU_STENCILOP_ZERO;
    case StencilOp::REPLACE:
        return SDL_GPU_STENCILOP_REPLACE;
    case StencilOp::INCREMENT_AND_CLAMP:
        return SDL_GPU_STENCILOP_INCREMENT_AND_CLAMP;
    case StencilOp::DECREMENT_AND_CLAMP:
        return SDL_GPU_STENCILOP_DECREMENT_AND_CLAMP;
    case StencilOp::INVERT:
        return SDL_GPU_STENCILOP_INVERT;
    case StencilOp::INCREMENT_AND_WRAP:
        return SDL_GPU_STENCILOP_INCREMENT_AND_WRAP;
    case StencilOp::DECREMENT_AND_WRAP:
        return SDL_GPU_STENCILOP_DECREMENT_AND_WRAP;
    default:
        return SDL_GPU_STENCILOP_KEEP;
    }
}

SDL_GPUBlendFactor RenderingDeviceSDL_GPU::sdl_blend_factor(BlendFactor f) {
    switch (f) {
    case BlendFactor::ZERO:
        return SDL_GPU_BLENDFACTOR_ZERO;
    case BlendFactor::ONE:
        return SDL_GPU_BLENDFACTOR_ONE;
    case BlendFactor::SRC_COLOR:
        return SDL_GPU_BLENDFACTOR_SRC_COLOR;
    case BlendFactor::ONE_MINUS_SRC_COLOR:
        return SDL_GPU_BLENDFACTOR_ONE_MINUS_SRC_COLOR;
    case BlendFactor::DST_COLOR:
        return SDL_GPU_BLENDFACTOR_DST_COLOR;
    case BlendFactor::ONE_MINUS_DST_COLOR:
        return SDL_GPU_BLENDFACTOR_ONE_MINUS_DST_COLOR;
    case BlendFactor::SRC_ALPHA:
        return SDL_GPU_BLENDFACTOR_SRC_ALPHA;
    case BlendFactor::ONE_MINUS_SRC_ALPHA:
        return SDL_GPU_BLENDFACTOR_ONE_MINUS_SRC_ALPHA;
    case BlendFactor::DST_ALPHA:
        return SDL_GPU_BLENDFACTOR_DST_ALPHA;
    case BlendFactor::ONE_MINUS_DST_ALPHA:
        return SDL_GPU_BLENDFACTOR_ONE_MINUS_DST_ALPHA;
    default:
        return SDL_GPU_BLENDFACTOR_ONE;
    }
}

SDL_GPUBlendOp RenderingDeviceSDL_GPU::sdl_blend_op(BlendOp op) {
    switch (op) {
    case BlendOp::ADD:
        return SDL_GPU_BLENDOP_ADD;
    case BlendOp::SUBTRACT:
        return SDL_GPU_BLENDOP_SUBTRACT;
    case BlendOp::REVERSE_SUBTRACT:
        return SDL_GPU_BLENDOP_REVERSE_SUBTRACT;
    case BlendOp::MIN:
        return SDL_GPU_BLENDOP_MIN;
    case BlendOp::MAX:
        return SDL_GPU_BLENDOP_MAX;
    default:
        return SDL_GPU_BLENDOP_ADD;
    }
}

SDL_GPUCullMode RenderingDeviceSDL_GPU::sdl_cull(CullMode mode) {
    switch (mode) {
    case CullMode::NONE:
        return SDL_GPU_CULLMODE_NONE;
    case CullMode::FRONT:
        return SDL_GPU_CULLMODE_FRONT;
    case CullMode::BACK:
        return SDL_GPU_CULLMODE_BACK;
    default:
        return SDL_GPU_CULLMODE_BACK;
    }
}

SDL_GPUFillMode RenderingDeviceSDL_GPU::sdl_fill(PolygonMode mode) {
    return (mode == PolygonMode::LINE) ? SDL_GPU_FILLMODE_LINE : SDL_GPU_FILLMODE_FILL;
}

SDL_GPUPrimitiveType RenderingDeviceSDL_GPU::sdl_topology(PrimitiveTopology topo) {
    switch (topo) {
    case PrimitiveTopology::POINTS:
        return SDL_GPU_PRIMITIVETYPE_POINTLIST;
    case PrimitiveTopology::LINES:
        return SDL_GPU_PRIMITIVETYPE_LINELIST;
    case PrimitiveTopology::LINE_STRIP:
        return SDL_GPU_PRIMITIVETYPE_LINESTRIP;
    case PrimitiveTopology::TRIANGLES:
        return SDL_GPU_PRIMITIVETYPE_TRIANGLELIST;
    case PrimitiveTopology::TRIANGLE_STRIP:
        return SDL_GPU_PRIMITIVETYPE_TRIANGLESTRIP;
    default:
        return SDL_GPU_PRIMITIVETYPE_TRIANGLELIST;
    }
}

SDL_GPUIndexElementSize RenderingDeviceSDL_GPU::sdl_index_type(IndexType type) {
    return (type == IndexType::UINT16) ? SDL_GPU_INDEXELEMENTSIZE_16BIT : SDL_GPU_INDEXELEMENTSIZE_32BIT;
}

SDL_GPUBufferUsageFlags RenderingDeviceSDL_GPU::sdl_buffer_usage(uint32_t flags) {
    SDL_GPUBufferUsageFlags result = 0;
    if (flags & (uint32_t) BufferUsage::BUFFER_USAGE_VERTEX) {
        result |= SDL_GPU_BUFFERUSAGE_VERTEX;
    }
    if (flags & (uint32_t) BufferUsage::BUFFER_USAGE_INDEX) {
        result |= SDL_GPU_BUFFERUSAGE_INDEX;
    }
    if (flags & (uint32_t) BufferUsage::BUFFER_USAGE_UNIFORM) {
        result |= SDL_GPU_BUFFERUSAGE_GRAPHICS_STORAGE_READ;
    }
    return result;
}
