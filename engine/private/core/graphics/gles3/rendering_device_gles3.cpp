#include "core/graphics/gles3/rendering_device_gles3.h"

#include "core/engine.h"
#include "core/graphics/gles3/gl_common.h"
#include "core/graphics/gles3/gl_physics_debug_drawer.h"
#include "core/graphics/gles3/storage/gl_framebuffer.h"
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

        if (!CreateDefaultFramebuffers()) {
            spdlog::error("RenderingDeviceGLES3::Initialize Failed to create default framebuffers.");
            return false;
        }

        if (!CreateSkyboxShader()) {
            spdlog::error("RenderingDeviceGLES3::Initialize Failed to create skybox shader.");
            return false;
        }

        if (!CreateDefaultSkybox()) {
            spdlog::error("RenderingDeviceGLES3::Initialize Failed to create default skybox.");
            return false;
        }

        spdlog::info("RenderingDeviceGLES3::Initialize Initialized successfully GLES3 Rendering Device.");

        physicsDebug3D = std::make_unique<PhysicsDebugDrawerGLES3>();

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

    void RenderingDeviceGLES3::UnbindMesh(Mesh* mesh) {
        if (mesh) {
            mesh->Unbind();
        }
    }

    PhysicsDebugDrawer* RenderingDeviceGLES3::GetPhysicsDebugDrawer() {
        return physicsDebug3D.get();
    }

    std::shared_ptr<Texture2D> RenderingDeviceGLES3::CreateTextureFromFile(const std::string_view pFilePath) {

        auto file_system = golias::Engine::GetInstance().GetFileSystem();

        return std::make_shared<OpenglTexture2D>(file_system.GetAssetFile(pFilePath));
    }

    std::shared_ptr<Texture2D> RenderingDeviceGLES3::CreateTextureFromData(int w, int h, ETextureFormat format, const Uint8* data) {

        return std::make_shared<OpenglTexture2D>(w, h, format, const_cast<Uint8*>(data));
    }

    std::shared_ptr<Shader> RenderingDeviceGLES3::CreateShaderFromSource(const std::string& vertexSource,
                                                                         const std::string& fragmentSource) {

        return std::make_shared<OpenglShader>(vertexSource, fragmentSource);
    }

    std::shared_ptr<Mesh> RenderingDeviceGLES3::CreateMeshFromData(const VertexLayout& layout,
                                                                   const std::vector<float>& vertices,

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


    std::shared_ptr<Framebuffer> RenderingDeviceGLES3::CreateFramebuffer(const FramebufferSpec& speficiation) {
        return std::make_shared<GLFramebuffer>(speficiation);
    }

    std::shared_ptr<Shader> RenderingDeviceGLES3::CreateShaderFromFile(const std::string_view pFilePath) {

        spdlog::warn("RenderingDeviceGLES3::CreateShaderFromFile not implemented yet.");

        return nullptr;
    }

    void RenderingDeviceGLES3::SwapChain() {
        SDL_GL_SwapWindow(window);
    }

    bool RenderingDeviceGLES3::CreateDefaultShaders() {
        const auto& fs = Engine::GetInstance().GetFileSystem();

        std::string vertexSource = GetShaderHeaderVersion() + fs.LoadAssetFileText("shaders/default_3d.vert");

        std::string fragmentSource = GetShaderHeaderVersion() + fs.LoadAssetFileText("shaders/default_3d.frag");

        default_shader_3d = std::make_shared<OpenglShader>(vertexSource, fragmentSource);


        if (!default_shader_3d) {
            spdlog::error("RenderingDeviceGLES3::CreateDefaultShaders Failed to create default 3D shader.");
            return false;
        }

        vertexSource = GetShaderHeaderVersion() + fs.LoadAssetFileText("shaders/default_2d.vert");

        fragmentSource = GetShaderHeaderVersion() + fs.LoadAssetFileText("shaders/default_2d.frag");

        default_shader_2d = std::make_shared<OpenglShader>(vertexSource, fragmentSource);

        if (!default_shader_2d) {
            spdlog::error("RenderingDeviceGLES3::CreateDefaultShaders Failed to create default 2D shader.");
            return false;
        }

        vertexSource   = GetShaderHeaderVersion() + fs.LoadAssetFileText("shaders/default_canvas.vert");
        fragmentSource = GetShaderHeaderVersion() + fs.LoadAssetFileText("shaders/default_canvas.frag");

        default_shader_canvas = std::make_shared<OpenglShader>(vertexSource, fragmentSource);

        if (!default_shader_canvas) {
            spdlog::error("RenderingDeviceGLES3::CreateDefaultShaders Failed to create default canvas shader.");
            return false;
        }

        vertexSource   = GetShaderHeaderVersion() + fs.LoadAssetFileText("shaders/shadow_map.vert");
        fragmentSource = GetShaderHeaderVersion() + fs.LoadAssetFileText("shaders/shadow_map.frag");

        default_shader_csm = std::make_shared<OpenglShader>(vertexSource, fragmentSource);

        if (!default_shader_csm) {
            spdlog::error("RenderingDeviceGLES3::CreateDefaultShaders Failed to create shadow map shader.");
            return false;
        }

        return true;
    }

    void RenderingDeviceGLES3::SetViewport(const Rect& vp) {
        viewport = vp;
        glViewport(vp.x, vp.y, vp.width, vp.height);
    }

    std::shared_ptr<Shader> RenderingDeviceGLES3::GetDefaultShader3D() const {
        return default_shader_3d;
    }

    std::shared_ptr<Shader> RenderingDeviceGLES3::GetDefaultShader2D() const {
        return default_shader_2d;
    }

    std::shared_ptr<Shader> RenderingDeviceGLES3::GetDefaultShaderCanvas() const {
        return default_shader_canvas;
    }


    bool RenderingDeviceGLES3::CreateDefaultFramebuffers() {
        FramebufferSpec shadowFboSpec;
        shadowFboSpec.width       = 4096;
        shadowFboSpec.height      = 4096;
        shadowFboSpec.attachments = {
            FramebufferAttachmentSpec(EFramebufferAttachment::DEPTH_ATTACHMENT, EFramebufferTextureFormat::DEPTH32F)};

        shadowFBO = std::make_shared<GLFramebuffer>(shadowFboSpec);

        if (!shadowFBO) {
            spdlog::error("RenderingDeviceGLES3::CreateDefaultFramebuffers Failed to create shadow map Framebuffer");
            return false;
        }

        spdlog::info("RenderingDeviceGLES3::CreateDefaultFramebuffers Created shadow map Framebuffer");

        return true;
    }

    std::shared_ptr<Shader> RenderingDeviceGLES3::GetDefaultShadowMapShader() const {
        return default_shader_csm;
    }

    void RenderingDeviceGLES3::BeginShadowPass() {

        if (!shadowFBO) {
            return;
        }

        shadowFBO->Bind();
        glClear(GL_DEPTH_BUFFER_BIT);
        glEnable(GL_DEPTH_TEST);
        glCullFace(GL_FRONT);
    }

    void RenderingDeviceGLES3::EndShadowPass() {
        glCullFace(GL_BACK);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glViewport(0, 0, viewport.width, viewport.height);
    }

    bool RenderingDeviceGLES3::CreateSkyboxShader() {
        const auto& fs = Engine::GetInstance().GetFileSystem();

        std::string vertexSource   = GetShaderHeaderVersion() + fs.LoadAssetFileText("shaders/skybox.vert");
        std::string fragmentSource = GetShaderHeaderVersion() + fs.LoadAssetFileText("shaders/skybox.frag");

        skybox_shader = std::make_shared<OpenglShader>(vertexSource, fragmentSource);

        if (!skybox_shader) {
            spdlog::error("RenderingDeviceGLES3::CreateSkyboxShader Failed to create skybox shader.");
            return false;
        }

        return true;
    }

    Uint32 RenderingDeviceGLES3::CreateCubemapFromFiles(const std::array<std::string, 6>& faces) {
        Uint32 textureID;
        glGenTextures(1, &textureID);
        glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

        const auto& fs = Engine::GetInstance().GetFileSystem();

        for (Uint32 i = 0; i < faces.size(); i++) {
            auto imagePath = fs.GetAssetFile(faces[i]);

            int width, height, nrChannels;
            unsigned char* data = stbi_load(imagePath.c_str(), &width, &height, &nrChannels, 0);

            if (data) {
                GLenum format = (nrChannels == 3) ? GL_RGB : GL_RGBA;
                glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
                stbi_image_free(data);
            } else {
                spdlog::error("RenderingDeviceGLES3::CreateCubemapFromFiles Failed to load cubemap texture: {}", faces[i]);
                stbi_image_free(data);
                return 0;
            }
        }

        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

        return textureID;
    }

    Uint32 RenderingDeviceGLES3::CreateCubemapFromCross(const std::string& crossPath) {
        const auto& fs = Engine::GetInstance().GetFileSystem();
        auto imagePath = fs.GetAssetFile(crossPath);

        // Check if HDR based on extension
        bool isHDR = imagePath.ends_with(".hdr") || imagePath.ends_with(".HDR");

        int width, height, nrChannels;
        void* data = nullptr;
        GLenum dataType;
        GLenum internalFormat;
        GLenum format;

        if (isHDR) {
            // Load HDR image as float
            float* hdrData = stbi_loadf(imagePath.c_str(), &width, &height, &nrChannels, 0);
            if (!hdrData) {
                spdlog::error("RenderingDeviceGLES3::CreateCubemapFromCross Failed to load HDR cross cubemap: {}", crossPath);
                return 0;
            }
            data           = hdrData;
            dataType       = GL_FLOAT;
            internalFormat = (nrChannels == 3) ? GL_RGB16F : GL_RGBA16F;
            format         = (nrChannels == 3) ? GL_RGB : GL_RGBA;
        } else {
            // Load LDR image as unsigned byte
            unsigned char* ldrData = stbi_load(imagePath.c_str(), &width, &height, &nrChannels, 0);
            if (!ldrData) {
                spdlog::error("RenderingDeviceGLES3::CreateCubemapFromCross Failed to load cross cubemap: {}", crossPath);
                return 0;
            }
            data           = ldrData;
            dataType       = GL_UNSIGNED_BYTE;
            internalFormat = (nrChannels == 3) ? GL_RGB8 : GL_RGBA8;
            format         = (nrChannels == 3) ? GL_RGB : GL_RGBA;
        }

        // Validate cross layout (3:4 or 4:3 aspect ratio)
        int faceSize         = 0;
        bool isVerticalCross = false;

        if (width == height * 3 / 4) {
            // Vertical cross: width = 3 * faceSize, height = 4 * faceSize
            faceSize        = width / 3;
            isVerticalCross = true;
        } else if (height == width * 3 / 4) {
            // Horizontal cross: width = 4 * faceSize, height = 3 * faceSize
            faceSize        = height / 3;
            isVerticalCross = false;
        } else {
            spdlog::error("RenderingDeviceGLES3::CreateCubemapFromCross Invalid cross cubemap dimensions: {}x{}", width, height);
            stbi_image_free(data);
            return 0;
        }

        // Create cubemap texture
        Uint32 textureID;
        glGenTextures(1, &textureID);
        glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

        // Extract faces from cross layout
        // Vertical cross layout:
        //       [+Y]
        //   [-X][+Z][+X][-Z]
        //       [-Y]
        //
        // Face positions (x, y) in grid coordinates:
        struct FacePos {
            int x, y;
        };
        FacePos facePositions[6];

        if (isVerticalCross) {
            facePositions[0] = {2, 1}; // +X (right)
            facePositions[1] = {0, 1}; // -X (left)
            facePositions[2] = {1, 0}; // +Y (top)
            facePositions[3] = {1, 2}; // -Y (bottom)
            facePositions[4] = {1, 1}; // +Z (front)
            facePositions[5] = {3, 1}; // -Z (back) - wraps or use position 3,1 if 4-wide
        } else {
            // Horizontal cross:
            //       [+Y]
            // [-X]  [+Z]  [+X]  [-Z]
            //       [-Y]
            facePositions[0] = {2, 1}; // +X (right)
            facePositions[1] = {0, 1}; // -X (left)
            facePositions[2] = {1, 0}; // +Y (top)
            facePositions[3] = {1, 2}; // -Y (bottom)
            facePositions[4] = {1, 1}; // +Z (front)
            facePositions[5] = {3, 1}; // -Z (back)
        }

        // Allocate buffer for face data
        size_t faceDataSize = faceSize * faceSize * nrChannels;
        if (isHDR) {
            faceDataSize *= sizeof(float);
        }
        void* faceData = malloc(faceDataSize);

        for (int face = 0; face < 6; face++) {
            int startX = facePositions[face].x * faceSize;
            int startY = facePositions[face].y * faceSize;

            // Extract face from cross image
            if (isHDR) {
                float* src = (float*) data;
                float* dst = (float*) faceData;
                for (int y = 0; y < faceSize; y++) {
                    for (int x = 0; x < faceSize; x++) {
                        int srcIdx = ((startY + y) * width + (startX + x)) * nrChannels;
                        int dstIdx = (y * faceSize + x) * nrChannels;
                        for (int c = 0; c < nrChannels; c++) {
                            dst[dstIdx + c] = src[srcIdx + c];
                        }
                    }
                }
            } else {
                unsigned char* src = (unsigned char*) data;
                unsigned char* dst = (unsigned char*) faceData;
                for (int y = 0; y < faceSize; y++) {
                    for (int x = 0; x < faceSize; x++) {
                        int srcIdx = ((startY + y) * width + (startX + x)) * nrChannels;
                        int dstIdx = (y * faceSize + x) * nrChannels;
                        for (int c = 0; c < nrChannels; c++) {
                            dst[dstIdx + c] = src[srcIdx + c];
                        }
                    }
                }
            }

            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, 0, internalFormat, faceSize, faceSize, 0, format, dataType, faceData);
        }

        free(faceData);
        stbi_image_free(data);

        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
        glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

        spdlog::info("RenderingDeviceGLES3::CreateCubemapFromCross Loaded {} cross cubemap: {} ({}x{}, face size: {})",
                     isHDR ? "HDR" : "LDR",
                     crossPath,
                     width,
                     height,
                     faceSize);

        return textureID;
    }

    bool RenderingDeviceGLES3::CreateDefaultSkybox() {
        default_skybox_cubemap = CreateCubemapFromCross("textures/sky.png");

        if (default_skybox_cubemap == 0) {
            spdlog::warn("RenderingDeviceGLES3::CreateDefaultSkybox Failed to load textures/sky.png, creating procedural skybox");

            glGenTextures(1, &default_skybox_cubemap);
            glBindTexture(GL_TEXTURE_CUBE_MAP, default_skybox_cubemap);

            const int size = 512;
            std::vector<unsigned char> data(size * size * 3);

            for (int face = 0; face < 6; face++) {
                // Simple gradient skybox
                for (int y = 0; y < size; y++) {
                    for (int x = 0; x < size; x++) {
                        int idx = (y * size + x) * 3;
                        float t = (float) y / size;

                        // Sky blue to horizon gradient
                        data[idx + 0] = (unsigned char) (135 * (1.0f - t) + 200 * t); // R
                        data[idx + 1] = (unsigned char) (206 * (1.0f - t) + 220 * t); // G
                        data[idx + 2] = (unsigned char) (235 * (1.0f - t) + 240 * t); // B
                    }
                }

                glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, 0, GL_RGB, size, size, 0, GL_RGB, GL_UNSIGNED_BYTE, data.data());
            }

            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
        }

        // Create skybox cube mesh (shared for both loaded and procedural skybox)
        std::vector<float> vertices = {
            -1.0f,  1.0f, -1.0f,
            -1.0f, -1.0f, -1.0f,
             1.0f, -1.0f, -1.0f,
             1.0f, -1.0f, -1.0f,
             1.0f,  1.0f, -1.0f,
            -1.0f,  1.0f, -1.0f,

            -1.0f, -1.0f,  1.0f,
            -1.0f, -1.0f, -1.0f,
            -1.0f,  1.0f, -1.0f,
            -1.0f,  1.0f, -1.0f,
            -1.0f,  1.0f,  1.0f,
            -1.0f, -1.0f,  1.0f,

             1.0f, -1.0f, -1.0f,
             1.0f, -1.0f,  1.0f,
             1.0f,  1.0f,  1.0f,
             1.0f,  1.0f,  1.0f,
             1.0f,  1.0f, -1.0f,
             1.0f, -1.0f, -1.0f,

            -1.0f, -1.0f,  1.0f,
            -1.0f,  1.0f,  1.0f,
             1.0f,  1.0f,  1.0f,
             1.0f,  1.0f,  1.0f,
             1.0f, -1.0f,  1.0f,
            -1.0f, -1.0f,  1.0f,

            -1.0f,  1.0f, -1.0f,
             1.0f,  1.0f, -1.0f,
             1.0f,  1.0f,  1.0f,
             1.0f,  1.0f,  1.0f,
            -1.0f,  1.0f,  1.0f,
            -1.0f,  1.0f, -1.0f,

            -1.0f, -1.0f, -1.0f,
            -1.0f, -1.0f,  1.0f,
             1.0f, -1.0f, -1.0f,
             1.0f, -1.0f, -1.0f,
            -1.0f, -1.0f,  1.0f,
             1.0f, -1.0f,  1.0f
        };
