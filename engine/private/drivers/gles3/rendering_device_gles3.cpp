#include "drivers/gles3/rendering_device_gles3.h"

#include "spdlog/spdlog.h"


RenderingDeviceGLES3::~RenderingDeviceGLES3() {
    shutdown();
}

bool RenderingDeviceGLES3::initialize(SDL_Window* sdl_window) {


#if defined(SDL_PLATFORM_ANDROID) || defined(SDL_PLATFORM_IOS) || defined(SDL_PLATFORM_EMSCRIPTEN)
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);

#else
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

    #if defined(SDL_PLATFORM_MACOS)
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG); // OSX compatibility Bit
    #endif


#endif

    gl_context = SDL_GL_CreateContext(sdl_window);

    if (!gl_context) {
        spdlog::critical("Failed to create OpenGL/ES context: {}", SDL_GetError());
        return false;
    }


#if defined(SDL_PLATFORM_ANDROID) && !defined(SDL_PLATFORM_IOS) && !defined(SDL_PLATFORM_EMSCRIPTEN)
    if (!gladLoadGLES2Loader(reinterpret_cast<GLADloadproc>(SDL_GL_GetProcAddress))) {
        spdlog::error("Failed to initialize OpenGL Loader (GLAD)");
        return false;
    }
#else
    if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(SDL_GL_GetProcAddress))) {
        spdlog::error("Failed to initialize OpenGLES Loader (GLAD)");
        return false;
    }
#endif

    SDL_GL_SetSwapInterval(1); // TODO: make configurable (vsync on/off)

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    _window = sdl_window;

    int major,minor;
    SDL_GL_GetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, &major);
    SDL_GL_GetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, &minor);
    spdlog::info("Initialized OpenGL/ES context version {}.{}", major, minor);

    const char* vendor   = (const char*) glGetString(GL_VENDOR);
    const char* renderer = (const char*) glGetString(GL_RENDERER);

    spdlog::info("GPU Vendor: {}", vendor);
    spdlog::info("GPU Renderer: {}", renderer);

    return true;
}

void RenderingDeviceGLES3::shutdown() {
    for (auto& [id, shader] : shaders) {
        glDeleteProgram((GLuint) shader.program);
    }

    for (auto& [id, buffer] : buffers) {
        GLuint buf = (GLuint) buffer.handle;
        glDeleteBuffers(1, &buf);
    }

    for (auto& [id, texture] : textures) {
        GLuint tex = (GLuint) texture.handle;
        glDeleteTextures(1, &tex);
    }

    for (auto& [id, sampler] : samplers) {
        GLuint samp = (GLuint) sampler.handle;
        glDeleteSamplers(1, &samp);
    }

    for (auto& [id, fb] : framebuffers) {
        GLuint fbo = (GLuint) fb.handle;
        glDeleteFramebuffers(1, &fbo);
    }

    for (auto& [id, pipeline] : pipelines) {
        GLuint vao = (GLuint) pipeline.handle;
        glDeleteVertexArrays(1, &vao);
    }

    SDL_GL_DestroyContext(gl_context);
}

GLuint RenderingDeviceGLES3::compile_shader(GLenum type, const String& source) {
    GLuint shader   = glCreateShader(type);
    const char* src = source.c_str();
    glShaderSource(shader, 1, &src, nullptr);
    glCompileShader(shader);

    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);

    if (!success) {
        char log[512];
        glGetShaderInfoLog(shader, 512, nullptr, log);
        spdlog::error("Shader compilation error: {}", log);
        glDeleteShader(shader);
        return 0;
    }

    return shader;
}

RID RenderingDeviceGLES3::shader_create_from_source(const String& vertex_src, const String& fragment_src) {
    GLuint vs = compile_shader(GL_VERTEX_SHADER, vertex_src);
    GLuint fs = compile_shader(GL_FRAGMENT_SHADER, fragment_src);

    if (!vs || !fs) {
        return INVALID_RID;
    }

    GLuint program = glCreateProgram();
    glAttachShader(program, vs);
    glAttachShader(program, fs);
    glLinkProgram(program);

    GLint success;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        char log[512];
        glGetProgramInfoLog(program, 512, nullptr, log);
        spdlog::error("Shader linking error: {}", log);
        glDeleteProgram(program);
        glDeleteShader(vs);
        glDeleteShader(fs);
        return INVALID_RID;
    }

    glDeleteShader(vs);
    glDeleteShader(fs);

    RID rid      = allocate_rid();
    shaders[rid] = ShaderModule{program};
    return rid;
}

