#include  "core/renderer/opengl/ogl_struct.h"

#include "core/io/assimp_io.h"

#if defined(SDL_PLATFORM_ANDROID) || defined(SDL_PLATFORM_IOS) || defined(SDL_PLATFORM_EMSCRIPTEN)
#define SHADER_HEADER "#version 300 es\nprecision highp float;\n\n"
#else
#define SHADER_HEADER "#version 330 core\n\n"
#endif

bool validate_gl_shader(GLuint handle, GLenum op, bool is_program = false) {
    GLint success = 0;
    char infoLog[1024];

    const char* op_str = (op == GL_COMPILE_STATUS)
                             ? "COMPILE"
                             : (op == GL_LINK_STATUS)
                             ? "LINK"
                             : (op == GL_VALIDATE_STATUS)
                             ? "VALIDATE"
                             : "UNKNOWN";

    if (is_program) {
        if (op == GL_VALIDATE_STATUS) {
            glValidateProgram(handle);
        }


        glGetProgramiv(handle, op, &success);
        if (!success) {
            glGetProgramInfoLog(handle, sizeof(infoLog), nullptr, infoLog);
            spdlog::critical("OPENGLSHADER::{}:ERROR: {}", op_str, infoLog);
            return false;
        }

    } else {
        glGetShaderiv(handle, op, &success);
        if (!success) {
            glGetShaderInfoLog(handle, sizeof(infoLog), nullptr, infoLog);
            spdlog::critical("OPENGLSHADER::{}:ERROR: {}", op_str, infoLog);
            return false;
        }
    }

    return true;
}

OpenglShader::OpenglShader(const std::string& vertex, const std::string& fragment) {
    spdlog::info("OpenglShader::OpenglShader - Compiling Shaders Sources Vertex ({}) | Fragment ({})", vertex, fragment);


    const std::string vertexSource   = SHADER_HEADER + load_assets_file(vertex);
    const std::string fragmentSource = SHADER_HEADER + load_assets_file(fragment);


    Uint32 vs = compile_shader(GL_VERTEX_SHADER, vertexSource.c_str());
    Uint32 fs = compile_shader(GL_FRAGMENT_SHADER, fragmentSource.c_str());

    Uint32 program = glCreateProgram();
    glAttachShader(program, vs);
    glAttachShader(program, fs);
    glLinkProgram(program);

    glUseProgram(program);
    glUniform1i(glGetUniformLocation(program, "ALBEDO_MAP"), ALBEDO_TEXTURE_UNIT);
    glUniform1i(glGetUniformLocation(program, "METALLIC_MAP"), METALLIC_TEXTURE_UNIT);
    glUniform1i(glGetUniformLocation(program, "ROUGHNESS_MAP"), ROUGHNESS_TEXTURE_UNIT);
    glUniform1i(glGetUniformLocation(program, "NORMAL_MAP"), NORMAL_MAP_TEXTURE_UNIT);
    glUniform1i(glGetUniformLocation(program, "AO_MAP"), AMBIENT_OCCLUSION_TEXTURE_UNIT);
    glUniform1i(glGetUniformLocation(program, "EMISSIVE_MAP"), EMISSIVE_TEXTURE_UNIT);
    glUniform1i(glGetUniformLocation(program, "SHADOW_MAP"), SHADOW_TEXTURE_UNIT);
    glUniform1i(glGetUniformLocation(program, "ENVIRONMENT_MAP"), ENVIRONMENT_TEXTURE_UNIT);

    const bool success = validate_gl_shader(program, GL_LINK_STATUS, true);

    // success &= validate_gl_shader(program, GL_VALIDATE_STATUS, true);

    if (!success) {
        spdlog::critical("OpenglShader::OpenglShader - Shader program setup failed");
        glDeleteProgram(program);
        program = 0;
        exit(EXIT_FAILURE);
    }

    glDeleteShader(vs);
    glDeleteShader(fs);

    this->id = program;

}

Uint32 OpenglShader::compile_shader(Uint32 type, const char* source) {
    spdlog::info("OpenglShader::CompileShader - Compiling shader of type {}", type == GL_VERTEX_SHADER ? "VERTEX" : "FRAGMENT");


    Uint32 shader = glCreateShader(type);

    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);

    bool success = validate_gl_shader(shader, GL_COMPILE_STATUS);


    if (!success) {
        spdlog::critical("OpenglShader::compile_shader - Shader compilation failed for type {}",
                         type == GL_VERTEX_SHADER ? "VERTEX" : "FRAGMENT");
        glDeleteShader(shader);
        exit(EXIT_FAILURE);

        return 0;
    }

    return shader;
}

Uint32 OpenglShader::get_uniform_location(const std::string& name) {
    if (_uniforms.find(name) != _uniforms.end()) {
        return _uniforms[name];
    }

    Uint32 location = glGetUniformLocation(id, name.c_str());

    if (location == -1) {
        spdlog::warn("OpenglShader::get_uniform_location - Uniform {} not found in shader {}", name, id);
        return location;
    }

    _uniforms[name] = location;
    return location;
}

