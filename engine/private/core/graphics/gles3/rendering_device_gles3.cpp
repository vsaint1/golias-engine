#include "core/graphics/gles3/rendering_device_gles3.h"

#include "core/engine.h"
#include "core/graphics/gles3/gl_common.h"
#include "core/graphics/rendering_device.h"

namespace golias {

    bool RenderingDeviceGLES3::Initialize(SDL_Window* sdl_window) {
        window = sdl_window;

#if defined(SDL_PLATFORM_ANDROID) || defined(SDL_PLATFORM_IOS) || defined(SDL_PLATFORM_EMSCRIPTEN)
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);

#else
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

    #if defined(SDL_PLATFORM_MACOS)
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG); // OSX compatibility Bit
    #endif


#endif

        gl_context = SDL_GL_CreateContext(sdl_window);

        if (!gl_context) {
            spdlog::critical("RenderingDeviceGLES3::Initialize Failed to create OpenGL/ES context: {}", SDL_GetError());
            return false;
        }


#if defined(SDL_PLATFORM_ANDROID) && !defined(SDL_PLATFORM_IOS) && !defined(SDL_PLATFORM_EMSCRIPTEN)
        if (!gladLoadGLES2Loader(reinterpret_cast<GLADloadproc>(SDL_GL_GetProcAddress))) {
            spdlog::error("RenderingDeviceGLES3::Initialize Failed to initialize OpenGL Loader (GLAD)");
            return false;
        }
#else
        if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(SDL_GL_GetProcAddress))) {
            spdlog::error("RenderingDeviceGLES3::Initialize Failed to initialize OpenGLES Loader (GLAD)");
            return false;
        }
