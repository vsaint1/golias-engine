#pragma once
#include "core/io/file_system.h"

/*!

    @brief Vertex structure
    - Position
    - Normal
    - UV coordinates

    @version  0.0.1

*/
struct Vertex {
    glm::vec3 position; /// 3D position
    glm::vec3 normal; /// 3D normal
    glm::vec2 uv; /// 2D texture coordinates
};

constexpr int MAX_BONES = 250;


/*!

    @brief Shader Abstract class
    - Compile the shader
    - Bind the shader
    - Send uniforms

    @version  0.0.2
    @param string vertex The shader source
    @param string fragment The shader source
*/
class Shader {
public:
    Shader() = default;

    virtual ~Shader() = default;

    virtual void activate() const = 0;

    virtual void set_value(const std::string& name, float value) = 0;

    virtual void set_value(const std::string& name, int value) = 0;

    virtual void set_value(const std::string& name, Uint32 value) = 0;

    virtual void set_value(const std::string& name, glm::mat4 value, Uint32 count = 1) =0;

    virtual void set_value(const std::string& name, const int* value, Uint32 count = 1) =0;

    virtual void set_value(const std::string& name, const float* value, Uint32 count = 1) =0;

    virtual void set_value(const std::string& name, glm::vec2 value, Uint32 count = 1) =0;

    virtual void set_value(const std::string& name, glm::vec3 value, Uint32 count = 1) =0;

    virtual void set_value(const std::string& name, glm::vec4 value, Uint32 count = 1) =0;

    virtual void set_value(const std::string& name, const glm::mat4* values, Uint32 count = 1) =0;


    virtual void destroy() = 0;

    virtual Uint32 get_id() const = 0;

    virtual bool is_valid() const = 0;

protected:
    Uint32 id = 0;

    std::unordered_map<std::string, Uint32> _uniforms;
};

// Forward declaration
class Renderer;