bool OpenglShader::is_valid() const {

    bool validated = glIsProgram(id);
    return validated;
}

void OpenglShader::activate() const {
    glUseProgram(id);
}

void OpenglShader::destroy() {
    glDeleteProgram(id);
}


OpenglGpuVertexLayout::OpenglGpuVertexLayout(const GpuBuffer* vertex_buffer, const GpuBuffer* index_buffer,
                                             const std::vector<VertexAttribute>& attributes, uint32_t stride) {
    glGenVertexArrays(1, &_vao);
    glBindVertexArray(_vao);

    vertex_buffer->bind();

    for (const auto& attr : attributes) {
        GLenum gl_type = to_gl_type(attr.type);
        glVertexAttribPointer(
            attr.location,
            attr.components,
            gl_type,
            attr.normalized ? GL_TRUE : GL_FALSE,
            stride,
            (void*) (uintptr_t) attr.offset
            );
        glEnableVertexAttribArray(attr.location);
    }

    if (index_buffer) {
        index_buffer->bind();
    }

    glBindVertexArray(0);
}

OpenglGpuVertexLayout::~OpenglGpuVertexLayout() {
    if (_vao)
        glDeleteVertexArrays(1, &_vao);
}

void OpenglGpuVertexLayout::bind() const {
    glBindVertexArray(_vao);
}

void OpenglGpuVertexLayout::unbind() const {
    glBindVertexArray(0);
}

GLenum OpenglGpuVertexLayout::to_gl_type(DataType type) {
    switch (type) {
    case DataType::USHORT:
        return GL_UNSIGNED_SHORT;
    case DataType::FLOAT:
        return GL_FLOAT;
    case DataType::INT:
        return GL_INT;
    case DataType::UNSIGNED_INT:
        return GL_UNSIGNED_INT;
    default:
        return GL_FLOAT;
    }
}

OpenglGpuBuffer::OpenglGpuBuffer(GpuBufferType type) : _buffer_type(type) {

    switch (type) {
    case GpuBufferType::VERTEX:
        _target = GL_ARRAY_BUFFER;
        break;
    case GpuBufferType::INDEX:
        _target = GL_ELEMENT_ARRAY_BUFFER;
        break;
    case GpuBufferType::UNIFORM:
        _target = GL_UNIFORM_BUFFER;
        break;
    default:
        _target = GL_ARRAY_BUFFER;
        break;
    }


    glGenBuffers(1, &_id);

    spdlog::info("OpenglGpuBuffer::OpenglGpuBuffer - Created GPU Buffer ID: {} of Type: {}", _id,
                 type == GpuBufferType::VERTEX
                     ? "VERTEX"
                     : type == GpuBufferType::INDEX
                     ? "INDEX"
                     : type == GpuBufferType::UNIFORM
                     ? "UNIFORM"
                     : "STORAGE");
}

OpenglGpuBuffer::~OpenglGpuBuffer() {
    glDeleteBuffers(1, &_id);
}

void OpenglGpuBuffer::bind() const {
    glBindBuffer(_target, _id);
}

void OpenglGpuBuffer::upload(const void* data, size_t size) {
    bind();
    glBufferData(_target, size, data, GL_STATIC_DRAW);
    _buffer_size = size;
}

size_t OpenglGpuBuffer::size() const {
    return _buffer_size;
}

GpuBufferType OpenglGpuBuffer::type() const {
    return _buffer_type;
}

OpenGLFramebuffer::OpenGLFramebuffer(const FramebufferSpecification& spec) : specification(spec) {
    spdlog::info("OpenGLFramebuffer::OpenGLFramebuffer - Creating Framebuffer ({}x{})", spec.width, spec.height);
    invalidate();
}

OpenGLFramebuffer::~OpenGLFramebuffer() {
    cleanup();
}


