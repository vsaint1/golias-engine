#pragma once
#include "core/io/file_system.h"

/*!

    @brief Shader Abstract class
    - Compile the shader
    - Bind the shader
    - Send uniforms

    @version 0.0.2
    @param string vertex The shader source
    @param string fragment The shader source
*/
class Shader {
public:
    Shader() = default;
    
    virtual void Bind() const = 0;

    virtual void Destroy() = 0;

    virtual unsigned int GetID() const = 0;

    virtual bool IsValid() const = 0;

protected:
    unsigned int id;

    std::unordered_map<std::string, unsigned int> uniforms;
};