std::vector<unsigned int> indices = {
     0,  1,  2,  3,  4,  5,      // -Z
     6,  7,  8,  9, 10, 11,      // -X
    12, 13, 14, 15, 16, 17,      // +X
    18, 19, 20, 21, 22, 23,      // +Z
    24, 25, 26, 27, 28, 29,      // +Y
    30, 31, 32, 33, 34, 35       // -Y
};

        VertexLayout layout;
        layout.elements = {
            {0, 3, EDataType::FLOAT, false, 0}  // position
        };

        skybox_mesh = CreateMeshFromData(layout, vertices, indices);

        return true;
    }

    std::shared_ptr<Shader> RenderingDeviceGLES3::GetDefaultSkyboxShader() const {
        return skybox_shader;
    }

    std::shared_ptr<Mesh> RenderingDeviceGLES3::GetSkyboxMesh() {
        return skybox_mesh;
    }

    Uint32 RenderingDeviceGLES3::GetDefaultSkyboxCubemap() const {
        return default_skybox_cubemap;
    }

    RenderingDeviceGLES3::~RenderingDeviceGLES3() {
        if (default_skybox_cubemap) {
            glDeleteTextures(1, &default_skybox_cubemap);
        }

        if (gl_context) {
            SDL_GL_DestroyContext(gl_context);
            gl_context = nullptr;
        }
    }


}; // namespace golias