void RenderingDeviceGLES3::shader_destroy(RID shader) {
    auto it = shaders.find(shader);
    if (it != shaders.end()) {
        glDeleteProgram((GLuint) it->second.program);
        shaders.erase(it);
    }
}

RID RenderingDeviceGLES3::buffer_create(size_t size, uint32_t usage_flags, const void* data) {
    GLuint buffer;
    glGenBuffers(1, &buffer);

    GLenum target = (usage_flags & (uint32_t) BufferUsage::BUFFER_USAGE_UNIFORM) ? GL_UNIFORM_BUFFER : GL_ARRAY_BUFFER;
    if (usage_flags & (uint32_t) BufferUsage::BUFFER_USAGE_INDEX) {
        target = GL_ELEMENT_ARRAY_BUFFER;
    }

    glBindBuffer(target, buffer);
    glBufferData(target, size, data, GL_DYNAMIC_DRAW);
    glBindBuffer(target, 0);

    RID rid      = allocate_rid();
    buffers[rid] = Buffer{buffer, size, usage_flags, (uint32_t) target};
    return rid;
}

void RenderingDeviceGLES3::buffer_update(RID buffer, size_t offset, size_t size, const void* data) {
    auto it = buffers.find(buffer);
    if (it != buffers.end()) {
        glBindBuffer((GLenum) it->second.target, (GLuint) it->second.handle);
        glBufferSubData((GLenum) it->second.target, offset, size, data);
        glBindBuffer((GLenum) it->second.target, 0);
    }
}

void RenderingDeviceGLES3::buffer_destroy(RID buffer) {
    auto it = buffers.find(buffer);
    if (it != buffers.end()) {
        GLuint buf = (GLuint) it->second.handle;
        glDeleteBuffers(1, &buf);
        buffers.erase(it);
    }
}

GLenum RenderingDeviceGLES3::to_gl_format(DataFormat format, bool* is_int) {
    if (is_int) {
        *is_int = false;
    }

    switch (format) {
    case DataFormat::R8_UNORM:
        return GL_R8;
    case DataFormat::R8G8_UNORM:
        return GL_RG8;
    case DataFormat::R8G8B8_UNORM:
        return GL_RGB8;
    case DataFormat::R8G8B8A8_UNORM:
        return GL_RGBA8;
    case DataFormat::R32_SFLOAT:
        return GL_R32F;
    case DataFormat::R32G32_SFLOAT:
        return GL_RG32F;
    case DataFormat::R32G32B32_SFLOAT:
        return GL_RGB32F;
    case DataFormat::R32G32B32A32_SFLOAT:
        return GL_RGBA32F;
    case DataFormat::D24_UNORM_S8_UINT:
        return GL_DEPTH24_STENCIL8;
    case DataFormat::D32_SFLOAT:
        return GL_DEPTH_COMPONENT32F;
    default:
        return GL_RGBA8;
    }
}

