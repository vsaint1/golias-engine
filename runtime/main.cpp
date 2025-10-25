#include "stdafx.h"
#include <SDL3/SDL_main.h>
#include  "core/renderer/opengl/ogl_struct.h"

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


class OpenGLFramebuffer final : public Framebuffer {
    Uint32 fbo = 0;
    FramebufferSpecification specification;
    std::vector<Uint32> color_attachments;
    uint32_t depth_attachment = 0;

public:
    OpenGLFramebuffer(const FramebufferSpecification& spec)
        : specification(spec) {
        spdlog::info("OpenGLFramebuffer::OpenGLFramebuffer - Creating Framebuffer ({}x{})", spec.width, spec.height);
        invalidate();
    }

    ~OpenGLFramebuffer() override {
        cleanup();
    }

    void invalidate() override {
        if (fbo)
            cleanup();

        spdlog::warn("OpenGLFramebuffer::invalidate - Recreating Framebuffer ({}x{})", specification.width, specification.height);
        glGenFramebuffers(1, &fbo);
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);

        bool hasDepthAttachment = false;

        for (auto& attachment : specification.attachments.attachments) {
            switch (attachment.format) {
            case FramebufferTextureFormat::RGBA8: {
                uint32_t tex;
                glGenTextures(1, &tex);
                glBindTexture(GL_TEXTURE_2D, tex);
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, specification.width, specification.height, 0,
                             GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + color_attachments.size(),
                                       GL_TEXTURE_2D, tex, 0);


                color_attachments.push_back(tex);
                break;
            }
            case FramebufferTextureFormat::DEPTH_COMPONENT:
            case FramebufferTextureFormat::DEPTH24STENCIL8: {
                hasDepthAttachment = true;
                glGenTextures(1, &depth_attachment);
                glBindTexture(GL_TEXTURE_2D, depth_attachment);
                glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, specification.width, specification.height,
                             0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
                glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depth_attachment, 0);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

                break;
            }
            default:
                break;
            }
        }

        if (color_attachments.empty()) {
            glDrawBuffers(0, nullptr);
            glReadBuffer(GL_NONE);
        } else {
            GLenum buffers[4] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3};
            glDrawBuffers((GLsizei) color_attachments.size(), buffers);
        }

        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
            std::cerr << "Framebuffer incomplete!" << std::endl;
        }

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    void bind() override {
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);
        glViewport(0, 0, specification.width, specification.height);
    }

    void unbind() override {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    void resize(unsigned int width, unsigned int height) override {
        specification.width  = width;
        specification.height = height;
        invalidate();
    }

    Uint32 get_color_attachment_id(size_t index = 0) const override {
        return color_attachments[index];
    }

    Uint32 get_depth_attachment_id() const override {
        return depth_attachment;
    }

    const FramebufferSpecification& get_specification() const override {
        return specification;
    }

    void cleanup() {
        if (depth_attachment)
            glDeleteTextures(1, &depth_attachment);

        if (!color_attachments.empty())
            glDeleteTextures((GLsizei) color_attachments.size(), color_attachments.data());

        if (fbo)
            glDeleteFramebuffers(1, &fbo);
    }
};

// ============================================================================
// COMPONENTS
// ============================================================================

struct Transform {
    glm::vec3 position{0.0f};
    glm::vec3 rotation{0.0f};
    glm::vec3 scale{1.0f};

    glm::mat4 get_matrix() const {
        glm::mat4 mat = glm::translate(glm::mat4(1.0f), position);
        mat           = glm::rotate(mat, rotation.x, glm::vec3(1, 0, 0));
        mat           = glm::rotate(mat, rotation.y, glm::vec3(0, 1, 0));
        mat           = glm::rotate(mat, rotation.z, glm::vec3(0, 0, 1));
        mat           = glm::scale(mat, scale);
        return mat;
    }
};

struct MeshRenderer {
    Uint32 VAO     = 0;
    Uint32 VBO     = 0;
    Uint32 EBO     = 0;
    int indexCount = 0;
};

struct Material {
    glm::vec3 albedo = glm::vec3(1.0f);
    float metallic   = 0.0f;
    float roughness  = 0.5f;
    float ao         = 1.0f;

    // Emissive properties
    glm::vec3 emissive     = glm::vec3(0.0f);
    float emissiveStrength = 1.0f;

    Uint32 albedoMap    = 0;
    Uint32 metallicMap  = 0;
    Uint32 roughnessMap = 0;
    Uint32 normalMap    = 0;
    Uint32 aoMap        = 0;
    Uint32 emissiveMap  = 0;

    bool useAlbedoMap    = false;
    bool useMetallicMap  = false;
    bool useRoughnessMap = false;
    bool useNormalMap    = false;
    bool useAOMap        = false;
    bool useEmissiveMap  = false;
};

struct Camera {
    glm::vec3 position = glm::vec3(0.0f);
    glm::vec3 front    = glm::vec3(0.0f, 0.0f, -1.0f);
    glm::vec3 up       = glm::vec3(0.0f, 1.0f, 0.0f);
    float fov          = 45.0f;
    float nearPlane    = 0.1f;
    float farPlane     = 1000.0f;

    // Camera movement
    float yaw              = -90.0f;
    float pitch            = 0.0f;
    float movementSpeed    = 5.0f;
    float mouseSensitivity = 0.1f;

    void update_vectors() {
        glm::vec3 direction;
        direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
        direction.y = sin(glm::radians(pitch));
        direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
        front       = glm::normalize(direction);
    }

