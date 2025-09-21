#include "core/renderer/opengl/shader_gl.h"

/*!

    @brief GLSL shader header (API `OpenGL`)

    @version 0.0.1
*/
#if defined(SDL_PLATFORM_ANDROID) || defined(SDL_PLATFORM_IOS) || defined(SDL_PLATFORM_EMSCRIPTEN)
#define SHADER_HEADER "#version 300 es\nprecision mediump float;\n"
#else
#define SHADER_HEADER "#version 330 core\n"
#endif

OpenglShader::OpenglShader(const std::string& vertex, const std::string& fragment) {
    LOG_INFO("Compiling Shaders Sources Vertex (%s) | Fragment (%s)", vertex.c_str(), fragment.c_str());


    const std::string vertexSource   = SHADER_HEADER + load_assets_file(vertex);
    const std::string fragmentSource = SHADER_HEADER + load_assets_file(fragment);


    unsigned int vs = compile_shader(GL_VERTEX_SHADER, vertexSource.c_str());
    unsigned int fs = compile_shader(GL_FRAGMENT_SHADER, fragmentSource.c_str());

    unsigned int program = glCreateProgram();
    glAttachShader(program, vs);
    glAttachShader(program, fs);
    glLinkProgram(program);

    int success;
    char infoLog[512];
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(program, 512, nullptr, infoLog);
        LOG_CRITICAL("SHADER_PROGRAM linking failed: %s", infoLog);
        exit(EXIT_FAILURE);
    }

    SDL_assert(success == GL_TRUE);


    glDeleteShader(vs);
    glDeleteShader(fs);

    this->id = program;

    LOG_INFO("Successfully created and linked SHADER_PROGRAM(%d)", program);
}

unsigned int OpenglShader::compile_shader(unsigned int type, const char* source) {
    LOG_INFO("OpenglShader::compile_shader()");

    EMBER_TIMER_START();

    unsigned int shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);

    int success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char info_log[512];
        glGetShaderInfoLog(shader, 512, nullptr, info_log);
        const char* type_str = type == GL_VERTEX_SHADER ? "VERTEX" : "FRAGMENT";
        LOG_CRITICAL("[%s] - Shader compilation failed: %s", type_str, info_log);
    }


    EMBER_TIMER_END("Compiling Shaders");

    LOG_INFO("Compiled %s", type == GL_VERTEX_SHADER ? "VERTEX" : "FRAGMENT");

    return shader;
}

unsigned int OpenglShader::get_uniform_location(const std::string& name) {
    if (_uniforms.find(name) != _uniforms.end()) {
        return _uniforms[name];
    }

    unsigned int location = glGetUniformLocation(id, name.c_str());

    if (location == -1) {
        LOG_WARN("Uniform %s not found", name.c_str());
    }

    _uniforms[name] = location;
    return location;
}

bool OpenglShader::is_valid() const {
    int status;

    glGetProgramiv(id, GL_VALIDATE_STATUS, &status);

    char infoLog[512];
    if (!status) {
        glGetProgramInfoLog(id, 512, nullptr, infoLog);
        LOG_CRITICAL("SHADER_PROGRAM validation failed: %s", infoLog);
        return false;
    }

    return true;
}

void OpenglShader::bind() const {
    glUseProgram(id);
}

void OpenglShader::destroy() {
    glDeleteProgram(id);
}

unsigned int OpenglShader::get_id() const {
    return id;
}


void OpenglShader::set_value(const std::string& name, float value) {
    const unsigned int location = get_uniform_location(name);
    glUniform1f(location, value);
}

void OpenglShader::set_value(const std::string& name, int value) {
    const unsigned int location = get_uniform_location(name);
    glUniform1i(location, value);
}
void OpenglShader::set_value(const std::string& name, unsigned int value) {
    const unsigned int location = get_uniform_location(name);
    glUniform1i(location, value);
}

void OpenglShader::set_value(const std::string& name, const int* value, Uint32 count) {
    const unsigned int location = get_uniform_location(name);
    glUniform1iv(location, count, value);
}

void OpenglShader::set_value(const std::string& name, const float* value, Uint32 count) {
    const unsigned int location = get_uniform_location(name);
    glUniform1fv(location, count, value);
}


void OpenglShader::set_value(const std::string& name, glm::mat4 value, Uint32 count) {
    const unsigned int location = get_uniform_location(name);
    glUniformMatrix4fv(location, count, GL_FALSE, glm::value_ptr(value));
}

void OpenglShader::set_value(const std::string& name, glm::vec2 value, Uint32 count) {
    const unsigned int location = get_uniform_location(name);
    glUniform2fv(location, count, glm::value_ptr(value));
}

void OpenglShader::set_value(const std::string& name, glm::vec3 value, Uint32 count) {
    const unsigned int location = get_uniform_location(name);
    glUniform3fv(location, count, glm::value_ptr(value));
}

void OpenglShader::set_value(const std::string& name, glm::vec4 value, Uint32 count) {
    const unsigned int location = get_uniform_location(name);
    glUniform4fv(location, count, glm::value_ptr(value));
}
