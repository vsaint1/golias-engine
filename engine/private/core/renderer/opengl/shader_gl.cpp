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
    LOG_INFO("Compiling Shaders Vertex (%s) | Fragment (%s)", vertex.c_str(), fragment.c_str());

    const auto bake_shader = [](const std::string& source) -> std::string {
        std::string header;

#if defined(SDL_PLATFORM_ANDROID) || defined(SDL_PLATFORM_IOS) || defined(SDL_PLATFORM_EMSCRIPTEN)
        header += "#version 300 es\n";
        header += "precision mediump float;\n";
#else
        header += "#version 330 core\n";
#endif

        const std::vector<std::string> defines = {"USE_TEXTURE_ARRAY"};

        for (const auto& def : defines) {
            header += "#define " + def + "\n";
        }

        header += "\n";

        return header + source;
    };

    const std::string vertexSource   = LoadAssetsFile(vertex);
    const std::string fragmentSource = LoadAssetsFile(fragment);

    const std::string vertexShaderSrc   = bake_shader(vertexSource);
    const std::string fragmentShaderSrc = bake_shader(fragmentSource);

    unsigned int vs = CompileShader(GL_VERTEX_SHADER, vertexShaderSrc.c_str());
    unsigned int fs = CompileShader(GL_FRAGMENT_SHADER, fragmentShaderSrc.c_str());

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

    LOG_INFO("Successfully linked SHADER_PROGRAM %d", program);

    glDeleteShader(vs);
    glDeleteShader(fs);

    this->id = program;
    LOG_INFO("Successfully created SHADER_PROGRAM %d", id);
}

unsigned int OpenglShader::CompileShader(unsigned int type, const char* source) {
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

    LOG_INFO("Successfully compiled %s", type == GL_VERTEX_SHADER ? "VERTEX" : "FRAGMENT");

    return shader;
}

unsigned int OpenglShader::GetUniformLocation(const std::string& name) {
    if (uniforms.find(name) != uniforms.end()) {
        return uniforms[name];
    }

    unsigned int location = glGetUniformLocation(id, name.c_str());

    if (location == -1) {
        LOG_WARN("Uniform %s not found", name.c_str());
    }

    uniforms[name] = location;
    return location;
}

bool OpenglShader::IsValid() const {
    int status;

    glGetProgramiv(id, GL_VALIDATE_STATUS, &status);

    char infoLog[512];
    if (!status) {
        glGetProgramInfoLog(id, 512, nullptr, infoLog);
        LOG_CRITICAL("SHADER_PROGARM validation failed: %s", infoLog);
        return false;
    }

    return true;
}

void OpenglShader::Bind() const {
    glUseProgram(id);
}

void OpenglShader::Destroy() {
    glDeleteProgram(id);
}

unsigned int OpenglShader::GetID() const {
    return id;
}


void OpenglShader::SetValue(const std::string& name, float value) {
    unsigned int location = GetUniformLocation(name);
    glUniform1f(location, value);
}

void OpenglShader::SetValue(const std::string& name, int value) {
    unsigned int location = GetUniformLocation(name);
    glUniform1i(location, value);
}
void OpenglShader::SetValue(const std::string& name, unsigned int value) {
    unsigned int location = GetUniformLocation(name);
    glUniform1i(location, value);
}

void OpenglShader::SetValue(const std::string& name, glm::mat4 value) {
    unsigned int location = GetUniformLocation(name);
    glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(value));
}

void OpenglShader::SetValue(const std::string& name, glm::vec2 value) {
    unsigned int location = GetUniformLocation(name);
    glUniform2fv(location, 1, glm::value_ptr(value));
}

void OpenglShader::SetValue(const std::string& name, glm::vec3 value) {
    unsigned int location = GetUniformLocation(name);
    glUniform3fv(location, 1, glm::value_ptr(value));
}

void OpenglShader::SetValue(const std::string& name, glm::vec4 value) {
    unsigned int location = GetUniformLocation(name);
    glUniform4fv(location, 1, glm::value_ptr(value));
}
