#pragma once
#include "core/io/file_system.h"


enum class FramebufferTextureFormat {
    None = 0,
    RGBA8,
    RED_INTEGER,
    DEPTH24STENCIL8,
    DEPTH_COMPONENT
};

struct FramebufferTextureSpecification {
    FramebufferTextureFormat format = FramebufferTextureFormat::None;
};

struct FramebufferAttachmentSpecification {
    std::vector<FramebufferTextureSpecification> attachments;
    FramebufferAttachmentSpecification() = default;

    FramebufferAttachmentSpecification(const std::initializer_list<FramebufferTextureSpecification> list)
        : attachments(list) {
    }
};

struct FramebufferSpecification {
    unsigned int width  = 0;
    unsigned int height = 0;
    FramebufferAttachmentSpecification attachments;
    bool swap_chain_target = false;
};


class Framebuffer {
public:
    virtual ~Framebuffer() = default;

    virtual void bind() = 0;
    virtual void unbind() = 0;
    virtual void invalidate() = 0;

    virtual void resize(unsigned int width, unsigned int height) = 0;
    virtual uint32_t get_color_attachment_id(size_t index = 0) const = 0;
    virtual uint32_t get_depth_attachment_id() const = 0;

    virtual const FramebufferSpecification& get_specification() const = 0;
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
