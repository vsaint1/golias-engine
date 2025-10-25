#include "stdafx.h"
#include <SDL3/SDL_main.h>


class TextureManager {
    std::unordered_map<std::string, GLuint> textures;

    static GLuint create_gl_texture(const unsigned char* data, int width, int height, int channels) {
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

        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        return texID;
    }

public:
    TextureManager() = default;

    ~TextureManager() {
        cleanup();
    }


    GLuint load_texture_from_file(const std::string& path) {

        if (auto it = textures.find(path); it != textures.end())
            return it->second;

        int width, height, channels;
        unsigned char* data = stbi_load(path.c_str(), &width, &height, &channels, 0);
        if (!data) {
            spdlog::error("Failed to load texture: {}", path);
            return 0;
        }

        GLuint texID = create_gl_texture(data, width, height, channels);
        stbi_image_free(data);

        textures[path] = texID;
        spdlog::info("Loaded Texture: {}", path);
        return texID;
    }


    GLuint load_texture_from_memory(const unsigned char* buffer, size_t size, const std::string& name = "") {
        std::string key = name.empty() ? "embedded_tex_" + std::to_string(reinterpret_cast<size_t>(buffer)) : name;

        // Return cached if already loaded
        if (auto it = textures.find(key); it != textures.end())
            return it->second;

        int width, height, channels;
        unsigned char* data = stbi_load_from_memory(buffer, (int) size, &width, &height, &channels, 0);
        if (!data) {
            spdlog::error("Failed to load texture from memory: {}", key);
            return 0;
        }

        GLuint texID = create_gl_texture(data, width, height, channels);
        stbi_image_free(data);

        textures[key] = texID;
        spdlog::info("Loaded embedded Texture: {}, Path {}",texID, key);
        return texID;
    }


    GLuint load_texture_from_raw_data(const unsigned char* data, int width, int height, int channels = 4,
                                  const std::string& name                                        = "") {
        std::string key = name.empty() ? "raw_" + std::to_string(reinterpret_cast<size_t>(data)) : name;

        if (auto it = textures.find(key); it != textures.end())
            return it->second;

        GLuint texID  = create_gl_texture(data, width, height, channels);
        textures[key] = texID;
        spdlog::info("Loaded embedded Texture: {}, Path {}",texID, key);

        return texID;
    }