    glm::mat4 get_view_matrix() const {
        return glm::lookAt(position, position + front, up);
    }

    glm::mat4 get_projection_matrix(float aspect) const {
        return glm::perspective(glm::radians(fov), aspect, nearPlane, farPlane);
    }
};

struct DirectionalLight {
    glm::vec3 direction{0.0f, -1.0f, 0.0f};
    glm::vec3 color{1.0f};
    float intensity  = 1.0f;
    bool castShadows = true;

    // Shadow projection
    float shadowDistance = 50.0f;
    float shadowNear     = 1.0f;
    float shadowFar      = 100.0f;

    glm::mat4 get_light_space_matrix(glm::mat4 camera_view, glm::mat4 camera_proj) const {
        glm::vec3 light_dir = glm::normalize(direction);

        glm::mat4 invCam = glm::inverse(camera_proj * camera_view);

        std::vector<glm::vec3> frustum_corners;
        for (int x = 0; x < 2; ++x)
            for (int y = 0; y < 2; ++y)
                for (int z = 0; z < 2; ++z) {
                    glm::vec4 corner = invCam * glm::vec4(
                                           2.0f * x - 1.0f,
                                           2.0f * y - 1.0f,
                                           2.0f * z - 1.0f,
                                           1.0f
                                           );
                    corner /= corner.w;
                    frustum_corners.push_back(glm::vec3(corner));
                }

        glm::vec3 scene_center(0.0f);
        for (auto& c : frustum_corners)
            scene_center += c;
        scene_center /= static_cast<float>(frustum_corners.size());

        glm::vec3 light_position = scene_center - light_dir * 500.0f;

        glm::mat4 lightView = glm::lookAt(light_position, scene_center, glm::vec3(0.0f, 0.0f, 1.0f));

        glm::vec3 min_bounds(FLT_MAX);
        glm::vec3 max_bounds(-FLT_MAX);
        for (auto& corner : frustum_corners) {
            auto ls    = glm::vec3(lightView * glm::vec4(corner, 1.0f));
            min_bounds = glm::min(min_bounds, ls);
            max_bounds = glm::max(max_bounds, ls);
        }

        constexpr float extend = 200.f;
        min_bounds.z -= extend;
        max_bounds.z += extend;

        glm::mat4 orthoProj = glm::ortho(
            min_bounds.x, max_bounds.x,
            min_bounds.y, max_bounds.y,
            -max_bounds.z, -min_bounds.z
            );

        return orthoProj * lightView;
    }

    glm::mat4 get_light_space_matrix() const {
        glm::mat4 lightProjection = glm::ortho(-shadowDistance, shadowDistance,
                                               -shadowDistance, shadowDistance,
                                               shadowNear, shadowFar);
        glm::vec3 lightPos  = -direction * (shadowDistance * 0.5f);
        glm::mat4 lightView = glm::lookAt(lightPos, glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        return lightProjection * lightView;
    }
};

struct SpotLight {
    glm::vec3 direction{0.0f, -1.0f, 0.0f};
    glm::vec3 color{1.0f};
    float intensity   = 1.0f;
    float cutOff      = 12.5f;
    float outerCutOff = 17.5f;
};

// ============================================================================
// ABSTRACT RENDERER
// ============================================================================
class Renderer {
public:
    virtual ~Renderer() = default;

    virtual bool initialize(int width, int height) = 0;
    virtual void resize(int width, int height) = 0;
    virtual void cleanup() = 0;

    // Texture loading
    virtual GLuint load_texture_from_file(const std::string& path) = 0;
    virtual GLuint load_texture_from_memory(const unsigned char* buffer, size_t size, const std::string& name = "") = 0;
    virtual GLuint load_texture_from_raw_data(const unsigned char* data, int width, int height, int channels = 4,
                                              const std::string& name                                        = "") = 0;

    // Shadow pass
    virtual void begin_shadow_pass() = 0;
    virtual void render_shadow_pass(const Transform& transform, const MeshRenderer& mesh, const glm::mat4& lightSpaceMatrix) = 0;
    virtual void end_shadow_pass() = 0;

    // Main render pass - REFACTORED to use light classes
    virtual void begin_render_target() = 0;
    virtual void render_entity(const Transform& transform,
                               const MeshRenderer& mesh,
                               const Material& material,
                               const Camera& camera,
                               const glm::mat4& lightSpaceMatrix,
                               const std::vector<DirectionalLight>& directionalLights,
                               const std::vector<std::pair<Transform, SpotLight>>& spotLights) = 0;
    virtual void end_render_target() = 0;

protected:
    std::unique_ptr<Shader> _default_shader     = nullptr;
    std::unique_ptr<Shader> _shadow_shader      = nullptr;
    std::shared_ptr<Framebuffer> shadow_map_fbo = nullptr;
    std::unordered_map<std::string, Uint32> _textures;
};


class OpenGLRenderer final : public Renderer {
    int width              = 0, height = 0;
    SDL_GLContext _context = nullptr;

    static GLuint create_gl_texture(const unsigned char* data, int w, int h, int channels) {
        GLuint texID = 0;
        glGenTextures(1, &texID);
        glBindTexture(GL_TEXTURE_2D, texID);

        GLenum format = GL_RGB;
        if (channels == 1)
            format = GL_RED;
        else if (channels == 3)
            format = GL_RGB;
        else if (channels == 4)
            format = GL_RGBA;

        glTexImage2D(GL_TEXTURE_2D, 0, format, w, h, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        return texID;
    }

public:
    ~OpenGLRenderer() override {
        OpenGLRenderer::cleanup();
    }

    bool initialize(int w, int h) override {
        width  = w;
        height = h;

        if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(SDL_GL_GetProcAddress))) {
            std::cerr << "Failed to initialize GLAD" << std::endl;
            return false;
        }

