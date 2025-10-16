#include "core/renderer/opengl/ogl_struct.h"

#include "core/io/file_system.h"


#if defined(SDL_PLATFORM_ANDROID) || defined(SDL_PLATFORM_IOS) || defined(SDL_PLATFORM_EMSCRIPTEN)
#define SHADER_HEADER "#version 300 es\nprecision mediump float;\n"
#else
#define SHADER_HEADER "#version 330 core\n"
#endif

bool validate_gl_shader(GLuint shader, GLuint op) {
    int success;
    glGetShaderiv(shader, op, &success);

    const char* op_str = (op == GL_COMPILE_STATUS)  ? "COMPILE"
                       : (op == GL_LINK_STATUS)     ? "LINK"
                       : (op == GL_VALIDATE_STATUS) ? "VALIDATE"
                                                    : "UNKNOWN";
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(shader, 512, NULL, infoLog);
        LOG_CRITICAL("OPENGLSHADER::%s:ERROR: %s", op_str, infoLog);
        return false;
    }

    return true;
}

OpenglShader::OpenglShader(const std::string& vertex, const std::string& fragment) {
    LOG_INFO("Compiling Shaders Sources Vertex (%s) | Fragment (%s)", vertex.c_str(), fragment.c_str());


    const std::string vertexSource   = SHADER_HEADER + load_assets_file(vertex);
    const std::string fragmentSource = SHADER_HEADER + load_assets_file(fragment);


    Uint32 vs = compile_shader(GL_VERTEX_SHADER, vertexSource.c_str());
    Uint32 fs = compile_shader(GL_FRAGMENT_SHADER, fragmentSource.c_str());

    Uint32 program = glCreateProgram();
    glAttachShader(program, vs);
    glAttachShader(program, fs);
    glLinkProgram(program);

    bool success = validate_gl_shader(program, GL_COMPILE_STATUS);
    success &= validate_gl_shader(program, GL_LINK_STATUS);
    success &= validate_gl_shader(program, GL_VALIDATE_STATUS);


    SDL_assert(success == GL_TRUE);


    glDeleteShader(vs);
    glDeleteShader(fs);

    this->id = program;

    // LOG_INFO("Successfully created and linked SHADER_PROGRAM(%d)", program);
}

Uint32 OpenglShader::compile_shader(Uint32 type, const char* source) {
    LOG_INFO("OpenglShader::CompileShader() - %s", type == GL_VERTEX_SHADER ? "VERTEX" : "FRAGMENT");

    EMBER_TIMER_START();

    Uint32 shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);

    bool success = validate_gl_shader(shader, GL_COMPILE_STATUS);

    EMBER_TIMER_END("Compiling Shaders");


    return shader;
}

Uint32 OpenglShader::get_uniform_location(const std::string& name) {
    if (_uniforms.find(name) != _uniforms.end()) {
        return _uniforms[name];
    }

    Uint32 location = glGetUniformLocation(id, name.c_str());

    if (location == -1) {
        LOG_WARN("Uniform %s not found", name.c_str());
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
        LOG_WARN("OpenglShader::set_value - Invalid matrix array or count is zero");
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

void OpenglMesh::bind() {
    glBindVertexArray(vao);
}

void OpenglMesh::draw(EDrawMode mode) {

    auto draw_mode = mode == EDrawMode::TRIANGLES ? GL_TRIANGLES : GL_LINES;
    if (ebo) {
        glDrawElements(draw_mode, index_count, GL_UNSIGNED_INT, 0);
    } else {
        glDrawArrays(draw_mode, 0, vertex_count);
    }
}

void OpenglMesh::unbind() {
    glBindVertexArray(0);
}

void OpenglMesh::destroy() {

    if (vbo) {
        glDeleteBuffers(1, &vbo);
    }

    if (ebo) {
        glDeleteBuffers(1, &ebo);
    }

    if (bone_id_vbo) {
        glDeleteBuffers(1, &bone_id_vbo);
    }

    if (bone_weight_vbo) {
        glDeleteBuffers(1, &bone_weight_vbo);
    }

    if (vao) {
        glDeleteVertexArrays(1, &vao);
    }

    if (texture_id) {
        glDeleteTextures(1, &texture_id);
    }
}


OpenglMesh::~OpenglMesh() {
    destroy();
}