    // -------------------------------------------------------------
    // Cleanup all OpenGL textures
    // -------------------------------------------------------------
    void cleanup() {
        for (auto& [key, texID] : textures)
            glDeleteTextures(1, &texID);
        textures.clear();
    }
};

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
        invalidate();
    }

    ~OpenGLFramebuffer() override {
        cleanup();
    }

    void invalidate() override {
        if (fbo)
            cleanup();

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

class Renderer {
public:
};

// ============================================================================
// OPENGL RENDERER
// ============================================================================

class OpenGLRenderer {
    GLuint shaderProgram       = 0;
    GLuint shadowShaderProgram = 0;
    int width, height;
    TextureManager* textureManager = nullptr;
    std::shared_ptr<Framebuffer> shadowMap;

    const char* shadowVertexShaderSource = R"(
        #version 330 core
        layout (location = 0) in vec3 aPos;

        uniform mat4 lightSpaceMatrix;
        uniform mat4 model;

        void main() {
            gl_Position = lightSpaceMatrix * model * vec4(aPos, 1.0);
        }
    )";

    const char* shadowFragmentShaderSource = R"(
        #version 330 core

        void main() {
            // gl_FragDepth is automatically written
        }
    )";

    const char* vertexShaderSource = R"(
        #version 330 core
        layout (location = 0) in vec3 aPos;
        layout (location = 1) in vec3 aNormal;
        layout (location = 2) in vec2 aTexCoord;

        out vec3 FragPos;
        out vec3 Normal;
        out vec2 TexCoord;
        out vec4 FragPosLightSpace;

        uniform mat4 model;
        uniform mat4 view;
        uniform mat4 projection;
        uniform mat4 lightSpaceMatrix;

        void main() {
            FragPos = vec3(model * vec4(aPos, 1.0));
            Normal = mat3(transpose(inverse(model))) * aNormal;
            TexCoord = aTexCoord;
            FragPosLightSpace = lightSpaceMatrix * vec4(FragPos, 1.0);
            gl_Position = projection * view * vec4(FragPos, 1.0);
        }
    )";

    const char* fragmentShaderSource = R"(
        #version 330 core
        out vec4 FragColor;

        in vec3 FragPos;
        in vec3 Normal;
        in vec2 TexCoord;
        in vec4 FragPosLightSpace;

        uniform vec3 camPos;

        // Directional lights
        uniform vec3 dirLightDirections[4];
        uniform vec3 dirLightColors[4];
        uniform bool dirLightCastShadows[4];
        uniform int numDirLights;

        // Spot lights
        uniform vec3 spotLightPositions[4];
        uniform vec3 spotLightDirections[4];
        uniform vec3 spotLightColors[4];
        uniform float spotLightCutOffs[4];
        uniform float spotLightOuterCutOffs[4];
        uniform int numSpotLights;

        // Material
        uniform vec3 albedo;
        uniform float metallic;
        uniform float roughness;
        uniform float ao;
        uniform vec3 emissive;
        uniform float emissiveStrength;

        uniform sampler2D albedoMap;
        uniform sampler2D metallicMap;
        uniform sampler2D roughnessMap;
        uniform sampler2D normalMap;
        uniform sampler2D aoMap;
        uniform sampler2D emissiveMap;
        uniform sampler2D shadowMap;

        uniform bool useAlbedoMap;
        uniform bool useMetallicMap;
        uniform bool useRoughnessMap;
        uniform bool useNormalMap;
        uniform bool useAOMap;
        uniform bool useEmissiveMap;

        const float PI = 3.14159265359;

        float ShadowCalculation(vec4 fragPosLightSpace) {
            vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
            projCoords = projCoords * 0.5 + 0.5;
            float closestDepth = texture(shadowMap, projCoords.xy).r;
            float currentDepth = projCoords.z;
            float bias = 0.005;

            float shadow = 0.0;
            vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
            for(int x = -1; x <= 1; ++x) {
                for(int y = -1; y <= 1; ++y) {
                    float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r;
                    shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;
                }
            }
            shadow /= 9.0;

            if(projCoords.z > 1.0)
                shadow = 0.0;

            return shadow;
        }

        float DistributionGGX(vec3 N, vec3 H, float roughness) {
            float a = roughness * roughness;
            float a2 = a * a;
            float NdotH = max(dot(N, H), 0.0);
            float NdotH2 = NdotH * NdotH;

            float num = a2;
            float denom = (NdotH2 * (a2 - 1.0) + 1.0);
            denom = PI * denom * denom;

            return num / denom;
        }

        float GeometrySchlickGGX(float NdotV, float roughness) {
            float r = (roughness + 1.0);
            float k = (r * r) / 8.0;

            float num = NdotV;
            float denom = NdotV * (1.0 - k) + k;

            return num / denom;
        }

        float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness) {
            float NdotV = max(dot(N, V), 0.0);
            float NdotL = max(dot(N, L), 0.0);
            float ggx2 = GeometrySchlickGGX(NdotV, roughness);
            float ggx1 = GeometrySchlickGGX(NdotL, roughness);

            return ggx1 * ggx2;
        }

        vec3 fresnelSchlick(float cosTheta, vec3 F0) {
            return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
        }

        vec3 getNormalFromMap() {
            vec3 tangentNormal = texture(normalMap, TexCoord).xyz * 2.0 - 1.0;

            vec3 Q1 = dFdx(FragPos);
            vec3 Q2 = dFdy(FragPos);
            vec2 st1 = dFdx(TexCoord);
            vec2 st2 = dFdy(TexCoord);

            vec3 N = normalize(Normal);
            vec3 T = normalize(Q1 * st2.t - Q2 * st1.t);
            vec3 B = -normalize(cross(N, T));
            mat3 TBN = mat3(T, B, N);

            return normalize(TBN * tangentNormal);
        }

        void main() {
            if(texture(albedoMap, TexCoord).a < 0.1)
                discard;

        float finalMetallic = 0.0f;
        float finalRoughness = 0.0f;
         if (useMetallicMap) {
                vec3 mr_sample = texture(metallicMap, TexCoord).rgb;
                finalMetallic = mr_sample.b;
                finalRoughness = mr_sample.g;

                if(useRoughnessMap){
        finalRoughness = texture(roughnessMap, TexCoord).r;
        }else{
        finalRoughness = roughness;
        }
    }else{
    finalMetallic = metallic;
    }



            vec3 finalAlbedo = useAlbedoMap ? texture(albedoMap, TexCoord).rgb : albedo;

            float finalAO = useAOMap ? texture(aoMap, TexCoord).r : ao;
            vec3 finalEmissive = useEmissiveMap ? texture(emissiveMap, TexCoord).rgb * emissiveStrength : emissive * emissiveStrength;

            vec3 N = useNormalMap ? getNormalFromMap() : normalize(Normal);
            vec3 V = normalize(camPos - FragPos);

            vec3 F0 = vec3(0.04);
            F0 = mix(F0, finalAlbedo, finalMetallic);

            vec3 Lo = vec3(0.0);

            float shadow = ShadowCalculation(FragPosLightSpace);

            // Directional lights
            for(int i = 0; i < numDirLights; ++i) {
                vec3 L = normalize(-dirLightDirections[i]);
                vec3 H = normalize(V + L);
                vec3 radiance = dirLightColors[i];

                float NDF = DistributionGGX(N, H, finalRoughness);
                float G = GeometrySmith(N, V, L, finalRoughness);
                vec3 F = fresnelSchlick(max(dot(H, V), 0.0), F0);

                vec3 kS = F;
                vec3 kD = vec3(1.0) - kS;
                kD *= 1.0 - finalMetallic;

                vec3 numerator = NDF * G * F;
                float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001;
                vec3 specular = numerator / denominator;

                float NdotL = max(dot(N, L), 0.0);

                float shadowFactor = dirLightCastShadows[i] ? (1.0 - shadow) : 1.0;
                Lo += (kD * finalAlbedo / PI + specular) * radiance * NdotL * shadowFactor;
            }

            // Spot lights
            for(int i = 0; i < numSpotLights; ++i) {
                vec3 L = normalize(spotLightPositions[i] - FragPos);
                vec3 H = normalize(V + L);

                float distance = length(spotLightPositions[i] - FragPos);
                float attenuation = 1.0 / (distance * distance);

                float theta = dot(L, normalize(-spotLightDirections[i]));
                float epsilon = spotLightCutOffs[i] - spotLightOuterCutOffs[i];
                float intensity = clamp((theta - spotLightOuterCutOffs[i]) / epsilon, 0.0, 1.0);

                vec3 radiance = spotLightColors[i] * attenuation * intensity;

                float NDF = DistributionGGX(N, H, finalRoughness);
                float G = GeometrySmith(N, V, L, finalRoughness);
                vec3 F = fresnelSchlick(max(dot(H, V), 0.0), F0);

                vec3 kS = F;
                vec3 kD = vec3(1.0) - kS;
                kD *= 1.0 - finalMetallic;

                vec3 numerator = NDF * G * F;
                float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001;
                vec3 specular = numerator / denominator;

                float NdotL = max(dot(N, L), 0.0);
                Lo += (kD * finalAlbedo / PI + specular) * radiance * NdotL;
            }

            vec3 ambient = vec3(0.03) * finalAlbedo * finalAO;
            vec3 color = ambient + Lo + finalEmissive;

            color = color / (color + vec3(1.0));
            color = pow(color, vec3(1.0 / 2.2));
            FragColor = vec4(clamp(color, 0.0, 1.0), 1.0);
        }
    )";

    GLuint compile_shader(GLenum type, const char* source) {
        GLuint shader = glCreateShader(type);
        glShaderSource(shader, 1, &source, nullptr);
        glCompileShader(shader);

        int success;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if (!success) {
            char infoLog[512];
            glGetShaderInfoLog(shader, 512, nullptr, infoLog);
            std::cerr << "Shader compilation failed: " << infoLog << std::endl;
        }
        return shader;
    }

    GLuint create_shader_program(const char* vertSrc, const char* fragSrc) {
        GLuint vertexShader   = compile_shader(GL_VERTEX_SHADER, vertSrc);
        GLuint fragmentShader = compile_shader(GL_FRAGMENT_SHADER, fragSrc);

        GLuint program = glCreateProgram();
        glAttachShader(program, vertexShader);
        glAttachShader(program, fragmentShader);
        glLinkProgram(program);

        int success;
        glGetProgramiv(program, GL_LINK_STATUS, &success);
        if (!success) {
            char infoLog[512];
            glGetProgramInfoLog(program, 512, nullptr, infoLog);
            std::cerr << "Shader linking failed: " << infoLog << std::endl;
        }

        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);

        return program;
    }