#endif

        if (!CreateDefaultShaders()) {
            spdlog::error("RenderingDeviceGLES3::Initialize Failed to create default shaders.");
            return false;
        }

        spdlog::info("RenderingDeviceGLES3::Initialize Initialized successfully GLES3 Rendering Device.");

        glEnable(GL_DEPTH_TEST);
        return true;
    }

    void RenderingDeviceGLES3::BindShader(Shader* shader) {
        if (shader) {
            shader->Bind();
        }
    }


    void RenderingDeviceGLES3::BindMesh(Mesh* mesh) {
        if (mesh) {
            mesh->Bind();
        }
    }

    void RenderingDeviceGLES3::BindMaterial(Material* material) {
        if (material) {
            material->Activate();
        }
    }

    void RenderingDeviceGLES3::DrawMesh(Mesh* mesh) {
        if (mesh) {
            mesh->Draw();
        }
    }

    std::shared_ptr<Texture2D> RenderingDeviceGLES3::CreateTextureFromFile(const std::string_view pFilePath) {

        auto file_system = golias::Engine::GetInstance().GetFileSystem();

        return std::make_shared<OpenglTexture2D>(file_system.GetAssetFile(pFilePath));
    }

    std::shared_ptr<Texture2D> RenderingDeviceGLES3::CreateTextureFromData(int w, int h, ETextureFormat format, const Uint8* data) {

        return std::make_shared<OpenglTexture2D>(w, h, 3, const_cast<Uint8*>(data));
    }

    std::shared_ptr<Shader> RenderingDeviceGLES3::CreateShaderFromSource(const std::string& vertexSource,
                                                                         const std::string& fragmentSource) {

        return std::make_shared<OpenglShader>(vertexSource, fragmentSource);
    }

    std::shared_ptr<Mesh> RenderingDeviceGLES3::CreateMeshFromData(const VertexLayout& layout, const std::vector<float>& vertices,

                                                                   const std::vector<uint32_t>& indices) {

        if (indices.empty()) {
            return std::make_shared<OpenglMesh>(layout, vertices);
        } else {
            return std::make_shared<OpenglMesh>(layout, vertices, indices);
        }
    }

    std::shared_ptr<Mesh> RenderingDeviceGLES3::CreateMesh() {
        return std::shared_ptr<OpenglMesh>();
    }


    void RenderingDeviceGLES3::Clear(glm::vec4 color) {
        glClearColor(color.r, color.g, color.b, color.a);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        clear_color = color;
    }

    Buffer RenderingDeviceGLES3::CreateGPUBuffer(size_t size, const void* data, EBufferUsageFlags bufferFlags, EBufferTarget bufferTarget) {
        Buffer buffer;
        glGenBuffers(1, &buffer.handle);

        GLint gl_buffer_target = ToGLBuferTarget(bufferTarget);
        glBindBuffer(gl_buffer_target, buffer.handle);
        glBufferData(gl_buffer_target, size, data, ToGLBufferUsage(bufferFlags));
        glBindBuffer(gl_buffer_target, 0);

        buffer.size        = size;
        buffer.usage_flags = bufferFlags;
        buffer.target      = bufferTarget;
        return buffer;
    }


    std::shared_ptr<Shader> RenderingDeviceGLES3::CreateShaderFromFile(const std::string_view pFilePath) {

        spdlog::warn("RenderingDeviceGLES3::CreateShaderFromFile not implemented yet.");

        return nullptr;
    }

    void RenderingDeviceGLES3::Present() {
        SDL_GL_SwapWindow(window);
    }

    bool RenderingDeviceGLES3::CreateDefaultShaders() {
        const std::string vertexSource = R"(
#version 330 core

layout(location = 0) in vec3 a_pos;
layout(location = 1) in vec3 a_color;
layout(location = 2) in vec2 a_texcoord;
layout(location = 3) in vec3 a_normal;

out vec3 v_color;
out vec2 v_texcoord;
out vec3 v_normal;
out vec3 v_frag_pos;

uniform mat4 MODEL_MATRIX;
uniform mat4 VIEW_MATRIX;
uniform mat4 PROJECTION_MATRIX;

void main() {
    v_color = a_color;
    v_texcoord = a_texcoord;
    
    // Transform position to world space
    vec4 worldPos = MODEL_MATRIX * vec4(a_pos, 1.0);
    v_frag_pos = worldPos.xyz;
    
    // Transform normal to world space
    if (length(a_normal) > 0.01) {
        mat3 normalMatrix = transpose(inverse(mat3(MODEL_MATRIX)));
        v_normal = normalize(normalMatrix * a_normal);
    } else {
        v_normal = vec3(0.0, 1.0, 0.0);
    }
    
    gl_Position = PROJECTION_MATRIX * VIEW_MATRIX * worldPos;
}
        )";


const std::string fragmentSource = R"(
#version 330 core
in vec3 v_color;
in vec2 v_texcoord;
in vec3 v_normal;
in vec3 v_frag_pos;
out vec4 COLOR;

// PBR Textures
uniform sampler2D ALBEDO_TEXTURE;
uniform sampler2D METALLIC_TEXTURE;
uniform sampler2D ROUGHNESS_TEXTURE;
uniform sampler2D NORMAL_TEXTURE;
uniform sampler2D AO_TEXTURE;
uniform sampler2D EMISSIVE_TEXTURE;

// Texture flags (0 = not present, 1 = present) - WITH DEFAULTS
uniform int HAS_ALBEDO = 0;
uniform int HAS_METALLIC = 0;
uniform int HAS_ROUGHNESS = 0;
uniform int HAS_NORMAL = 0;
uniform int HAS_AO = 0;
uniform int HAS_EMISSIVE = 0;

// Material properties - WITH DEFAULTS
uniform vec4 BASE_COLOR = vec4(1.0, 1.0, 1.0, 1.0);
uniform float METALLIC_FACTOR = 0.0;
uniform float ROUGHNESS_FACTOR = 1.0;
uniform vec3 EMISSIVE_FACTOR = vec3(0.0);

