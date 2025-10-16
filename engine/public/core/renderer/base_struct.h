#pragma once

#include "stdafx.h"
/*!
    @brief Cube map orientation options
    
    @version 0.0.4

*/
enum class CUBEMAP_ORIENTATION {
    DEFAULT, // original layout
    TOP, //  (+Y)
    BOTTOM, // (-Y)
    FLIP_X, // flip left-right
    FLIP_Y // flip up-down
};


/*!

    @brief Texture types

    @version  0.0.1

*/
enum class ETextureType {
    TEXTURE_2D, /// e.g. GL_TEXTURE_2D
    TEXTURE_3D, /// e.g. GL_TEXTURE_3D
    TEXTURE_CUBE_MAP, /// e.g. GL_TEXTURE_CUBE_MAP
    RENDER_TARGET /// e.g. Metal Depth Texture
};

/*!

    @brief Draw modes
    - LINES
    - TRIANGLES

    @version  0.0.1

*/
enum class EDrawMode {
    LINES,
    TRIANGLES,
};

/*!

    @brief Draw commands for the renderer queue


    @version  0.0.1

*/
enum class EDrawCommand { MODEL, MESH, TEXT, ENVIRONMENT };


struct Tokens {
    std::string text;
    bool is_emoji;
};


/*!

    @brief Font class
    - Get size of the text
    - Get the underlying TTF_Font*


    @version  0.0.1
    @param string path The font path
*/
class Font {
public:
    Font(TTF_Font* font) : _font(font) {
    }

    ~Font();

    glm::vec2 get_size(const std::string& text);
    TTF_Font* get_font() const;

protected:
    TTF_Font* _font = nullptr;
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

struct Metallic {
    glm::vec3 specular = glm::vec3(0.f); /// specular reflections
    float value        = 0.0f; /// 0.0 -> non-metal | 1.0 -> metal
};

struct Material {
    glm::vec3 albedo  = glm::vec3(1.f);
    Metallic metallic = {};
    float roughness   = 0.0f; /// 0.0 -> mirror | 1.0 -> blurs
};


struct Vertex {
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 uv;
};

constexpr int MAX_BONES = 200;


/*!
    @brief Bone structure for skeletal animation
    - Stores offset matrix (inverse bind pose) and final transform
    - Used in GPU skinning for animated meshes

    @version 0.0.1
*/
struct Bone {
    std::string name;
    glm::mat4 offset_matrix; // Inverse bind pose matrix
    glm::mat4 final_transform; // Final transform to upload to GPU

    Bone() : offset_matrix(1.0f), final_transform(1.0f) {
    }
};

/*!

    @brief Mesh Abstract class
    - Bind the mesh
    - Draw the mesh
    - Unbind the mesh

    @version  0.0.1

*/
struct Mesh {
    std::string_view name = "UNNAMED_MESH";

    size_t vertex_count = 0;
    size_t index_count  = 0;

    std::vector<glm::vec3> vertices;
    std::vector<Uint32> indices;

    Material material;

    // Animation support
    bool has_bones = false;
    std::unordered_map<std::string, int> bone_map; // Bone name -> bone index
    std::vector<Bone> bones; // All bones in this mesh

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

    virtual void set_value(const std::string& name, const glm::mat4* values, Uint32 count) = 0;

    virtual void destroy() = 0;

    virtual Uint32 get_id() const = 0;

    virtual bool is_valid() const = 0;


protected:
    Uint32 id = 0;

    std::unordered_map<std::string, Uint32> _uniforms;
};