public:
    void begin_shadow_pass() {
        shadowMap->bind();

        glEnable(GL_DEPTH_TEST);
        glClear(GL_DEPTH_BUFFER_BIT);
        glUseProgram(shadowShaderProgram);
    }

    void render_shadow_pass(const Transform& transform, const MeshRenderer& mesh, const glm::mat4& lightSpaceMatrix) {
        glm::mat4 model = transform.get_matrix();

        glUniformMatrix4fv(glGetUniformLocation(shadowShaderProgram, "lightSpaceMatrix"), 1, GL_FALSE, glm::value_ptr(lightSpaceMatrix));
        glUniformMatrix4fv(glGetUniformLocation(shadowShaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

        glBindVertexArray(mesh.VAO);
        glDrawElements(GL_TRIANGLES, mesh.indexCount, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
    }

    void end_shadow_pass() {
        shadowMap->unbind();
        glCullFace(GL_BACK);
        glViewport(0, 0, width, height);
    }

    bool initialize(int w, int h, TextureManager* texMgr) {
        width          = w;
        height         = h;
        textureManager = texMgr;


        if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(SDL_GL_GetProcAddress))) {
            std::cerr << "Failed to initialize GLAD" << std::endl;
            return false;
        }

        glEnable(GL_DEPTH_TEST);
        glViewport(0, 0, width, height);

        shaderProgram       = create_shader_program(vertexShaderSource, fragmentShaderSource);
        shadowShaderProgram = create_shader_program(shadowVertexShaderSource, shadowFragmentShaderSource);


        FramebufferSpecification spec;
        spec.width       = 8192;
        spec.height      = 8192;
        spec.attachments = {
            {FramebufferTextureFormat::DEPTH_COMPONENT}
        };

        shadowMap = std::make_shared<OpenGLFramebuffer>(spec);

        return true;
    }


    void begin_render_target() {
        glClearColor(0.1f, 0.1f, 0.15f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glUseProgram(shaderProgram);
        glViewport(0, 0, width, height);

    }


    void render_entity(const Transform& transform, const MeshRenderer& mesh,
                      const Material& material, const Camera& camera,
                      const glm::mat4& lightSpaceMatrix,
                      const std::vector<glm::vec3>& dirLightDirs,
                      const std::vector<glm::vec3>& dirLightColors,
                      const std::vector<bool>& dirLightCastShadows,
                      const std::vector<glm::vec3>& spotLightPos,
                      const std::vector<glm::vec3>& spotLightDirs,
                      const std::vector<glm::vec3>& spotLightColors,
                      const std::vector<float>& spotLightCutOffs,
                      const std::vector<float>& spotLightOuterCutOffs) {
        glm::mat4 model      = transform.get_matrix();
        glm::mat4 view       = camera.get_view_matrix();
        glm::mat4 projection = camera.get_projection_matrix(static_cast<float>(width) / height);

        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "lightSpaceMatrix"), 1, GL_FALSE, glm::value_ptr(lightSpaceMatrix));

        glUniform3fv(glGetUniformLocation(shaderProgram, "camPos"), 1, glm::value_ptr(camera.position));
        glUniform3fv(glGetUniformLocation(shaderProgram, "albedo"), 1, glm::value_ptr(material.albedo));
        glUniform1f(glGetUniformLocation(shaderProgram, "metallic"), material.metallic);
        glUniform1f(glGetUniformLocation(shaderProgram, "roughness"), material.roughness);
        glUniform1f(glGetUniformLocation(shaderProgram, "ao"), material.ao);
        glUniform3fv(glGetUniformLocation(shaderProgram, "emissive"), 1, glm::value_ptr(material.emissive));
        glUniform1f(glGetUniformLocation(shaderProgram, "emissiveStrength"), material.emissiveStrength);

        glUniform1i(glGetUniformLocation(shaderProgram, "useAlbedoMap"), material.useAlbedoMap);
        glUniform1i(glGetUniformLocation(shaderProgram, "useMetallicMap"), material.useMetallicMap);
        glUniform1i(glGetUniformLocation(shaderProgram, "useRoughnessMap"), material.useRoughnessMap);
        glUniform1i(glGetUniformLocation(shaderProgram, "useNormalMap"), material.useNormalMap);
        glUniform1i(glGetUniformLocation(shaderProgram, "useAOMap"), material.useAOMap);
        glUniform1i(glGetUniformLocation(shaderProgram, "useEmissiveMap"), material.useEmissiveMap);


        if (material.useAlbedoMap && material.albedoMap) {
            glActiveTexture(GL_TEXTURE0 + ALBEDO_TEXTURE_UNIT);
            glBindTexture(GL_TEXTURE_2D, material.albedoMap);
            glUniform1i(glGetUniformLocation(shaderProgram, "albedoMap"), ALBEDO_TEXTURE_UNIT);
        }

        if (material.useMetallicMap && material.metallicMap) {
            glActiveTexture(GL_TEXTURE0 + METALLIC_ROUGHNESS_TEXTURE_UNIT);
            glBindTexture(GL_TEXTURE_2D, material.metallicMap);
            glUniform1i(glGetUniformLocation(shaderProgram, "metallicMap"), METALLIC_ROUGHNESS_TEXTURE_UNIT);
        }

        if (material.useRoughnessMap && material.roughnessMap) {
            glActiveTexture(GL_TEXTURE0 + ROUGHNESS_TEXTURE_UNIT);
            glBindTexture(GL_TEXTURE_2D, material.roughnessMap);
            glUniform1i(glGetUniformLocation(shaderProgram, "roughnessMap"), ROUGHNESS_TEXTURE_UNIT);
        }

        if (material.useNormalMap && material.normalMap) {
            glActiveTexture(GL_TEXTURE0 + NORMAL_MAP_TEXTURE_UNIT);
            glBindTexture(GL_TEXTURE_2D, material.normalMap);
            glUniform1i(glGetUniformLocation(shaderProgram, "normalMap"), NORMAL_MAP_TEXTURE_UNIT);
        }

        if (material.useAOMap && material.aoMap) {
            glActiveTexture(GL_TEXTURE0 + AMBIENT_OCCLUSION_TEXTURE_UNIT);
            glBindTexture(GL_TEXTURE_2D, material.aoMap);
            glUniform1i(glGetUniformLocation(shaderProgram, "aoMap"), AMBIENT_OCCLUSION_TEXTURE_UNIT);
        }

        if (material.useEmissiveMap && material.emissiveMap) {
            glActiveTexture(GL_TEXTURE0 + EMISSIVE_TEXTURE_UNIT);
            glBindTexture(GL_TEXTURE_2D, material.emissiveMap);
            glUniform1i(glGetUniformLocation(shaderProgram, "emissiveMap"), EMISSIVE_TEXTURE_UNIT);
        }

        // Directional lights
        glUniform1i(glGetUniformLocation(shaderProgram, "numDirLights"), dirLightDirs.size());
        if (!dirLightDirs.empty()) {
            glUniform3fv(glGetUniformLocation(shaderProgram, "dirLightDirections"), dirLightDirs.size(), glm::value_ptr(dirLightDirs[0]));
            glUniform3fv(glGetUniformLocation(shaderProgram, "dirLightColors"), dirLightColors.size(), glm::value_ptr(dirLightColors[0]));

            std::vector<int> shadowsInt(dirLightCastShadows.begin(), dirLightCastShadows.end());
            glUniform1iv(glGetUniformLocation(shaderProgram, "dirLightCastShadows"), shadowsInt.size(), shadowsInt.data());
        }

        // Spot lights
        glUniform1i(glGetUniformLocation(shaderProgram, "numSpotLights"), spotLightPos.size());
        if (!spotLightPos.empty()) {
            glUniform3fv(glGetUniformLocation(shaderProgram, "spotLightPositions"), spotLightPos.size(), glm::value_ptr(spotLightPos[0]));
            glUniform3fv(glGetUniformLocation(shaderProgram, "spotLightDirections"), spotLightDirs.size(),
                         glm::value_ptr(spotLightDirs[0]));
            glUniform3fv(glGetUniformLocation(shaderProgram, "spotLightColors"), spotLightColors.size(),
                         glm::value_ptr(spotLightColors[0]));
            glUniform1fv(glGetUniformLocation(shaderProgram, "spotLightCutOffs"), spotLightCutOffs.size(), spotLightCutOffs.data());
            glUniform1fv(glGetUniformLocation(shaderProgram, "spotLightOuterCutOffs"), spotLightOuterCutOffs.size(),
                         spotLightOuterCutOffs.data());
        }

        glActiveTexture(GL_TEXTURE0 + SHADOW_TEXTURE_UNIT);
        glBindTexture(GL_TEXTURE_2D, shadowMap->get_depth_attachment_id());
        glUniform1i(glGetUniformLocation(shaderProgram, "shadowMap"), SHADOW_TEXTURE_UNIT);

        glBindVertexArray(mesh.VAO);
        glDrawElements(GL_TRIANGLES, mesh.indexCount, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
    }


    void end_render_target() {

    }

    void resize(int w, int h) {
        width  = w;
        height = h;
        glViewport(0, 0, width, height);
    }

    void cleanup() {
        if (shaderProgram)
            glDeleteProgram(shaderProgram);
        if (shadowShaderProgram)
            glDeleteProgram(shadowShaderProgram);
    }
};