void OpenGLFramebuffer::invalidate() {
    if (fbo)
        cleanup();

    if (specification.width == 0 || specification.height == 0) {
        spdlog::error("OpenGLFramebuffer::invalidate - Invalid dimensions ({}x{})",
                      specification.width, specification.height);
        return;
    }

    spdlog::info("OpenGLFramebuffer::invalidate - Recreating Framebuffer ({}x{})",
                 specification.width, specification.height);

    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    bool hasDepthAttachment = false;
    for (auto& attachment : specification.attachments.attachments) {
        switch (attachment.format) {
        case FramebufferTextureFormat::RGBA8: {
            uint32_t tex;
            glGenTextures(1, &tex);
            glBindTexture(GL_TEXTURE_2D, tex);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, specification.width, specification.height, 0,
                         GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + color_attachments.size(),
                                   GL_TEXTURE_2D, tex, 0);
            color_attachments.push_back(tex);
            break;
        }
        case FramebufferTextureFormat::DEPTH_COMPONENT:
        case FramebufferTextureFormat::DEPTH24STENCIL8: {
            hasDepthAttachment = true;
            glGenTextures(1, &depth_attachment);
            glBindTexture(GL_TEXTURE_2D, depth_attachment);

            if (attachment.format == FramebufferTextureFormat::DEPTH24STENCIL8) {
                glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, specification.width, specification.height,
                             0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, nullptr);
                glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, depth_attachment, 0);
            } else {
                glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, specification.width, specification.height,
                             0, GL_DEPTH_COMPONENT, GL_UNSIGNED_INT, nullptr);
                glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depth_attachment, 0);
            }

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            break;
        }
        default:
            spdlog::warn("Unsupported framebuffer attachment format");
            break;
        }
    }

    if (color_attachments.empty()) {
        glDrawBuffers(0, nullptr);
        glReadBuffer(GL_NONE);
    } else {
        GLenum buffers[4] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3};
        glDrawBuffers((GLsizei) color_attachments.size(), buffers);
    }

    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE) {
        spdlog::critical("OpenGLFramebuffer::invalidate - Framebuffer is incomplete! Status: 0x{:x}", status);

        switch (status) {
            case GL_FRAMEBUFFER_UNDEFINED:
                spdlog::error("  -> GL_FRAMEBUFFER_UNDEFINED");
                break;
            case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
                spdlog::error("  -> GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT");
                break;
            case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
                spdlog::error("  -> GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT");
                break;
            case GL_FRAMEBUFFER_UNSUPPORTED:
                spdlog::error("  -> GL_FRAMEBUFFER_UNSUPPORTED");
                break;
        }
    } else {
        spdlog::info("Framebuffer created successfully ({}x{})", specification.width, specification.height);
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}



void OpenGLFramebuffer::bind() {
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glViewport(0, 0, specification.width, specification.height);
}

void OpenGLFramebuffer::unbind() {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void OpenGLFramebuffer::resize(unsigned int width, unsigned int height) {
    specification.width  = width;
    specification.height = height;
    invalidate();
}

Uint32 OpenGLFramebuffer::get_fbo_id() const {
    return fbo;
}

Uint32 OpenGLFramebuffer::get_color_attachment_id(size_t index) const {
    return color_attachments[index];
}

Uint32 OpenGLFramebuffer::get_depth_attachment_id() const {
    return depth_attachment;
}

const FramebufferSpecification& OpenGLFramebuffer::get_specification() const {
    return specification;
}

void OpenGLFramebuffer::cleanup() {
    if (fbo != 0) {
        glDeleteFramebuffers(1, &fbo);
        fbo = 0;
    }

    for (auto& tex_id : color_attachments) {
        if (tex_id != 0) {
            glDeleteTextures(1, &tex_id);
        }
    }
    color_attachments.clear();

    if (depth_attachment != 0) {
        glDeleteTextures(1, &depth_attachment);
        depth_attachment = 0;
    }
}

OpenglShader::~OpenglShader() {
    destroy();
}

Uint32 OpenglShader::get_id() const {
    return id;
}


void OpenglShader::set_value(const char* name, float value) {
    const Uint32 location = get_uniform_location(name);
    glUniform1f(location, value);
}

void OpenglShader::set_value(const char* name, int value) {
    const Uint32 location = get_uniform_location(name);
    glUniform1i(location, value);
}

void OpenglShader::set_value(const char* name, Uint32 value) {
    const Uint32 location = get_uniform_location(name);
    glUniform1i(location, value);
}

void OpenglShader::set_value(const char* name, const int* value, Uint32 count) {
    const Uint32 location = get_uniform_location(name);
    glUniform1iv(location, count, value);
}

void OpenglShader::set_value(const char* name, const float* value, Uint32 count) {
    const Uint32 location = get_uniform_location(name);
    glUniform1fv(location, count, value);
}


void OpenglShader::set_value(const char* name, glm::mat4 value, Uint32 count) {
    const Uint32 location = get_uniform_location(name);
    glUniformMatrix4fv(location, count, GL_FALSE, glm::value_ptr(value));
}

void OpenglShader::set_value(const char* name, const glm::mat4* values, Uint32 count) {
    if (values == nullptr || count == 0) {
        spdlog::warn("OpenglShader::set_value - Invalid matrix array or count is zero");
        return;
    }

    const Uint32 location = get_uniform_location(name);
    glUniformMatrix4fv(location, count, GL_FALSE, glm::value_ptr(*values));
}

void OpenglShader::set_value(const char* name, glm::vec2 value, Uint32 count) {
    const Uint32 location = get_uniform_location(name);
    glUniform2fv(location, count, glm::value_ptr(value));
}

void OpenglShader::set_value(const char* name, glm::vec3 value, Uint32 count) {
    const Uint32 location = get_uniform_location(name);
    glUniform3fv(location, count, glm::value_ptr(value));
}

void OpenglShader::set_value(const char* name, glm::vec4 value, Uint32 count) {
    const Uint32 location = get_uniform_location(name);
    glUniform4fv(location, count, glm::value_ptr(value));
}