RID RenderingDeviceGLES3::texture_create(const TextureFormat& format, void* data) {
    GLuint texture;
    glGenTextures(1, &texture);

    GLenum target = GL_TEXTURE_2D;
    if (format.type == TextureType::TEXTURE_TYPE_CUBEMAP) {
        target = GL_TEXTURE_CUBE_MAP;
    } else if (format.type == TextureType::TEXTURE_TYPE_2D_ARRAY) {
        target = GL_TEXTURE_2D_ARRAY;
    } else if (format.type == TextureType::TEXTURE_TYPE_3D) {
        target = GL_TEXTURE_3D;
    }

    glBindTexture(target, texture);

    GLenum internal_format = to_gl_format(format.format);
    GLenum pixel_format    = GL_RGBA;
    GLenum pixel_type      = GL_UNSIGNED_BYTE;

    if (format.format == DataFormat::R8G8B8_UNORM) {
        internal_format = GL_RGB8;
        pixel_format    = GL_RGB;
    } else if (format.format == DataFormat::R8G8_UNORM) {
        internal_format = GL_RG8;
        pixel_format    = GL_RG;
    } else if (format.format == DataFormat::R8_UNORM) {
        internal_format = GL_R8;
        pixel_format    = GL_RED;
    } else if (format.format == DataFormat::R8G8B8A8_UNORM) {
        internal_format = GL_RGBA8;
        pixel_format    = GL_RGBA;
    }

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    if (format.type == TextureType::TEXTURE_TYPE_2D) {
        if (data == nullptr) {
            printf("ERROR: texture_create called with null data!\n");
            glTexImage2D(GL_TEXTURE_2D, 0, internal_format, format.width, format.height, 0, pixel_format, pixel_type, nullptr);
        } else {
            glTexImage2D(GL_TEXTURE_2D, 0, internal_format, format.width, format.height, 0, pixel_format, pixel_type, data);
        }
    }


    glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(target, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(target, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glBindTexture(target, 0);

    RID rid       = allocate_rid();
    textures[rid] = Texture{rid, texture, format};

    return rid;
}

void RenderingDeviceGLES3::texture_destroy(RID texture) {
    auto it = textures.find(texture);
    if (it != textures.end()) {
        GLuint tex = (GLuint) it->second.handle;
        glDeleteTextures(1, &tex);
        textures.erase(it);
    }
}

void RenderingDeviceGLES3::get_texture_size(RID texture, uint32_t& width, uint32_t& height) {
    auto it = textures.find(texture);
    if (it != textures.end()) {
        width  = it->second.format.width;
        height = it->second.format.height;
    } else {
        width  = 0;
        height = 0;
    }
}

uint32_t RenderingDeviceGLES3::texture_get_native_handle(RID texture) {
    auto it = textures.find(texture);
    if (it != textures.end()) {
        return it->second.handle;
    }
    return 0;
}

Texture RenderingDeviceGLES3::get_texture(RID texture) {
    auto it = textures.find(texture);
    if (it != textures.end()) {
        return it->second;
    }
    return Texture{}; // Returns empty texture
}

RID RenderingDeviceGLES3::pipeline_create(const PipelineState& state) {
    GLuint vao;
    glGenVertexArrays(1, &vao);

    RID rid        = allocate_rid();
    pipelines[rid] = Pipeline{state, vao};
    return rid;
}

void RenderingDeviceGLES3::pipeline_destroy(RID pipeline) {
    auto it = pipelines.find(pipeline);
    if (it != pipelines.end()) {
        GLuint vao = (GLuint) it->second.handle;
        glDeleteVertexArrays(1, &vao);
        pipelines.erase(it);
    }
}

void RenderingDeviceGLES3::bind_pipeline(RID pipeline) {
    auto it = pipelines.find(pipeline);
    if (it == pipelines.end()) {
        return;
    }

    current_pipeline = pipeline;
    const auto& pipe = it->second;

    glBindVertexArray((GLuint) pipe.handle);

    auto shader_it = shaders.find(pipe.state.shader);
    if (shader_it != shaders.end()) {
        glUseProgram(shader_it->second.program);
    }

    apply_rasterization_state(pipe.state.rasterization);
    apply_depth_stencil_state(pipe.state.depth_stencil);
    apply_blend_state(pipe.state.blend_states);
}

void RenderingDeviceGLES3::apply_rasterization_state(const RasterizationState& state) {
    if (state.cull_mode == CullMode::NONE) {
        glDisable(GL_CULL_FACE);
    } else {
        glEnable(GL_CULL_FACE);
        GLenum mode = (state.cull_mode == CullMode::FRONT) ? GL_FRONT : (state.cull_mode == CullMode::BACK) ? GL_BACK : GL_FRONT_AND_BACK;
        glCullFace(mode);
    }
    glFrontFace(state.front_face_ccw ? GL_CCW : GL_CW);
}

void RenderingDeviceGLES3::apply_depth_stencil_state(const DepthStencilState& state) {
    if (state.depth_test_enable) {
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(to_gl_compare(state.depth_compare_op));
        glDepthMask(state.depth_write_enable ? GL_TRUE : GL_FALSE);
    } else {
        glDisable(GL_DEPTH_TEST);
    }
}

void RenderingDeviceGLES3::apply_blend_state(const Vector<BlendState>& states) {

    if (states.empty() || !states[0].enable) {
        glDisable(GL_BLEND);
    } else {
        glEnable(GL_BLEND);
        const auto& blend = states[0];
        glBlendFuncSeparate(to_gl_blend_factor(blend.src_color), to_gl_blend_factor(blend.dst_color), to_gl_blend_factor(blend.src_alpha),
                            to_gl_blend_factor(blend.dst_alpha));
        glBlendEquationSeparate(to_gl_blend_op(blend.color_op), to_gl_blend_op(blend.alpha_op));
    }
}

GLenum RenderingDeviceGLES3::to_gl_compare(CompareOp op) {
    switch (op) {
    case CompareOp::NEVER:
        return GL_NEVER;
    case CompareOp::LESS:
        return GL_LESS;
    case CompareOp::EQUAL:
        return GL_EQUAL;
    case CompareOp::LESS_OR_EQUAL:
        return GL_LEQUAL;
    case CompareOp::GREATER:
        return GL_GREATER;
    case CompareOp::NOT_EQUAL:
        return GL_NOTEQUAL;
    case CompareOp::GREATER_OR_EQUAL:
        return GL_GEQUAL;
    case CompareOp::ALWAYS:
        return GL_ALWAYS;
    default:
        return GL_LESS;
    }
}

GLenum RenderingDeviceGLES3::to_gl_blend_factor(BlendFactor factor) {
    switch (factor) {
    case BlendFactor::ZERO:
        return GL_ZERO;
    case BlendFactor::ONE:
        return GL_ONE;
    case BlendFactor::SRC_COLOR:
        return GL_SRC_COLOR;
    case BlendFactor::ONE_MINUS_SRC_COLOR:
        return GL_ONE_MINUS_SRC_COLOR;
    case BlendFactor::DST_COLOR:
        return GL_DST_COLOR;
    case BlendFactor::ONE_MINUS_DST_COLOR:
        return GL_ONE_MINUS_DST_COLOR;
    case BlendFactor::SRC_ALPHA:
        return GL_SRC_ALPHA;
    case BlendFactor::ONE_MINUS_SRC_ALPHA:
        return GL_ONE_MINUS_SRC_ALPHA;
    case BlendFactor::DST_ALPHA:
        return GL_DST_ALPHA;
    case BlendFactor::ONE_MINUS_DST_ALPHA:
        return GL_ONE_MINUS_DST_ALPHA;
    default:
        return GL_ONE;
    }
}

GLenum RenderingDeviceGLES3::to_gl_blend_op(BlendOp op) {
    switch (op) {
    case BlendOp::ADD:
        return GL_FUNC_ADD;
    case BlendOp::SUBTRACT:
        return GL_FUNC_SUBTRACT;
    case BlendOp::REVERSE_SUBTRACT:
        return GL_FUNC_REVERSE_SUBTRACT;
    default:
        return GL_FUNC_ADD;
    }
}

void RenderingDeviceGLES3::bind_vertex_buffers(const Vector<RID>& buffer_rids, const Vector<size_t>& offsets) {
    if (current_pipeline == INVALID_RID) {
        return;
    }

    auto pipe_it = pipelines.find(current_pipeline);
    if (pipe_it == pipelines.end()) {
        return;
    }

    const auto& vertex_format = pipe_it->second.state.vertex_format;

    for (size_t i = 0; i < buffer_rids.size(); ++i) {
        auto buf_it = buffers.find(buffer_rids[i]);
        if (buf_it == buffers.end()) {
            continue;
        }

        glBindBuffer(GL_ARRAY_BUFFER, (GLuint) buf_it->second.handle);

        for (const auto& attr : vertex_format.attributes) {
            glEnableVertexAttribArray(attr.location);

            GLint size           = 4;
            GLenum type          = GL_FLOAT;
            GLboolean normalized = GL_FALSE;

            switch (attr.format) {
            case DataFormat::R8_UNORM:
                size       = 1;
                type       = GL_UNSIGNED_BYTE;
                normalized = GL_TRUE;
                break;
            case DataFormat::R8G8_UNORM:
                size       = 2;
                type       = GL_UNSIGNED_BYTE;
                normalized = GL_TRUE;
                break;
            case DataFormat::R8G8B8_UNORM:
                size       = 3;
                type       = GL_UNSIGNED_BYTE;
                normalized = GL_TRUE;
                break;
            case DataFormat::R8G8B8A8_UNORM:
                size       = 4;
                type       = GL_UNSIGNED_BYTE;
                normalized = GL_TRUE;
                break;
            case DataFormat::R16_SFLOAT:
                size       = 1;
                type       = GL_HALF_FLOAT;
                normalized = GL_FALSE;
                break;
            case DataFormat::R16G16_SFLOAT:
                size       = 2;
                type       = GL_HALF_FLOAT;
                normalized = GL_FALSE;
                break;
            case DataFormat::R16G16B16A16_SFLOAT:
                size       = 4;
                type       = GL_HALF_FLOAT;
                normalized = GL_FALSE;
                break;
            case DataFormat::R32_SFLOAT:
                size       = 1;
                type       = GL_FLOAT;
                normalized = GL_FALSE;
                break;
            case DataFormat::R32G32_SFLOAT:
                size       = 2;
                type       = GL_FLOAT;
                normalized = GL_FALSE;
                break;
            case DataFormat::R32G32B32_SFLOAT:
                size       = 3;
                type       = GL_FLOAT;
                normalized = GL_FALSE;
                break;
            case DataFormat::R32G32B32A32_SFLOAT:
                size       = 4;
                type       = GL_FLOAT;
                normalized = GL_FALSE;
                break;
            default:
                size       = 4;
                type       = GL_FLOAT;
                normalized = GL_FALSE;
                break;
            }

            size_t offset = attr.offset + (i < offsets.size() ? offsets[i] : 0);
            glVertexAttribPointer(attr.location, size, type, normalized, vertex_format.stride, (void*) offset);
        }
    }
}

void RenderingDeviceGLES3::bind_index_buffer(RID buffer, IndexType type, size_t offset) {
    auto it = buffers.find(buffer);
    if (it != buffers.end()) {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, (GLuint) it->second.handle);
        current_index_type = type;
    }
}

void RenderingDeviceGLES3::draw(uint32_t vertex_count, uint32_t instance_count, uint32_t first_vertex, uint32_t first_instance) {
    if (current_pipeline == INVALID_RID) {
        return;
    }
    auto it = pipelines.find(current_pipeline);
    if (it == pipelines.end()) {
        return;
    }

    GLenum mode = to_gl_topology(it->second.state.topology);
    glDrawArrays(mode, first_vertex, vertex_count);
}

void RenderingDeviceGLES3::draw_indexed(uint32_t index_count, uint32_t instance_count, uint32_t first_index, int32_t vertex_offset,
                                        uint32_t first_instance) {
    if (current_pipeline == INVALID_RID) {
        return;
    }
    auto it = pipelines.find(current_pipeline);
    if (it == pipelines.end()) {
        return;
    }

    GLenum mode   = to_gl_topology(it->second.state.topology);
    GLenum type   = (current_index_type == IndexType::UINT16) ? GL_UNSIGNED_SHORT : GL_UNSIGNED_INT;
    size_t offset = first_index * (type == GL_UNSIGNED_SHORT ? 2 : 4);
    glDrawElements(mode, index_count, type, (void*) offset);
}

GLenum RenderingDeviceGLES3::to_gl_topology(PrimitiveTopology topology) {
    switch (topology) {
    case PrimitiveTopology::POINTS:
        return GL_POINTS;
    case PrimitiveTopology::LINES:
        return GL_LINES;
    case PrimitiveTopology::LINE_STRIP:
        return GL_LINE_STRIP;
    case PrimitiveTopology::TRIANGLES:
        return GL_TRIANGLES;
    case PrimitiveTopology::TRIANGLE_STRIP:
        return GL_TRIANGLE_STRIP;
    case PrimitiveTopology::TRIANGLE_FAN:
        return GL_TRIANGLE_FAN;
    default:
        return GL_TRIANGLES;
    }
}

void RenderingDeviceGLES3::bind_uniform_buffer(uint32_t binding, RID buffer, size_t offset, size_t size) {
    auto it = buffers.find(buffer);
    if (it != buffers.end()) {
        if (size == 0) {
            size = it->second.size - offset;
        }
        glBindBufferRange(GL_UNIFORM_BUFFER, binding, (GLuint) it->second.handle, offset, size);
    }
}

void RenderingDeviceGLES3::bind_texture(uint32_t binding, RID texture, RID sampler) {
    auto tex_it = textures.find(texture);
    if (tex_it != textures.end()) {
        glActiveTexture(GL_TEXTURE0 + binding);
        GLenum target = GL_TEXTURE_2D;
        if (tex_it->second.format.type == TextureType::TEXTURE_TYPE_CUBEMAP) {
            target = GL_TEXTURE_CUBE_MAP;
        }

        glBindTexture(target, (GLuint) tex_it->second.handle);

        if (sampler != INVALID_RID) {
            auto samp_it = samplers.find(sampler);
            if (samp_it != samplers.end()) {
                glBindSampler(binding, (GLuint) samp_it->second.handle);
            }
        }
    }
}

RID RenderingDeviceGLES3::sampler_create(const SamplerState& state) {
    GLuint sampler;
    glGenSamplers(1, &sampler);

    glSamplerParameteri(sampler, GL_TEXTURE_MIN_FILTER, to_gl_filter(state.min_filter));
    glSamplerParameteri(sampler, GL_TEXTURE_MAG_FILTER, to_gl_filter(state.mag_filter));
    glSamplerParameteri(sampler, GL_TEXTURE_WRAP_S, to_gl_wrap(state.wrap_u));
    glSamplerParameteri(sampler, GL_TEXTURE_WRAP_T, to_gl_wrap(state.wrap_v));
    glSamplerParameteri(sampler, GL_TEXTURE_WRAP_R, to_gl_wrap(state.wrap_w));

    if (state.compare_enabled) {
        glSamplerParameteri(sampler, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
        glSamplerParameteri(sampler, GL_TEXTURE_COMPARE_FUNC, to_gl_compare(state.compare_op));
    }

    RID rid       = allocate_rid();
    samplers[rid] = Sampler{sampler, state};
    return rid;
}

void RenderingDeviceGLES3::sampler_destroy(RID sampler) {
    auto it = samplers.find(sampler);
    if (it != samplers.end()) {
        GLuint samp = (GLuint) it->second.handle;
        glDeleteSamplers(1, &samp);
        samplers.erase(it);
    }
}

GLenum RenderingDeviceGLES3::to_gl_filter(TextureFilter filter) {
    switch (filter) {
    case TextureFilter::NEAREST:
        return GL_NEAREST;
    case TextureFilter::LINEAR:
        return GL_LINEAR;
    case TextureFilter::NEAREST_MIPMAP_NEAREST:
        return GL_NEAREST_MIPMAP_NEAREST;
    case TextureFilter::LINEAR_MIPMAP_NEAREST:
        return GL_LINEAR_MIPMAP_NEAREST;
    case TextureFilter::NEAREST_MIPMAP_LINEAR:
        return GL_NEAREST_MIPMAP_LINEAR;
    case TextureFilter::LINEAR_MIPMAP_LINEAR:
        return GL_LINEAR_MIPMAP_LINEAR;
    default:
        return GL_LINEAR;
    }
}

GLenum RenderingDeviceGLES3::to_gl_wrap(TextureWrap wrap) {
    switch (wrap) {
    case TextureWrap::REPEAT:
        return GL_REPEAT;
    case TextureWrap::MIRRORED_REPEAT:
        return GL_MIRRORED_REPEAT;
    case TextureWrap::CLAMP_TO_EDGE:
        return GL_CLAMP_TO_EDGE;
    default:
        return GL_REPEAT;
    }
}

RID RenderingDeviceGLES3::framebuffer_create(const Vector<RenderPassAttachment>& attachments) {
    GLuint fbo;
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    uint32_t width = 0, height = 0;
    uint32_t color_attachment_idx = 0;

    for (const auto& attachment : attachments) {
        auto tex_it = textures.find(attachment.texture);
        if (tex_it == textures.end()) {
            continue;
        }

        const auto& tex = tex_it->second;

        if (width == 0) {
            width  = tex.format.width;
            height = tex.format.height;
        }

        GLenum attachment_point;
        if (tex.format.depth_stencil) {
            attachment_point = GL_DEPTH_STENCIL_ATTACHMENT;
        } else {
            attachment_point = GL_COLOR_ATTACHMENT0 + color_attachment_idx++;
        }

        glFramebufferTexture2D(GL_FRAMEBUFFER, attachment_point, GL_TEXTURE_2D, (GLuint) tex.handle, attachment.mip_level);
    }

    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);

    if (status != GL_FRAMEBUFFER_COMPLETE) {
        spdlog::error("Framebuffer incomplete: 0x{:x}", status);
        glDeleteFramebuffers(1, &fbo);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        return INVALID_RID;
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    RID rid           = allocate_rid();
    framebuffers[rid] = Framebuffer{fbo, attachments, width, height};
    return rid;
}

void RenderingDeviceGLES3::framebuffer_destroy(RID framebuffer) {
    auto it = framebuffers.find(framebuffer);
    if (it != framebuffers.end()) {
        GLuint fbo = (GLuint) it->second.handle;
        glDeleteFramebuffers(1, &fbo);
        framebuffers.erase(it);
    }
}

void RenderingDeviceGLES3::render_pass_begin(RID framebuffer, const Viewport& viewport, const Scissor& scissor) {
    if (framebuffer == INVALID_RID) {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    } else {
        auto it = framebuffers.find(framebuffer);
        if (it == framebuffers.end()) {
            return;
        }

        glBindFramebuffer(GL_FRAMEBUFFER, (GLuint) it->second.handle);
        current_framebuffer = framebuffer;

        for (const auto& attachment : it->second.attachments) {
            if (attachment.clear) {
                auto tex_it = textures.find(attachment.texture);
                if (tex_it != textures.end()) {
                    if (tex_it->second.format.depth_stencil) {
                        glClearDepth(attachment.clear_value.depth);
                        glClearStencil(attachment.clear_value.stencil);
                        glClear(GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
                    } else {
                        glClearColor(attachment.clear_value.color.r, attachment.clear_value.color.g, attachment.clear_value.color.b,
                                     attachment.clear_value.color.a);
                        glClear(GL_COLOR_BUFFER_BIT);
                    }
                }
            }
        }
    }

    set_viewport(viewport);
    set_scissor(scissor);
}

void RenderingDeviceGLES3::render_pass_end() {
    current_framebuffer = INVALID_RID;
}

void RenderingDeviceGLES3::set_viewport(const Viewport& viewport) {
    glViewport(static_cast<GLint>(viewport.x), static_cast<GLint>(viewport.y), static_cast<GLsizei>(viewport.width),
               static_cast<GLsizei>(viewport.height));
    glDepthRangef(viewport.min_depth, viewport.max_depth);
}

void RenderingDeviceGLES3::set_scissor(const Scissor& scissor) {
    glEnable(GL_SCISSOR_TEST);
    glScissor(scissor.x, scissor.y, scissor.width, scissor.height);
}

void RenderingDeviceGLES3::clear_color(const glm::vec4& color) {
    glClearColor(color.r, color.g, color.b, color.a);
    glClear(GL_COLOR_BUFFER_BIT);
}

void RenderingDeviceGLES3::clear_depth_stencil(float depth, uint32_t stencil) {
    glClearDepth(depth);
    glClearStencil(stencil);
    glClear(GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
}

void RenderingDeviceGLES3::swap_buffers() {
    SDL_GL_SwapWindow(_window);
}

void RenderingDeviceGLES3::push_constant(const String& name, const void* data, size_t size) {
    if (current_pipeline == INVALID_RID) {
        return;
    }

    auto pipe_it = pipelines.find(current_pipeline);
    if (pipe_it == pipelines.end()) {
        return;
    }

    auto shader_it = shaders.find(pipe_it->second.state.shader);
    if (shader_it == shaders.end()) {
        return;
    }

    auto& shader   = shader_it->second;
    GLint location = glGetUniformLocation(shader.program, name.c_str());

    if (location == -1) {
        return;
    }

    if (size == sizeof(float)) {
        glUniform1f(location, *static_cast<const float*>(data));
    } else if (size == sizeof(glm::vec2)) {
        glUniform2fv(location, 1, static_cast<const float*>(data));
    } else if (size == sizeof(glm::vec3)) {
        glUniform3fv(location, 1, static_cast<const float*>(data));
    } else if (size == sizeof(glm::vec4)) {
        glUniform4fv(location, 1, static_cast<const float*>(data));
    } else if (size == sizeof(glm::mat4)) {
        glUniformMatrix4fv(location, 1, GL_FALSE, static_cast<const float*>(data));
    } else if (size == sizeof(glm::mat3)) {
        glUniformMatrix3fv(location, 1, GL_FALSE, static_cast<const float*>(data));
    } else if (size == sizeof(int)) {
        glUniform1i(location, *static_cast<const int*>(data));
    }
}

void RenderingDeviceGLES3::texture_update(RID texture, uint32_t mip_level, uint32_t layer, const void* data, size_t size) {
    auto it = textures.find(texture);
    if (it == textures.end()) {
        return;
    }

    const auto& tex = it->second;
    GLenum target   = GL_TEXTURE_2D;
    if (tex.format.type == TextureType::TEXTURE_TYPE_CUBEMAP) {
        target = GL_TEXTURE_CUBE_MAP;
    }

    glBindTexture(target, (GLuint) tex.handle);

    GLenum pixel_format = GL_RGBA;
    GLenum pixel_type   = GL_UNSIGNED_BYTE;

    if (tex.format.format == DataFormat::R8G8B8_UNORM) {
        pixel_format = GL_RGB;
    } else if (tex.format.format == DataFormat::R8G8_UNORM) {
        pixel_format = GL_RG;
    } else if (tex.format.format == DataFormat::R8_UNORM) {
        pixel_format = GL_RED;
    }

    if (tex.format.type == TextureType::TEXTURE_TYPE_2D) {
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        glTexSubImage2D(GL_TEXTURE_2D, mip_level, 0, 0, tex.format.width >> mip_level, tex.format.height >> mip_level, pixel_format,
                        pixel_type, data);
    }

    glBindTexture(target, 0);
}

void RenderingDeviceGLES3::texture_generate_mipmaps(RID texture) {
    auto it = textures.find(texture);
    if (it == textures.end()) {
        return;
    }


    GLenum target = GL_TEXTURE_2D;
    if (it->second.format.type == TextureType::TEXTURE_TYPE_CUBEMAP) {
        target = GL_TEXTURE_CUBE_MAP;
    }

    glBindTexture(target, (GLuint) it->second.handle);
    glGenerateMipmap(target);
    glBindTexture(target, 0);
}

void RenderingDeviceGLES3::begin_frame() {
    // Placeholder for frame setup logic
}

void RenderingDeviceGLES3::end_frame() {
    // Placeholder for frame cleanup/present logic
}

GLenum RenderingDeviceGLES3::to_gl_stencil_op(StencilOp op) {
    switch (op) {
    case StencilOp::KEEP:
        return GL_KEEP;
    case StencilOp::ZERO:
        return GL_ZERO;
    case StencilOp::REPLACE:
        return GL_REPLACE;
    case StencilOp::INCREMENT_AND_CLAMP:
        return GL_INCR;
    case StencilOp::DECREMENT_AND_CLAMP:
        return GL_DECR;
    case StencilOp::INVERT:
        return GL_INVERT;
    case StencilOp::INCREMENT_AND_WRAP:
        return GL_INCR_WRAP;
    case StencilOp::DECREMENT_AND_WRAP:
        return GL_DECR_WRAP;
    default:
        return GL_KEEP;
    }
}