// ============================================================================
// MODEL (combines mesh + material)
// ============================================================================

struct Model {
    std::vector<MeshRenderer> meshes;
    std::vector<Material> materials;
};

// ============================================================================
// MESH LOADER (ASSIMP)
// ============================================================================

class MeshLoader {
private:
    static std::string get_directory(const std::string& path) {
        size_t found = path.find_last_of("/\\");
        if (found != std::string::npos) {
            return path.substr(0, found + 1);
        }
        return "";
    }

    static Material load_material(const aiScene* scene, aiMesh* mesh,
                                 const std::string& directory,
                                 TextureManager& texMgr) {

        Material material;
        if (scene->mNumMaterials > mesh->mMaterialIndex) {
            aiMaterial* aiMat = scene->mMaterials[mesh->mMaterialIndex];

            // Load diffuse/albedo color
            aiColor3D color(1.0f, 1.0f, 1.0f);
            aiMat->Get(AI_MATKEY_COLOR_DIFFUSE, color);
            material.albedo = glm::vec3(color.r, color.g, color.b);

            // Load emissive color
            aiColor3D emissiveColor(0.0f, 0.0f, 0.0f);
            if (aiMat->Get(AI_MATKEY_COLOR_EMISSIVE, emissiveColor) == AI_SUCCESS) {
                material.emissive = glm::vec3(emissiveColor.r, emissiveColor.g, emissiveColor.b);
            }

            // Load emissive strength
            float emissiveStrength = 1.0f;
            if (aiMat->Get(AI_MATKEY_EMISSIVE_INTENSITY, emissiveStrength) == AI_SUCCESS) {
                material.emissiveStrength = emissiveStrength;
            }

            // Load metallic (if available)
            float metallic = 0.0f;
            if (aiMat->Get(AI_MATKEY_METALLIC_FACTOR, metallic) == AI_SUCCESS) {
                material.metallic = metallic;
            }

            // Load roughness (if available)
            float roughness = 0.5f;
            if (aiMat->Get(AI_MATKEY_ROUGHNESS_FACTOR, roughness) == AI_SUCCESS) {
                material.roughness = roughness;
            } else {
                // Try shininess (convert to roughness)
                float shininess = 0.0f;
                if (aiMat->Get(AI_MATKEY_SHININESS, shininess) == AI_SUCCESS) {
                    material.roughness = 1.0f - glm::clamp(shininess / 128.0f, 0.0f, 1.0f);
                }
            }

            auto load_texture = [&](aiTextureType type, GLuint& texId, bool& useFlag, const char* typeName) {
                if (aiMat->GetTextureCount(type) > 0) {
                    aiString texPath;
                    if (aiMat->GetTexture(type, 0, &texPath) == AI_SUCCESS) {
                        std::string texStr = texPath.C_Str();

                        // Check for embedded texture (starts with '*')
                        if (!texStr.empty() && texStr[0] == '*') {
                            int texIndex = std::atoi(texStr.c_str() + 1);
                            if (texIndex >= 0 && texIndex < static_cast<int>(scene->mNumTextures)) {
                                const aiTexture* embeddedTex = scene->mTextures[texIndex];
                                if (embeddedTex) {
                                    // Handle compressed embedded textures (e.g., PNG, JPEG)
                                    if (embeddedTex->mHeight == 0) {
                                        texId = texMgr.load_texture_from_memory(
                                            reinterpret_cast<const unsigned char*>(embeddedTex->pcData),
                                            embeddedTex->mWidth
                                            );
                                    }
                                    // Handle uncompressed embedded textures (raw pixel data)
                                    else {
                                        texId = texMgr.load_texture_from_raw_data(
                                            reinterpret_cast<const unsigned char*>(embeddedTex->pcData),
                                            embeddedTex->mWidth, embeddedTex->mHeight
                                            );
                                    }

                                    if (texId != 0) {
                                        useFlag = true;
                                        spdlog::info("  - Loaded embedded {} ({})", typeName, texStr);
                                        return;
                                    }
                                }
                            }
                        }

                        // Otherwise, load from file path (external texture)
                        std::string fullPath = directory + "/" + texStr;
                        texId                = texMgr.load_texture_from_file(fullPath);
                        if (texId != 0) {
                            useFlag = true;
                            spdlog::info("  - Loaded {}: {}", typeName, fullPath);
                        }
                    }
                }
            };

            load_texture(aiTextureType_DIFFUSE, material.albedoMap, material.useAlbedoMap, "albedo");
            load_texture(aiTextureType_METALNESS, material.metallicMap, material.useMetallicMap, "metallic");
            load_texture(aiTextureType_DIFFUSE_ROUGHNESS, material.roughnessMap, material.useRoughnessMap, "roughness");
            load_texture(aiTextureType_NORMALS, material.normalMap, material.useNormalMap, "normal");
            load_texture(aiTextureType_AMBIENT_OCCLUSION, material.aoMap, material.useAOMap, "AO");
            load_texture(aiTextureType_EMISSIVE, material.emissiveMap, material.useEmissiveMap, "emissive");

            // Alternative texture types (fallbacks)
            if (!material.useAlbedoMap) {
                load_texture(aiTextureType_BASE_COLOR, material.albedoMap, material.useAlbedoMap, "base color");
            }
            if (!material.useNormalMap) {
                load_texture(aiTextureType_HEIGHT, material.normalMap, material.useNormalMap, "normal (height)");
            }
            if (!material.useEmissiveMap) {
                load_texture(aiTextureType_EMISSION_COLOR, material.emissiveMap, material.useEmissiveMap, "emission color");
            }
        }

        return material;
    }

public:
    static MeshRenderer load_mesh(const std::string& path) {
        Model model = load_model(path, nullptr);
        return model.meshes.empty() ? MeshRenderer() : model.meshes[0];
    }

