#pragma once
#include "core/io/assimp_io.h"
#include "core/io/assimp_io.h"
#include "core/io/assimp_io.h"
#include "core/io/assimp_io.h"
#include "core/io/assimp_io.h"
#include "core/io/assimp_io.h"
#include "core/io/assimp_io.h"
#include "core/io/assimp_io.h"
#include "core/io/assimp_io.h"
#include "core/io/assimp_io.h"
#include "core/renderer/renderer.h"
#include "core/renderer/base_struct.h"

class OpenglGpuVertexLayout final: public  GpuVertexLayout {
public:

    OpenglGpuVertexLayout(
      const GpuBuffer* vertex_buffer,
      const GpuBuffer* index_buffer,
      const std::vector<VertexAttribute>& attributes,
      uint32_t stride);

    ~OpenglGpuVertexLayout() override;

    void bind() const override;

    void unbind() const override;

private:
    GLuint _vao = 0;

    static GLenum to_gl_type(DataType type);
};

class OpenglGpuBuffer final : public GpuBuffer {

public:
    OpenglGpuBuffer(GpuBufferType type);

    ~OpenglGpuBuffer() override;

    void bind() const override;

    void upload(const void* data, size_t size) override;

    size_t size() const override;

    GpuBufferType type() const override;


private:
    GLuint _id                 = 0;
    GpuBufferType _buffer_type = GpuBufferType::VERTEX;
    size_t _buffer_size        = 0;
    GLenum _target             = GL_ARRAY_BUFFER;

};

class OpenGLFramebuffer final : public Framebuffer {

public:
    explicit OpenGLFramebuffer(const FramebufferSpecification& spec);

    ~OpenGLFramebuffer() override;

    void invalidate() override;

    void bind() override;

    void unbind() override;

    void resize(unsigned int width, unsigned int height) override;

    Uint32 get_fbo_id() const override;

    Uint32 get_color_attachment_id(size_t index = 0) const override;

    Uint32 get_depth_attachment_id() const override;

    const FramebufferSpecification& get_specification() const override;

    void cleanup();

private:
    Uint32 fbo = 0;
    FramebufferSpecification specification;
    std::vector<Uint32> color_attachments;
    uint32_t depth_attachment = 0;
};


class OpenglShader final : public Shader {
public:
    OpenglShader() = default;
    ~OpenglShader() override;

    template <typename T>
    T get_value(const char* name);

    OpenglShader(const std::string& vertex, const std::string& fragment);

    void activate() const override;

    void set_value(const char* name, float value) override;

    void set_value(const char* name, int value) override;

    void set_value(const char* name, Uint32 value) override;

    void set_value(const char* name, glm::mat4 value, Uint32 count) override;

    void set_value(const char* name, const int* value, Uint32 count) override;

    void set_value(const char* name, const float* value, Uint32 count) override;

    void set_value(const char* name, glm::vec2 value, Uint32 count) override;

    void set_value(const char* name, glm::vec3 value, Uint32 count) override;

    void set_value(const char* name, glm::vec4 value, Uint32 count) override;

    void set_value(const char* name, const glm::mat4* values, Uint32 count) override;

    void destroy() override;

    unsigned int get_id() const override;

    bool is_valid() const override;

private:
    Uint32 get_uniform_location(const std::string& name);

    Uint32 compile_shader(Uint32 type, const char* source);
};


template <typename T>
inline T OpenglShader::get_value(const char* name) {
    const unsigned int location = get_uniform_location(name);

    if (location == -1) {
        printf("Shader variable not found: %s\n", name);
        return T();
    }

    if constexpr (std::is_same_v<T, float>) {
        float value;
        glGetUniformfv(id, location, &value);
        return value;
    } else if constexpr (std::is_same_v<T, int>) {
        int value;
        glGetUniformiv(id, location, &value);
        return value;
    } else if constexpr (std::is_same_v<T, glm::vec2>) {
        GLfloat data[2];
        glGetUniformfv(id, location, data);
        return glm::vec2(data[0], data[1]);
    } else if constexpr (std::is_same_v<T, glm::vec3>) {
        GLfloat data[3];
        glGetUniformfv(id, location, data);
        return glm::vec3(data[0], data[1], data[2]);
    } else if constexpr (std::is_same_v<T, glm::vec4>) {
        GLfloat data[4];
        glGetUniformfv(id, location, data);
        return glm::vec4(data[0], data[1], data[2], data[3]);
    } else if constexpr (std::is_same_v<T, glm::mat4>) {
        GLfloat data[16];
        glGetUniformfv(id, location, data);
        return glm::make_mat4(data);
    } else {
        printf("Unsupported type: %s\n", typeid(T).name());
    }

    return T();
}