        glEnable(GL_DEPTH_TEST);
        glViewport(0, 0, width, height);

        _default_shader = std::make_unique<OpenglShader>("shaders/opengl/default.vert", "shaders/opengl/default.frag");
        _shadow_shader  = std::make_unique<OpenglShader>("shaders/opengl/shadow.vert", "shaders/opengl/shadow.frag");

        FramebufferSpecification spec;
        spec.width       = 8192;
        spec.height      = 8192;
        spec.attachments = {
            {FramebufferTextureFormat::DEPTH_COMPONENT}
        };

        shadow_map_fbo = std::make_shared<OpenGLFramebuffer>(spec);

        return true;
    }

    GLuint load_texture_from_file(const std::string& path) override {
        if (auto it = _textures.find(path); it != _textures.end())
            return it->second;

        int w, h, channels;
        unsigned char* data = stbi_load(path.c_str(), &w, &h, &channels, 0);
        if (!data) {
            spdlog::error("Failed to load texture: {}", path);
            return 0;
        }

        GLuint texID = create_gl_texture(data, w, h, channels);
        stbi_image_free(data);

        _textures[path] = texID;
        spdlog::info("Loaded Texture: {}", path);
        return texID;
    }

    GLuint load_texture_from_memory(const unsigned char* buffer, size_t size, const std::string& name = "") override {
        std::string key = name.empty() ? "embedded_tex_" + std::to_string(reinterpret_cast<size_t>(buffer)) : name;

        if (auto it = _textures.find(key); it != _textures.end())
            return it->second;

        int w, h, channels;
        unsigned char* data = stbi_load_from_memory(buffer, (int) size, &w, &h, &channels, 0);
        if (!data) {
            spdlog::error("Failed to load texture from memory: {}", key);
            return 0;
        }

        GLuint texID = create_gl_texture(data, w, h, channels);
        stbi_image_free(data);

        _textures[key] = texID;
        spdlog::info("Loaded embedded Texture: {}, Path {}", texID, key);
        return texID;
    }

    GLuint load_texture_from_raw_data(const unsigned char* data, int w, int h, int channels = 4, const std::string& name = "") override {
        std::string key = name.empty() ? "raw_" + std::to_string(reinterpret_cast<size_t>(data)) : name;

        if (auto it = _textures.find(key); it != _textures.end())
            return it->second;

        GLuint texID   = create_gl_texture(data, w, h, channels);
        _textures[key] = texID;
        spdlog::info("Loaded raw Texture: {}, Path {}", texID, key);

        return texID;
    }

    void begin_shadow_pass() override {
        shadow_map_fbo->bind();
        glEnable(GL_DEPTH_TEST);
        glClear(GL_DEPTH_BUFFER_BIT);
        _shadow_shader->activate();
    }

