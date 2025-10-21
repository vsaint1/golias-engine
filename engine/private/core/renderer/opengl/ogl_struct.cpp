#include "core/renderer/opengl/ogl_struct.h"

#include "core/io/file_system.h"


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
            LOG_CRITICAL("OPENGLPROGRAM::%s:ERROR: %s", op_str, infoLog);
            return false;
        }
       
    } else {
        glGetShaderiv(handle, op, &success);
        if (!success) {
            glGetShaderInfoLog(handle, sizeof(infoLog), nullptr, infoLog);
            LOG_CRITICAL("OPENGLSHADER::%s:ERROR: %s", op_str, infoLog);
            return false;
        }
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

    bool success = validate_gl_shader(program, GL_LINK_STATUS, true);
    
    success &= validate_gl_shader(program, GL_VALIDATE_STATUS, true);

    if (!success) {
        LOG_CRITICAL("Shader program setup failed, exiting");
        glDeleteProgram(program);
        program = 0;
        exit(EXIT_FAILURE);
    }

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

    if (!success) {
        LOG_CRITICAL("Shader compilation failed, deleting shader");
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

void OpenglMesh::upload_to_gpu() {
   
}

void OpenglMesh::bind() {
    glBindVertexArray(vao);
}

void OpenglMesh::draw(EDrawMode mode) {

    auto draw_mode = mode == EDrawMode::TRIANGLES ? GL_TRIANGLES : GL_LINES;
    if (ebo && !indices.empty()) {
        glDrawElements(draw_mode, static_cast<GLsizei>(index_count), GL_UNSIGNED_INT, 0);
    } else {
        glDrawArrays(draw_mode, 0, static_cast<GLsizei>(vertex_count));
    }
}

void OpenglMesh::unbind() {
    glBindVertexArray(0);
}

void OpenglMesh::destroy() {

    if (vbo) {
        glDeleteBuffers(1, &vbo);
        vbo = 0;
    }

    if (ebo) {
        glDeleteBuffers(1, &ebo);
        ebo = 0;
    }

    if (bone_id_vbo) {
        glDeleteBuffers(1, &bone_id_vbo);
        bone_id_vbo = 0;
    }

    if (bone_weight_vbo) {
        glDeleteBuffers(1, &bone_weight_vbo);
        bone_weight_vbo = 0;
    }


    if (vao) {
        glDeleteVertexArrays(1, &vao);
        vao = 0;
    }

    material.reset(); // free material resource

}


OpenglMesh::~OpenglMesh() {
    destroy();
}


GLuint gl_texture_target_cast(ETextureTarget target) {
    switch (target) {
    case ETextureTarget::TEXTURE_2D:
        return GL_TEXTURE_2D;
    case ETextureTarget::TEXTURE_3D:
        return GL_TEXTURE_3D;
    case ETextureTarget::TEXTURE_CUBE_MAP:
        return GL_TEXTURE_CUBE_MAP;
    case ETextureTarget::RENDER_TARGET:
        SDL_Log("RENDER_TARGET not directly supported in OpenGL");
        return GL_TEXTURE_2D; // Fallback
    default:
        SDL_Log("Unknown texture target");
        return GL_TEXTURE_2D; // Fallback
    }
}


void OpenglTexture::bind(Uint32 slot) {
    glActiveTexture(GL_TEXTURE0 + slot);
    glBindTexture(gl_texture_target_cast(target), id);
    // LOG_DEBUG("Binding texture ID %d to slot %d", id, slot);
}

OpenglTexture::~OpenglTexture() {
    if (is_valid()) {
        glDeleteTextures(1, &id);
        id = -1;
    }
}
