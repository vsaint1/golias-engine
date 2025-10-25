#include  "core/renderer/opengl/ogl_struct.h"


#if defined(SDL_PLATFORM_ANDROID) || defined(SDL_PLATFORM_IOS) || defined(SDL_PLATFORM_EMSCRIPTEN)
#define SHADER_HEADER "#version 300 es\nprecision highp float;\n\n"
#else
#define SHADER_HEADER "#version 330 core\n\n"
#endif

bool validate_gl_shader(GLuint handle, GLenum op, bool is_program = false) {
    GLint success = 0;
    char infoLog[1024];

    const char* op_str = (op == GL_COMPILE_STATUS)  ? "COMPILE"
                       : (op == GL_LINK_STATUS)     ? "LINK"
                       : (op == GL_VALIDATE_STATUS) ? "VALIDATE"
                                                    : "UNKNOWN";

    if (is_program) {
        if (op == GL_VALIDATE_STATUS) {
            glValidateProgram(handle);
        }


        glGetProgramiv(handle, op, &success);
        if (!success) {
            glGetProgramInfoLog(handle, sizeof(infoLog), nullptr, infoLog);
            spdlog::critical("OPENGLSHADER::%s:ERROR: %s", op_str, infoLog);
            return false;
        }

    } else {
        glGetShaderiv(handle, op, &success);
        if (!success) {
            glGetShaderInfoLog(handle, sizeof(infoLog), nullptr, infoLog);
            spdlog::critical("OPENGLSHADER::%s:ERROR: %s", op_str, infoLog);
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

    bool success = validate_gl_shader(program, GL_LINK_STATUS, true);

    success &= validate_gl_shader(program, GL_VALIDATE_STATUS, true);

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
        spdlog::critical("OpenglShader::compile_shader - Shader compilation failed for type {}", type);
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

OpenGLFramebuffer::OpenGLFramebuffer(const FramebufferSpecification& spec): specification(spec) {
    spdlog::info("OpenGLFramebuffer::OpenGLFramebuffer - Creating Framebuffer ({}x{})", spec.width, spec.height);
    invalidate();
}

OpenGLFramebuffer::~OpenGLFramebuffer() {
    cleanup();
}

void OpenGLFramebuffer::invalidate() {
    if (fbo)
        cleanup();

    spdlog::warn("OpenGLFramebuffer::invalidate - Recreating Framebuffer ({}x{})", specification.width, specification.height);
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
            glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, specification.width, specification.height,
                         0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depth_attachment, 0);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

            break;
        }
        default:
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

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        std::cerr << "Framebuffer incomplete!" << std::endl;
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
    if (depth_attachment)
        glDeleteTextures(1, &depth_attachment);

    if (!color_attachments.empty())
        glDeleteTextures((GLsizei) color_attachments.size(), color_attachments.data());

    if (fbo)
        glDeleteFramebuffers(1, &fbo);
}

OpenglShader::~OpenglShader() {
    destroy();
}

Uint32 OpenglShader::get_id() const {
    return id;
}


void OpenglShader::set_value(const std::string& name, float value) {
    const Uint32 location = get_uniform_location(name);
    glUniform1f(location, value);
}

void OpenglShader::set_value(const std::string& name, int value) {
    const Uint32 location = get_uniform_location(name);
    glUniform1i(location, value);
}
void OpenglShader::set_value(const std::string& name, Uint32 value) {
    const Uint32 location = get_uniform_location(name);
    glUniform1i(location, value);
}

void OpenglShader::set_value(const std::string& name, const int* value, Uint32 count) {
    const Uint32 location = get_uniform_location(name);
    glUniform1iv(location, count, value);
}

void OpenglShader::set_value(const std::string& name, const float* value, Uint32 count) {
    const Uint32 location = get_uniform_location(name);
    glUniform1fv(location, count, value);
}


void OpenglShader::set_value(const std::string& name, glm::mat4 value, Uint32 count) {
    const Uint32 location = get_uniform_location(name);
    glUniformMatrix4fv(location, count, GL_FALSE, glm::value_ptr(value));
}

void OpenglShader::set_value(const std::string& name, const glm::mat4* values, Uint32 count) {
    if (values == nullptr || count == 0) {
        spdlog::warn("OpenglShader::set_value - Invalid matrix array or count is zero");
        return;
    }

    const Uint32 location = get_uniform_location(name);
    glUniformMatrix4fv(location, count, GL_FALSE, glm::value_ptr(*values));
}

void OpenglShader::set_value(const std::string& name, glm::vec2 value, Uint32 count) {
    const Uint32 location = get_uniform_location(name);
    glUniform2fv(location, count, glm::value_ptr(value));
}

void OpenglShader::set_value(const std::string& name, glm::vec3 value, Uint32 count) {
    const Uint32 location = get_uniform_location(name);
    glUniform3fv(location, count, glm::value_ptr(value));
}

void OpenglShader::set_value(const std::string& name, glm::vec4 value, Uint32 count) {
    const Uint32 location = get_uniform_location(name);
    glUniform4fv(location, count, glm::value_ptr(value));
}