    void render_shadow_pass(const Transform& transform, const MeshRenderer& mesh, const glm::mat4& lightSpaceMatrix) override {
        glm::mat4 model = transform.get_matrix();

        _shadow_shader->set_value("lightSpaceMatrix", lightSpaceMatrix, 1);
        _shadow_shader->set_value("model", model, 1);

        glBindVertexArray(mesh.VAO);
        glDrawElements(GL_TRIANGLES, mesh.indexCount, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
    }

    void end_shadow_pass() override {
        shadow_map_fbo->unbind();
        glCullFace(GL_BACK);
        glViewport(0, 0, width, height);
    }

    void begin_render_target() override {
        glClearColor(0.1f, 0.1f, 0.15f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        _default_shader->activate();
        glViewport(0, 0, width, height);
    }

    void render_entity(const Transform& transform,
                       const MeshRenderer& mesh,
                       const Material& material,
                       const Camera& camera,
                       const glm::mat4& lightSpaceMatrix,
                       const std::vector<DirectionalLight>& directionalLights,
                       const std::vector<std::pair<Transform, SpotLight>>& spotLights) override {
        glm::mat4 model      = transform.get_matrix();
        glm::mat4 view       = camera.get_view_matrix();
        glm::mat4 projection = camera.get_projection_matrix(static_cast<float>(width) / height);

        _default_shader->set_value("model", model);
        _default_shader->set_value("view", view);
        _default_shader->set_value("projection", projection);
        _default_shader->set_value("lightSpaceMatrix", lightSpaceMatrix);

        // Camera
        _default_shader->set_value("camPos", camera.position);

        // Material
        _default_shader->set_value("albedo", material.albedo);
        _default_shader->set_value("metallic", material.metallic);
        _default_shader->set_value("roughness", material.roughness);
        _default_shader->set_value("ao", material.ao);
        _default_shader->set_value("emissive", material.emissive);
        _default_shader->set_value("emissiveStrength", material.emissiveStrength);

        // Texture usage flags
        _default_shader->set_value("useAlbedoMap", material.useAlbedoMap);
        _default_shader->set_value("useMetallicMap", material.useMetallicMap);
        _default_shader->set_value("useRoughnessMap", material.useRoughnessMap);
        _default_shader->set_value("useNormalMap", material.useNormalMap);
        _default_shader->set_value("useAOMap", material.useAOMap);
        _default_shader->set_value("useEmissiveMap", material.useEmissiveMap);

        // Texture bindings
        if (material.useAlbedoMap && material.albedoMap) {
            glActiveTexture(GL_TEXTURE0 + ALBEDO_TEXTURE_UNIT);
            glBindTexture(GL_TEXTURE_2D, material.albedoMap);
            _default_shader->set_value("albedoMap", ALBEDO_TEXTURE_UNIT);
        }

        if (material.useMetallicMap && material.metallicMap) {
            glActiveTexture(GL_TEXTURE0 + METALLIC_ROUGHNESS_TEXTURE_UNIT);
            glBindTexture(GL_TEXTURE_2D, material.metallicMap);
            _default_shader->set_value("metallicMap", METALLIC_ROUGHNESS_TEXTURE_UNIT);
        }

        if (material.useRoughnessMap && material.roughnessMap) {
            glActiveTexture(GL_TEXTURE0 + ROUGHNESS_TEXTURE_UNIT);
            glBindTexture(GL_TEXTURE_2D, material.roughnessMap);
            _default_shader->set_value("roughnessMap", ROUGHNESS_TEXTURE_UNIT);
        }

        if (material.useNormalMap && material.normalMap) {
            glActiveTexture(GL_TEXTURE0 + NORMAL_MAP_TEXTURE_UNIT);
            glBindTexture(GL_TEXTURE_2D, material.normalMap);
            _default_shader->set_value("normalMap", NORMAL_MAP_TEXTURE_UNIT);
        }

        if (material.useAOMap && material.aoMap) {
            glActiveTexture(GL_TEXTURE0 + AMBIENT_OCCLUSION_TEXTURE_UNIT);
            glBindTexture(GL_TEXTURE_2D, material.aoMap);
            _default_shader->set_value("aoMap", AMBIENT_OCCLUSION_TEXTURE_UNIT);
        }

        if (material.useEmissiveMap && material.emissiveMap) {
            glActiveTexture(GL_TEXTURE0 + EMISSIVE_TEXTURE_UNIT);
            glBindTexture(GL_TEXTURE_2D, material.emissiveMap);
            _default_shader->set_value("emissiveMap", EMISSIVE_TEXTURE_UNIT);
        }

        // REFACTORED: Directional lights using DirectionalLight class
        _default_shader->set_value("numDirLights", static_cast<int>(directionalLights.size()));
        if (!directionalLights.empty()) {
            std::vector<glm::vec3> directions;
            std::vector<glm::vec3> colors;
            std::vector<int> castShadows;

            for (const auto& light : directionalLights) {
                directions.push_back(light.direction);
                colors.push_back(light.color * light.intensity);
                castShadows.push_back(light.castShadows ? 1 : 0);
            }

            for (int i = 0; i < static_cast<int>(directionalLights.size()); ++i) {
                _default_shader->set_value(fmt::format("dirLights[{}].direction", i), directions[i]);
                _default_shader->set_value(fmt::format("dirLights[{}].color", i), colors[i]);
                _default_shader->set_value(fmt::format("dirLights[{}].cast_shadows", i), castShadows[i]);
            }

        }

        // REFACTORED: Spot lights using SpotLight class with Transform
        _default_shader->set_value("numSpotLights", static_cast<int>(spotLights.size()));
        if (!spotLights.empty()) {
            std::vector<glm::vec3> positions;
            std::vector<glm::vec3> directions;
            std::vector<glm::vec3> colors;
            std::vector<float> cutOffs;
            std::vector<float> outerCutOffs;

            for (const auto& [transform, light] : spotLights) {
                positions.push_back(transform.position);
                directions.push_back(light.direction);
                colors.push_back(light.color * light.intensity);
                cutOffs.push_back(glm::cos(glm::radians(light.cutOff)));
                outerCutOffs.push_back(glm::cos(glm::radians(light.outerCutOff)));
            }

            for (int i = 0; i < static_cast<int>(spotLights.size()); ++i) {
                _default_shader->set_value(fmt::format("spotLights[{}].position", i), positions[i]);
                _default_shader->set_value(fmt::format("spotLights[{}].direction", i), directions[i]);
                _default_shader->set_value(fmt::format("spotLights[{}].color", i), colors[i]);
                _default_shader->set_value(fmt::format("spotLights[{}].inner_cut_off", i), cutOffs[i]);
                _default_shader->set_value(fmt::format("spotLights[{}].outer_cut_off", i), outerCutOffs[i]);
            }

        }

        // Shadow map
        glActiveTexture(GL_TEXTURE0 + SHADOW_TEXTURE_UNIT);
        glBindTexture(GL_TEXTURE_2D, shadow_map_fbo->get_depth_attachment_id());
        _default_shader->set_value("shadowMap", SHADOW_TEXTURE_UNIT);

        glBindVertexArray(mesh.VAO);
        glDrawElements(GL_TRIANGLES, mesh.indexCount, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
    }

    void end_render_target() override {
    }

    void resize(int w, int h) override {
        width  = w;
        height = h;
        glViewport(0, 0, width, height);
    }

    void cleanup() override {
        // Clean up textures
        for (auto& [key, texID] : _textures)
            glDeleteTextures(1, &texID);
        _textures.clear();

        _default_shader->destroy();
        _shadow_shader->destroy();
    }
};


// ============================================================================
// MODEL (combines mesh + material)
// ============================================================================

struct Model {
    std::vector<MeshRenderer> meshes;
    std::vector<Material> materials;
};


class ObjectLoader {
public:
    static MeshRenderer load_mesh(const std::string& path) {
        Model model = load_model(path, nullptr);
        return model.meshes.empty() ? MeshRenderer() : model.meshes[0];
    }

    static Model load_model(const std::string& path, Renderer* renderer = nullptr) {
        Model model;

        const char* extension = strrchr(path.c_str(), '.');

        spdlog::info("Loading Model Path: {}, FileFormat: {}", path, extension ? extension + 1 : "UNKNOWN");

        Assimp::Importer importer;

        constexpr unsigned int ASSIMP_FLAGS =
            aiProcess_Triangulate |
            aiProcess_FlipUVs |
            aiProcess_CalcTangentSpace |
            aiProcess_GenNormals;

        const aiScene* scene = importer.ReadFile(path, ASSIMP_FLAGS);
        if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
            spdlog::error("Failed to load model: {}, Error: ", path, importer.GetErrorString());
            return model;
        }

        const std::string directory = get_directory(path);
        spdlog::info("  Meshes: {}, Materials: {}, Animations: {}",
                     scene->mNumMeshes, scene->mNumMaterials, scene->mNumAnimations);

        for (unsigned int i = 0; i < scene->mNumMeshes; ++i) {
            aiMesh* aiMesh = scene->mMeshes[i];
            spdlog::info("  Parsing Mesh({}) - Name {}", i, aiMesh->mName.C_Str());

            MeshRenderer mesh = create_mesh(aiMesh);
            model.meshes.push_back(mesh);

            if (renderer) {
                Material material = load_material(scene, aiMesh, directory, *renderer);
                model.materials.push_back(material);
                spdlog::info("    Material [{}]: Albedo ({:.2f},{:.2f},{:.2f}) | Metallic {:.2f} | Roughness {:.2f} | AO {:.2f}",
                             aiMesh->mName.C_Str(), material.albedo.r, material.albedo.g, material.albedo.b,
                             material.metallic, material.roughness, material.ao);
            }

            if (aiMesh->HasBones()) {
                parse_bones(aiMesh, model);
            }
        }

        if (scene->HasAnimations()) {
            parse_animations(scene, model);
        }

        return model;
    }

private:
    static std::string get_directory(const std::string& path) {
        size_t found = path.find_last_of("/\\");
        return (found != std::string::npos) ? path.substr(0, found + 1) : "";
    }


    static MeshRenderer create_mesh(aiMesh* aiMesh) {
        std::vector<Vertex> vertices;
        std::vector<unsigned int> indices;
        vertices.reserve(aiMesh->mNumVertices);

        for (unsigned int i = 0; i < aiMesh->mNumVertices; ++i) {
            Vertex vertex;

            // Position
            vertex.position = {
                aiMesh->mVertices[i].x,
                aiMesh->mVertices[i].y,
                aiMesh->mVertices[i].z
            };

            // Normal
            if (aiMesh->HasNormals()) {
                vertex.normal = {
                    aiMesh->mNormals[i].x,
                    aiMesh->mNormals[i].y,
                    aiMesh->mNormals[i].z
                };
            } else {
                vertex.normal = {0.0f, 0.0f, 0.0f};
            }

            // UV
            if (aiMesh->mTextureCoords[0]) {
                vertex.uv = {
                    aiMesh->mTextureCoords[0][i].x,
                    aiMesh->mTextureCoords[0][i].y
                };
            } else {
                vertex.uv = {0.0f, 0.0f};
            }

            vertices.push_back(vertex);
        }

        // Process indices
        for (unsigned int i = 0; i < aiMesh->mNumFaces; ++i) {
            const aiFace& face = aiMesh->mFaces[i];
            indices.insert(indices.end(), face.mIndices, face.mIndices + face.mNumIndices);
        }

        MeshRenderer mesh;
        mesh.indexCount = indices.size();

        glGenVertexArrays(1, &mesh.VAO);
        glGenBuffers(1, &mesh.VBO);
        glGenBuffers(1, &mesh.EBO);

        glBindVertexArray(mesh.VAO);

        glBindBuffer(GL_ARRAY_BUFFER, mesh.VBO);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

        // Vertex attributes
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*) offsetof(Vertex, position));
        glEnableVertexAttribArray(0);

        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*) offsetof(Vertex, normal));
        glEnableVertexAttribArray(1);

        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*) offsetof(Vertex, uv));
        glEnableVertexAttribArray(2);

        glBindVertexArray(0);

        spdlog::info("  Mesh created: {} vertices, {} triangles", aiMesh->mNumVertices, indices.size() / 3);
        return mesh;
    }

    static Material load_material(const aiScene* scene, aiMesh* mesh,
                                  const std::string& directory, Renderer& renderer) {
        Material material;
        if (scene->mNumMaterials <= mesh->mMaterialIndex)
            return material;

        aiMaterial* aiMat = scene->mMaterials[mesh->mMaterialIndex];
        load_colors(aiMat, material);
        load_textures(aiMat, scene, directory, renderer, material);
        return material;
    }

    static void load_colors(aiMaterial* aiMat, Material& mat) {
        aiColor3D color(1, 1, 1);
        aiMat->Get(AI_MATKEY_COLOR_DIFFUSE, color);
        mat.albedo = {color.r, color.g, color.b};

        aiColor3D emissive(0, 0, 0);
        if (aiMat->Get(AI_MATKEY_COLOR_EMISSIVE, emissive) == AI_SUCCESS)
            mat.emissive = {emissive.r, emissive.g, emissive.b};

        aiMat->Get(AI_MATKEY_EMISSIVE_INTENSITY, mat.emissiveStrength);
        aiMat->Get(AI_MATKEY_METALLIC_FACTOR, mat.metallic);
        aiMat->Get(AI_MATKEY_ROUGHNESS_FACTOR, mat.roughness);
    }

    static void load_textures(aiMaterial* aiMat, const aiScene* scene,
                              const std::string& dir, Renderer& renderer, Material& mat) {
        auto load_tex = [&](aiTextureType type, GLuint& id, bool& flag, const char* name) {
            if (aiMat->GetTextureCount(type) == 0)
                return;

            aiString texPath;
            aiMat->GetTexture(type, 0, &texPath);
            std::string texStr = texPath.C_Str();

            if (texStr.empty())
                return;

            if (texStr[0] == '*') {
                int texIndex = std::atoi(texStr.c_str() + 1);
                if (texIndex >= 0 && texIndex < (int) scene->mNumTextures) {
                    const aiTexture* embedded = scene->mTextures[texIndex];
                    if (!embedded)
                        return;

                    if (embedded->mHeight == 0) {
                        id = renderer.load_texture_from_memory(
                            reinterpret_cast<const unsigned char*>(embedded->pcData),
                            embedded->mWidth);
                    } else {
                        id = renderer.load_texture_from_raw_data(
                            reinterpret_cast<const unsigned char*>(embedded->pcData),
                            embedded->mWidth, embedded->mHeight);
                    }

                    flag = (id != 0);
                    if (flag)
                        spdlog::info("    Embedded Texture loaded: {}", name);
                    return;
                }
            }

            std::string full = dir + "/" + texStr;
            id               = renderer.load_texture_from_file(full);
            flag             = (id != 0);
            if (flag)
                spdlog::info("    Texture loaded [{}]: {}", name, full);
        };

        load_tex(aiTextureType_DIFFUSE, mat.albedoMap, mat.useAlbedoMap, "albedo");
        load_tex(aiTextureType_NORMALS, mat.normalMap, mat.useNormalMap, "normal");
        load_tex(aiTextureType_METALNESS, mat.metallicMap, mat.useMetallicMap, "metallic");
        load_tex(aiTextureType_DIFFUSE_ROUGHNESS, mat.roughnessMap, mat.useRoughnessMap, "roughness");
        load_tex(aiTextureType_AMBIENT_OCCLUSION, mat.aoMap, mat.useAOMap, "ao");
        load_tex(aiTextureType_EMISSIVE, mat.emissiveMap, mat.useEmissiveMap, "emissive");
    }

    static void parse_bones(aiMesh* mesh, Model& model) {
        spdlog::info("    Bones: {}", mesh->mNumBones);
        for (unsigned int i = 0; i < mesh->mNumBones; ++i) {
            aiBone* bone = mesh->mBones[i];
            spdlog::info("      - Bone {}: {}", i, bone->mName.C_Str());
        }
    }


    static void parse_animations(const aiScene* scene, Model& model) {
        spdlog::info("  → Animations: {}", scene->mNumAnimations);
        for (unsigned int i = 0; i < scene->mNumAnimations; ++i) {
            const aiAnimation* anim = scene->mAnimations[i];
            spdlog::info("    Animation {}: {} | Duration: {:.2f} | FPS: {}",
                         i, anim->mName.C_Str(), anim->mDuration, anim->mTicksPerSecond);
        }
    }
};


