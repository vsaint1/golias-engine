#pragma once
#include "core/file_system.h"
/*!

    @brief Shader class
    - Compile the shader
    - Use the shader
    - Send uniforms

    @version 0.0.2
    @param string vertex The shader source
    @param string fragment The shader source
*/
class Shader {
public:
    Shader() = default;

    Shader(const std::string& vertex, const std::string& fragment);

    void Use() const;

    void SetValue(const std::string& name, float value) const;

    void SetValue(const std::string& name, int value) const;

    void SetValue(const std::string& name, glm::mat4 value) const;

    void SetValue(const std::string& name, glm::vec2 value) const;

    void SetValue(const std::string& name, glm::vec3 value) const;

    void SetValue(const std::string& name, glm::vec4 value) const;

    void Destroy() {
        glDeleteProgram(id);
    }

    unsigned int GetID() const {
        return id;
    }
    

private:
    unsigned int id;

    unsigned int GetUniformLocation(const std::string&name) const;
    
    unsigned int CompileShader(unsigned int type, const char* source);
};