// Lighting - WITH DEFAULTS
uniform vec3 VIEW_POSITION = vec3(0.0, 0.0, 5.0);
uniform vec3 LIGHT_POSITION = vec3(50.0, 100.0, 80.0);
uniform vec3 LIGHT_COLOR = vec3(1.0, 0.95, 0.9);
uniform float AMBIENT_STRENGTH = 0.25;
uniform float SPECULAR_STRENGTH = 0.4;
uniform float SHININESS = 32.0;

void main() {
    // Sample albedo texture or use base color
    vec4 albedo = (HAS_ALBEDO == 1) ? texture(ALBEDO_TEXTURE, v_texcoord) : BASE_COLOR;
    albedo.rgb *= v_color; // Multiply by vertex color
    
    // Sample metallic
    float metallic = (HAS_METALLIC == 1) ? texture(METALLIC_TEXTURE, v_texcoord).b : METALLIC_FACTOR;
    
    // Sample roughness (G channel in GLTF metallic-roughness texture)
    float roughness = (HAS_ROUGHNESS == 1) ? texture(ROUGHNESS_TEXTURE, v_texcoord).g : ROUGHNESS_FACTOR;
    
    // Sample AO (Ambient Occlusion)
    float ao = (HAS_AO == 1) ? texture(AO_TEXTURE, v_texcoord).r : 1.0;
    
    // Sample emissive
    vec3 emissive = (HAS_EMISSIVE == 1) ? texture(EMISSIVE_TEXTURE, v_texcoord).rgb * EMISSIVE_FACTOR : vec3(0.0);
    
    // Get normal
    vec3 norm = normalize(v_normal);
    if (HAS_NORMAL == 1) {
        vec3 tangentNormal = texture(NORMAL_TEXTURE, v_texcoord).xyz * 2.0 - 1.0;
        // TODO: Apply TBN matrix here
        norm = normalize(v_normal);
    }
    
    // Base color
    vec3 baseColor = albedo.rgb;
    
    // Ambient lighting (affected by AO)
    vec3 ambient = AMBIENT_STRENGTH * LIGHT_COLOR * ao;
    
    // Diffuse lighting
    vec3 lightDir = normalize(LIGHT_POSITION - v_frag_pos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * LIGHT_COLOR;
    
    // Specular lighting (Blinn-Phong) - affected by roughness
    vec3 viewDir = normalize(VIEW_POSITION - v_frag_pos);
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float adjustedShininess = max(SHININESS * (1.0 - roughness), 1.0); // Clamp to avoid pow(x,0)
    float spec = pow(max(dot(norm, halfwayDir), 0.0), adjustedShininess);
    vec3 specular = SPECULAR_STRENGTH * spec * LIGHT_COLOR;
    
    // Metallic surfaces have different specular behavior
    vec3 F0 = mix(vec3(0.04), baseColor, metallic);
    specular = mix(specular, specular * baseColor, metallic);
    
    // Combine lighting
    vec3 result = (ambient + diffuse + specular) * baseColor + emissive;
    
    // Output final color with original alpha
    COLOR = vec4(result, albedo.a);
}
)";

        default_shader_3d = std::make_shared<OpenglShader>(vertexSource, fragmentSource);


        if (!default_shader_3d) {
            spdlog::error("RenderingDeviceGLES3::CreateDefaultShaders Failed to create default 3D shader.");
            return false;
        }


        return true;
    }

    std::shared_ptr<Shader> RenderingDeviceGLES3::GetDefaultShader3D() const {
        return default_shader_3d;
    }


    RenderingDeviceGLES3::~RenderingDeviceGLES3() {
        if (gl_context) {
            SDL_GL_DestroyContext(gl_context);
            gl_context = nullptr;
        }

        spdlog::info("RenderingDeviceGLES3::~RenderingDeviceGLES3 GLES3 Rendering Device destroyed.");
    }


}; // namespace golias
