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

    @brief Texture target

    @version  0.0.1

*/
enum class ETextureTarget {
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
    Uint32 id  = -1;
    int width  = 0;
    int height = 0;
    int pitch = 0; /// Number of bytes in a row of pixel data
    std::string_view path;
    void* pixels      = nullptr; /// Raw pixel data before uploading to GPU **MUST** be freed after upload


    Texture() = default;

    // todo: add more texture properties
    virtual void bind(Uint32 slot = 0) {
        SDL_Log("Texture::activate - Not implemented for this renderer");
    }

    virtual bool is_valid() const {
        return id != -1;
    }

    virtual ~Texture() = default;

    ETextureTarget target = ETextureTarget::TEXTURE_2D;
};

struct Metallic {
    glm::vec3 specular               = glm::vec3(0.f); /// specular reflections
    float value                      = 0.0f; /// 0.0 -> non-metal | 1.0 -> metal
    std::shared_ptr<Texture> texture = nullptr;
};

/*!

    @brief Material structure
    - Albedo texture
    - Albedo color
    - Metallic properties
    - Roughness

    @version 0.0.1

*/
struct Material {
    std::shared_ptr<Texture> albedo_texture = nullptr;
    glm::vec3 ambient                       = glm::vec3(0.f);
    glm::vec3 albedo                        = glm::vec3(1.f);
    Metallic metallic                       = {};

    float roughness       = 0.0f; /// 0.0 -> mirror | 1.0 -> blurs
    float dissolve        = 1.0f; /// 1.0 -> opaque | 0.0 -> transparent
    int illumination_mode = 0; // TODO: define illumination modes


    bool is_valid() const;

    // TODO: Additional textures (normal, metallic, roughness, etc.) and properties
    void bind();
};

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

constexpr int MAX_BONES = 250; /// I NEED TO UPGRADE OPENGL TO SUPPORT SSBO </3


/*!
    @brief Bone structure for skeletal animation
    - Stores offset matrix (inverse bind pose) and final transform
    - Used in GPU skinning for animated meshes

    @version 0.0.1
*/
struct Bone {
    std::string name;
    glm::mat4 offset_matrix   = glm::mat4(1.f); // Inverse bind pose matrix
    glm::mat4 final_transform = glm::mat4(1.f); // Final transform to upload to GPU

    Bone() = default;
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

    std::vector<Vertex> vertices;
    std::vector<Uint32> indices;

    std::unique_ptr<Material> material = std::make_unique<Material>();

    // Animation support
    bool has_bones = false;
    std::unordered_map<std::string, int> bone_map; // Bone name -> bone index
    std::vector<Bone> bones; // All bones in this mesh

    virtual void bind() = 0;

    virtual void upload_to_gpu() = 0;

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

// Forward declaration
class Renderer;

void parse_material(aiMesh* ai_mesh, const aiScene* scene, const std::string& base_dir, Mesh& mesh_ref);

void parse_meshes(aiMesh* ai_mesh, const aiScene* scene, const std::string& base_dir, Mesh& mesh_ref);

void parse_bones(aiMesh* ai_mesh,std::vector<glm::ivec4>& bone_ids, std::vector<glm::vec4>& bone_weights, Mesh& mesh_ref  );