// ============================================================================
// ENGINE
// ============================================================================

class Engine {
    SDL_Window* window      = nullptr;
    SDL_GLContext glContext = nullptr;
    bool running            = false;
    int width, height;

    std::unique_ptr<Renderer> renderer;
    flecs::world ecs;

    // Input state
    bool keys[SDL_SCANCODE_COUNT] = {false};
    bool firstMouse               = true;
    float lastX                   = 0.0f;
    float lastY                   = 0.0f;
    bool mouseCaptured            = false;

public:
    Engine(const std::string& title, int w, int h) : width(w), height(h) {
        if (!SDL_Init(SDL_INIT_VIDEO)) {
            spdlog::error("SDL initialization failed: {}", SDL_GetError());
            return;
        }

        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
        SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
        SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

        window = SDL_CreateWindow(
            title.c_str(),
            width, height,
            SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE
            );

        if (!window) {
            spdlog::error("Window creation failed: {}", SDL_GetError());
            SDL_Quit();
            return;
        }

        glContext = SDL_GL_CreateContext(window);
        if (!glContext) {
            spdlog::error("OpenGL context creation failed: {}", SDL_GetError());
            SDL_DestroyWindow(window);
            SDL_Quit();
            return;
        }

        SDL_GL_SetSwapInterval(0);

        renderer = std::make_unique<OpenGLRenderer>();
        if (!renderer->initialize(width, height)) {
            spdlog::error("Renderer initialization failed: UNKNOWN");
            cleanup();
            return;
        }

        lastX = width / 2.0f;
        lastY = height / 2.0f;

        running = true;
    }