    static Model load_model(const std::string& path, TextureManager* texMgr = nullptr) {
        Model model;

        Assimp::Importer importer;
        const aiScene* scene = importer.ReadFile(path,
                                                 aiProcess_Triangulate |
                                                 aiProcess_FlipUVs |
                                                 aiProcess_CalcTangentSpace |
                                                 aiProcess_GenNormals);

        if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
            spdlog::error("Failed to load model: {}", path);
            return model;
        }

        std::string directory = get_directory(path);
        spdlog::info("Loading model: {}", path);
        spdlog::info("Meshes: {}, Materials: {}", scene->mNumMeshes, scene->mNumMaterials);

        // Load all meshes
        for (unsigned int m = 0; m < scene->mNumMeshes; m++) {
            aiMesh* aiMesh = scene->mMeshes[m];

            std::vector<float> vertices;
            std::vector<unsigned int> indices;

            // Process vertices
            for (unsigned int i = 0; i < aiMesh->mNumVertices; i++) {
                // Position
                vertices.push_back(aiMesh->mVertices[i].x);
                vertices.push_back(aiMesh->mVertices[i].y);
                vertices.push_back(aiMesh->mVertices[i].z);

                // Normal
                if (aiMesh->HasNormals()) {
                    vertices.push_back(aiMesh->mNormals[i].x);
                    vertices.push_back(aiMesh->mNormals[i].y);
                    vertices.push_back(aiMesh->mNormals[i].z);
                } else {
                    vertices.push_back(0.0f);
                    vertices.push_back(1.0f);
                    vertices.push_back(0.0f);
                }

                // Texture coordinates
                if (aiMesh->mTextureCoords[0]) {
                    vertices.push_back(aiMesh->mTextureCoords[0][i].x);
                    vertices.push_back(aiMesh->mTextureCoords[0][i].y);
                } else {
                    vertices.push_back(0.0f);
                    vertices.push_back(0.0f);
                }
            }

            // Process indices
            for (unsigned int i = 0; i < aiMesh->mNumFaces; i++) {
                aiFace face = aiMesh->mFaces[i];
                for (unsigned int j = 0; j < face.mNumIndices; j++) {
                    indices.push_back(face.mIndices[j]);
                }
            }

            // Create OpenGL buffers
            MeshRenderer mesh;
            glGenVertexArrays(1, &mesh.VAO);
            glGenBuffers(1, &mesh.VBO);
            glGenBuffers(1, &mesh.EBO);

            glBindVertexArray(mesh.VAO);

            glBindBuffer(GL_ARRAY_BUFFER, mesh.VBO);
            glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.EBO);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

