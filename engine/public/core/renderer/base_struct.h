#pragma once

#include "stdafx.h"

enum class CUBEMAP_ORIENTATION {
    DEFAULT, // original layout
    TOP, //  (+Y)
    BOTTOM, // (-Y)
    FLIP_X, // flip left-right
    FLIP_Y // flip up-down
};

struct Tokens {
    std::string text;
    bool is_emoji;
};


/*!

    @brief Font Abstract class
    - Get size of the text
    - Render text to texture

    @version  0.0.1
    @param string path The font path
*/
class Font {
public:
    virtual ~Font() = default;

    virtual glm::vec2 get_size(const std::string& text) {
        return {0, 0};
    }
};

/*!

    @brief Texture Abstract class


    @version  0.0.1
    @param string path The texture path
*/
class Texture {
public:
    Uint32 id        = 0;
    int width        = 0;
    int height       = 0;
    std::string path = "";

    virtual ~Texture() = default;
};

struct Material {
    glm::vec3 diffuse  = glm::vec3(0.5f);
    glm::vec3 specular = glm::vec3(1.f);
    float shininess    = 1.f;
};


struct Vertex {
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 uv;
};

enum class EDrawMode {
    LINES,
    TRIANGLES,
};

struct Mesh {
    std::string name = "UNNAMED_MESH";

    size_t vertex_count = 0;
    size_t index_count  = 0;

    std::vector<glm::vec3> vertices;
    std::vector<unsigned int> indices;

    Material material;

    virtual bool has_texture() const = 0;

    virtual void bind() = 0;

    virtual void draw(EDrawMode mode = EDrawMode::TRIANGLES) = 0;

    virtual void unbind() = 0;

    Mesh()          = default;
    virtual ~Mesh() = default;

protected:
    virtual void destroy() = 0;
};


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

    virtual void set_value(const std::string& name, const int* values, Uint32 count) = 0;

    virtual void set_value(const std::string& name, const float* values, Uint32 count) = 0;

    virtual void set_value(const std::string& name, glm::mat4 value, Uint32 count) = 0;

    virtual void set_value(const std::string& name, glm::vec2 value, Uint32 count) = 0;

    virtual void set_value(const std::string& name, glm::vec3 value, Uint32 count) = 0;

    virtual void set_value(const std::string& name, glm::vec4 value, Uint32 count) = 0;

    virtual void set_value(const std::string& name, Uint32 value) = 0;

    virtual void destroy() = 0;

    virtual Uint32 get_id() const = 0;

    virtual bool is_valid() const = 0;


protected:
    Uint32 id = 0;

    std::unordered_map<std::string, Uint32> _uniforms;
};