    void handle_events() {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
            case SDL_EVENT_QUIT:
                running = false;
                break;

            case SDL_EVENT_WINDOW_RESIZED:
                width = event.window.data1;
                height = event.window.data2;
                renderer->resize(width, height);
                break;

            case SDL_EVENT_KEY_DOWN:
                if (event.key.scancode < SDL_SCANCODE_COUNT) {
                    keys[event.key.scancode] = true;
                }
                if (event.key.scancode == SDL_SCANCODE_ESCAPE) {
                    mouseCaptured = !mouseCaptured;
                    SDL_SetWindowRelativeMouseMode(window, mouseCaptured);
                    firstMouse = true;
                }
                break;

            case SDL_EVENT_KEY_UP:
                if (event.key.scancode < SDL_SCANCODE_COUNT) {
                    keys[event.key.scancode] = false;
                }
                break;

            case SDL_EVENT_MOUSE_MOTION:
                if (mouseCaptured) {
                    float xpos = event.motion.x;
                    float ypos = event.motion.y;

                    if (firstMouse) {
                        lastX      = xpos;
                        lastY      = ypos;
                        firstMouse = false;
                    }

                    float xoffset = event.motion.xrel;
                    float yoffset = -event.motion.yrel;

                    ecs.each([&](Camera& cam) {
                        cam.yaw += xoffset * cam.mouseSensitivity;
                        cam.pitch += yoffset * cam.mouseSensitivity;

                        if (cam.pitch > 89.0f)
                            cam.pitch = 89.0f;
                        if (cam.pitch < -89.0f)
                            cam.pitch = -89.0f;

                        cam.update_vectors();
                    });
                }
                break;

            case SDL_EVENT_MOUSE_BUTTON_DOWN:
                if (event.button.button == SDL_BUTTON_LEFT && !mouseCaptured) {
                    mouseCaptured = true;
                    SDL_SetWindowRelativeMouseMode(window, true);
                    firstMouse = true;
                }
                break;
            default: ;
            }
        }
    }

    void process_camera_movement(float deltaTime) {
        ecs.each([&](Camera& cam) {
            float velocity = cam.movementSpeed * deltaTime;

            if (keys[SDL_SCANCODE_W])
                cam.position += cam.front * velocity;
            if (keys[SDL_SCANCODE_S])
                cam.position -= cam.front * velocity;
            if (keys[SDL_SCANCODE_A])
                cam.position -= glm::normalize(glm::cross(cam.front, cam.up)) * velocity;
            if (keys[SDL_SCANCODE_D])
                cam.position += glm::normalize(glm::cross(cam.front, cam.up)) * velocity;
            if (keys[SDL_SCANCODE_SPACE])
                cam.position += cam.up * velocity;
            if (keys[SDL_SCANCODE_LCTRL] || keys[SDL_SCANCODE_RCTRL])
                cam.position -= cam.up * velocity;

            if (keys[SDL_SCANCODE_LSHIFT] || keys[SDL_SCANCODE_RSHIFT])
                cam.movementSpeed = 500.0f;
            else
                cam.movementSpeed = 100.0f;
        });
    }

    void update(float deltaTime) {
        process_camera_movement(deltaTime);
        ecs.progress(deltaTime);
    }

    void render() {
        std::vector<DirectionalLight> directionalLights;
        glm::mat4 lightSpaceMatrix(1.0f);

        Camera mainCamera;
        ecs.each([&](const Camera& cam) {
            mainCamera = cam;
        });

        ecs.each([&](flecs::entity e, Transform& t, DirectionalLight& light) {
            directionalLights.push_back(light);

            if (light.castShadows && lightSpaceMatrix == glm::mat4(1.0f)) {
                lightSpaceMatrix = light.get_light_space_matrix();
            }
        });

        std::vector<std::pair<Transform, SpotLight>> spotLights;
        ecs.each([&](flecs::entity e, Transform& t, SpotLight& light) {
            spotLights.push_back({t, light});
        });

        // Shadow pass
        renderer->begin_shadow_pass();
        ecs.each([&](Transform& t, MeshRenderer& mesh, Material& mat) {
            renderer->render_shadow_pass(t, mesh, lightSpaceMatrix);
        });
        renderer->end_shadow_pass();

        // Main render pass
        renderer->begin_render_target();
        ecs.each([&](Transform& t, MeshRenderer& mesh, Material& mat) {
            renderer->render_entity(t, mesh, mat, mainCamera, lightSpaceMatrix,
                                    directionalLights, spotLights);
        });
        renderer->end_render_target();

        SDL_GL_SwapWindow(window);
    }

    void run() {
        uint64_t lastTime = SDL_GetPerformanceCounter();

        while (running) {
            uint64_t currentTime = SDL_GetPerformanceCounter();
            float deltaTime      = (float) (currentTime - lastTime) / SDL_GetPerformanceFrequency();
            lastTime             = currentTime;

            handle_events();
            update(deltaTime);
            render();
        }
    }

    flecs::world& get_world() {
        return ecs;
    }

    Renderer* get_renderer() const {
        return renderer.get();
    }

    void cleanup() {
        if (renderer)
            renderer->cleanup();

        if (glContext)
            SDL_GL_DestroyContext(glContext);
        if (window)
            SDL_DestroyWindow(window);
        SDL_Quit();
    }

    ~Engine() {
        cleanup();
    }
};