            // Position
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*) 0);
            glEnableVertexAttribArray(0);
            // Normal
            glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*) (3 * sizeof(float)));
            glEnableVertexAttribArray(1);
            // TexCoord
            glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*) (6 * sizeof(float)));
            glEnableVertexAttribArray(2);

            glBindVertexArray(0);

            mesh.indexCount = indices.size();
            model.meshes.push_back(mesh);

            // Load material if texture manager is provided
            if (texMgr) {
                Material mat = load_material(scene, aiMesh, directory, *texMgr);
                model.materials.push_back(mat);
                spdlog::info("Mesh {}: {} vertices, {} triangles", m, aiMesh->mNumVertices, indices.size() / 3);
                spdlog::info("Material - Albedo: ({}, {}, {}), Metallic: {}, Roughness: {}, AmbientOcclusion: {}",
                             mat.albedo.r, mat.albedo.g, mat.albedo.b,
                             mat.metallic, mat.roughness, mat.ao);
            }
        }

        return model;
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

    OpenGLRenderer renderer;
    TextureManager textureManager;
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

        if (!renderer.initialize(width, height, &textureManager)) {
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
                renderer.resize(width, height);
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
        std::vector<glm::vec3> dirLightDirections;
        std::vector<glm::vec3> dirLightColors;
        std::vector<bool> dirLightCastShadows;
        glm::mat4 lightSpaceMatrix(1.0f);

        Camera mainCamera;
        ecs.each([&](const Camera& cam) {
            mainCamera = cam;
        });

        ecs.each([&](flecs::entity e, Transform& t, DirectionalLight& light) {
            dirLightDirections.push_back(light.direction);
            dirLightColors.push_back(light.color * light.intensity);
            dirLightCastShadows.push_back(light.castShadows);

            if (light.castShadows && lightSpaceMatrix == glm::mat4(1.0f)) {
                lightSpaceMatrix = light.get_light_space_matrix();
            }
        });

        std::vector<glm::vec3> spotLightPositions;
        std::vector<glm::vec3> spotLightDirections;
        std::vector<glm::vec3> spotLightColors;
        std::vector<float> spotLightCutOffs;
        std::vector<float> spotLightOuterCutOffs;

        ecs.each([&](flecs::entity e, Transform& t, SpotLight& light) {
            spotLightPositions.push_back(t.position);
            spotLightDirections.push_back(light.direction);
            spotLightColors.push_back(light.color * light.intensity);
            spotLightCutOffs.push_back(glm::cos(glm::radians(light.cutOff)));
            spotLightOuterCutOffs.push_back(glm::cos(glm::radians(light.outerCutOff)));
        });

        renderer.begin_shadow_pass();
        ecs.each([&](Transform& t, MeshRenderer& mesh, Material& mat) {
            renderer.render_shadow_pass(t, mesh, lightSpaceMatrix);
        });
        renderer.end_shadow_pass();

        renderer.begin_render_target();
        ecs.each([&](Transform& t, MeshRenderer& mesh, Material& mat) {
            renderer.render_entity(t, mesh, mat, mainCamera, lightSpaceMatrix,
                                  dirLightDirections, dirLightColors, dirLightCastShadows,
                                  spotLightPositions, spotLightDirections,
                                  spotLightColors, spotLightCutOffs, spotLightOuterCutOffs);
        });
        renderer.end_render_target();

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

    TextureManager& get_texture_manager() {
        return textureManager;
    }

    void cleanup() {
        renderer.cleanup();
        textureManager.cleanup();

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

    auto& ecs    = engine.get_world();
    auto& texMgr = engine.get_texture_manager();

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


    Model carModel = MeshLoader::load_model("res/sprites/obj/Car2.obj", &texMgr);
    for (size_t i = 0; i < carModel.meshes.size(); i++) {
        ecs.entity(("Car_Mesh_" + std::to_string(i)).c_str())
           .set(Transform{glm::vec3(-10, 0, -5), glm::vec3(0), glm::vec3(1.0f)})
           .set(carModel.meshes[i])
           .set(carModel.materials[i]);
    }


    Model damagedHelmet = MeshLoader::load_model("res/sprites/obj/DamagedHelmet.glb", &texMgr);
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

    Model medieval = MeshLoader::load_model("res/sprites/obj/sponza/huge_medieval_battle_scene.glb", &texMgr);
    for (size_t i = 0; i < medieval.meshes.size(); i++) {
        ecs.entity(("Sponza_Mesh_" + std::to_string(i)).c_str())
            .set(Transform{glm::vec3(0, 0, 0), glm::vec3(0), glm::vec3(1.0f)})
            .set(medieval.meshes[i])
            .set(medieval.materials[i]);
    }

    MeshRenderer cylinderMesh = MeshLoader::load_mesh("res/models/cylinder.obj");
    ecs.entity("Cylinder").set(Transform{glm::vec3(0, 0, -10), glm::vec3(0), glm::vec3(1.0f)}).set(cylinderMesh).set(Material{
        .albedo = glm::vec3(0, 0, 1.0f)});

    MeshRenderer torusMesh = MeshLoader::load_mesh("res/models/torus.obj");
    ecs.entity("Torus").set(Transform{glm::vec3(0, 0, 5), glm::vec3(0), glm::vec3(1.0f)}).set(torusMesh).set(Material{
        .albedo = glm::vec3(0, 1.0f, 1.0), .emissive = glm::vec3(1, 1, 1), .emissiveStrength = 1.0f});

    MeshRenderer coneMesh = MeshLoader::load_mesh("res/models/cone.obj");
    ecs.entity("Cone").set(Transform{glm::vec3(0, 0, 20), glm::vec3(0), glm::vec3(1.0f)}).set(coneMesh).set(Material{
        .albedo = glm::vec3(0, 1.0f, 1.0)});

    MeshRenderer blenderMonkeyMesh = MeshLoader::load_mesh("res/models/blender_monkey.obj");
    ecs.entity("Cone").set(Transform{glm::vec3(0, 0, 10), glm::vec3(0), glm::vec3(1.0f)}).set(blenderMonkeyMesh).set(Material{
        .albedo = glm::vec3(1.0f, 0.0f, 1.0)});

    MeshRenderer cubeMesh = MeshLoader::load_mesh("res/models/cube.obj");
    ecs.entity("Red Cube")
       .set(Transform{glm::vec3(3, 0, 0), glm::vec3(0), glm::vec3(1.5f)})
       .set(cubeMesh)
       .set(Material{glm::vec3(0.8f, 0.1f, 0.1f), 0.0f, 0.3f, 1.0f});

    MeshRenderer sphereMesh = MeshLoader::load_mesh("res/models/sphere.obj");

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
