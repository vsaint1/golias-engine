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