int main(int argc, char* argv[]) {
    auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    console_sink->set_level(spdlog::level::debug);

    auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>("logs/output.log", true);
    file_sink->set_level(spdlog::level::info);

    std::vector<spdlog::sink_ptr> sinks{console_sink, file_sink};
    auto logger = std::make_shared<spdlog::logger>("GoliasEngine", sinks.begin(), sinks.end());

    spdlog::set_default_logger(logger);
    spdlog::set_level(spdlog::level::debug);
    spdlog::flush_on(spdlog::level::info);

    Engine engine("depressao gamer", 1280, 720);

    auto& ecs = engine.get_world();

    auto renderer = engine.get_renderer();

    // Create camera
    auto camera = ecs.entity()
                     .set(Camera{glm::vec3(0, 3, 10), glm::vec3(0, 0, -1), glm::vec3(0, 1, 0), 45.0f});

    // Create directional light (sun) with shadows
    auto dirLight = ecs.entity()
                       .set(Transform{})
                       .set(DirectionalLight{glm::vec3(1, -2.5, 1), glm::vec3(1.0f, 0.95f, 0.8f), 2.0f, true});

    // Create spot lights (no shadows)
    auto spotLight1 = ecs.entity()
                         .set(Transform{glm::vec3(5, 5, 5)})
                         .set(SpotLight{glm::vec3(-1, -1, -1), glm::vec3(1.0f, 0.3f, 0.3f), 30.0f, 12.5f, 17.5f});

    auto spotLight2 = ecs.entity()
                         .set(Transform{glm::vec3(-5, 5, 5)})
                         .set(SpotLight{glm::vec3(1, -1, -1), glm::vec3(0.3f, 0.3f, 1.0f), 50.0f, 12.5f, 17.5f});


    Model carModel = ObjectLoader::load_model("res/sprites/obj/Car2.obj", renderer);
    for (size_t i = 0; i < carModel.meshes.size(); i++) {
        ecs.entity(("Car_Mesh_" + std::to_string(i)).c_str())
           .set(Transform{glm::vec3(-10, 0, -5), glm::vec3(0), glm::vec3(1.0f)})
           .set(carModel.meshes[i])
           .set(carModel.materials[i]);
    }


    Model damagedHelmet = ObjectLoader::load_model("res/sprites/obj/DamagedHelmet.glb", renderer);
    for (size_t i = 0; i < damagedHelmet.meshes.size(); i++) {
        ecs.entity(("Helmet_Mesh_" + std::to_string(i)).c_str())
           .set(Transform{glm::vec3(15, 0, 0), glm::vec3(0), glm::vec3(1.0f)})
           .set(damagedHelmet.meshes[i])
           .set(damagedHelmet.materials[i]);
    }

    // Model sponza = MeshLoader::LoadModel("res/sprites/obj/sponza/sponza.glb", &texMgr);
    // for (size_t i = 0; i < sponza.meshes.size(); i++) {
    //     ecs.entity(("Sponza_Mesh_" + std::to_string(i)).c_str())
    //         .set(Transform{glm::vec3(0, 0, 0), glm::vec3(0), glm::vec3(1.0f)})
    //         .set(sponza.meshes[i])
    //         .set(sponza.materials[i]);
    // }

    Model medieval = ObjectLoader::load_model("res/sprites/obj/nagonford/Nagonford_Animated.glb", renderer);
    for (size_t i = 0; i < medieval.meshes.size(); i++) {
        ecs.entity(("Sponza_Mesh_" + std::to_string(i)).c_str())
           .set(Transform{glm::vec3(0, 0, 0), glm::vec3(0), glm::vec3(1.0f)})
           .set(medieval.meshes[i])
           .set(medieval.materials[i]);
    }

    MeshRenderer cylinderMesh = ObjectLoader::load_mesh("res/models/cylinder.obj");
    ecs.entity("Cylinder").set(Transform{glm::vec3(0, 0, -10), glm::vec3(0), glm::vec3(1.0f)}).set(cylinderMesh).set(Material{
        .albedo = glm::vec3(0, 0, 1.0f)});

    MeshRenderer torusMesh = ObjectLoader::load_mesh("res/models/torus.obj");
    ecs.entity("Torus").set(Transform{glm::vec3(0, 0, 5), glm::vec3(0), glm::vec3(1.0f)}).set(torusMesh).set(Material{
        .albedo = glm::vec3(0, 1.0f, 1.0), .emissive = glm::vec3(1, 1, 1), .emissiveStrength = 1.0f});

    MeshRenderer coneMesh = ObjectLoader::load_mesh("res/models/cone.obj");
    ecs.entity("Cone").set(Transform{glm::vec3(0, 0, 20), glm::vec3(0), glm::vec3(1.0f)}).set(coneMesh).set(Material{
        .albedo = glm::vec3(0, 1.0f, 1.0)});

    MeshRenderer blenderMonkeyMesh = ObjectLoader::load_mesh("res/models/blender_monkey.obj");
    ecs.entity("Cone").set(Transform{glm::vec3(0, 0, 10), glm::vec3(0), glm::vec3(1.0f)}).set(blenderMonkeyMesh).set(Material{
        .albedo = glm::vec3(1.0f, 0.0f, 1.0)});

    MeshRenderer cubeMesh = ObjectLoader::load_mesh("res/models/cube.obj");
    ecs.entity("Red Cube")
       .set(Transform{glm::vec3(3, 0, 0), glm::vec3(0), glm::vec3(1.5f)})
       .set(cubeMesh)
       .set(Material{glm::vec3(0.8f, 0.1f, 0.1f), 0.0f, 0.3f, 1.0f});

    MeshRenderer sphereMesh = ObjectLoader::load_mesh("res/models/sphere.obj");

    ecs.entity("Sphere")
       .set(Transform{glm::vec3(-3, 0, 0), glm::vec3(0), glm::vec3(1.5f)})
       .set(sphereMesh)
       .set(Material{glm::vec3(1.f), 0.0f, 0.1f, 1.0f});

    ecs.entity("Cube")
       .set(Transform{glm::vec3(3, 0, 0), glm::vec3(0), glm::vec3(1.f)})
       .set(cubeMesh)
       .set(Material{glm::vec3(1.f), 0.0f, 0.0f, 1.0f,});

    ecs.entity("Ground")
       .set(Transform{glm::vec3(0, -2, 0), glm::vec3(0), glm::vec3(20.0f, 0.1f, 20.0f)})
       .set(cubeMesh)
       .set(Material{glm::vec3(1.0f), 0.0f, 0.8f, 1.0f});

    engine.run();

    spdlog::shutdown();
    return 0;
}
