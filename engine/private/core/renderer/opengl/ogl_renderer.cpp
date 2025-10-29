#include  "core/renderer/opengl/ogl_renderer.h"
#include  "core/engine.h"

void APIENTRY ogl_validation_layer(
    GLenum source,
    GLenum type,
    GLuint id,
    GLenum severity,
    GLsizei length,
    const GLchar* message,
    const void* userParam) {

    if (severity == GL_DEBUG_SEVERITY_NOTIFICATION)
        return;

    SDL_Log("ValidationLayer Type: 0x%x | Severity: 0x%x | ID: %u | Message: %s", type, severity, id, message);

#if !defined(NDEBUG)
#if defined(_MSC_VER)
#define DEBUG_BREAK() __debugbreak()
#elif defined(__clang__) || defined(__GNUC__)
#define DEBUG_BREAK() __builtin_trap()
#else
#define DEBUG_BREAK() std::abort()
#endif

    if (type == GL_DEBUG_TYPE_ERROR)
        DEBUG_BREAK();

#endif


}


GLuint load_cubemap_from_atlas(const std::string& atlas_path, CubemapOrientation orient = CubemapOrientation::DEFAULT) {
    spdlog::debug("Loading cubemap atlas: {}", atlas_path);

    int W, H, channels;
    unsigned char* pixels = stbi_load(atlas_path.c_str(), &W, &H, &channels, STBI_rgb_alpha);

    if (!pixels) {
        spdlog::error("Failed to load cubemap atlas: {}", atlas_path);
        return 0;
    }

    spdlog::debug("Atlas loaded: {}x{} channels: {}", W, H, channels);

    if (W <= 0 || H <= 0) {
        spdlog::error("Invalid atlas dimensions: {}x{}", W, H);
        stbi_image_free(pixels);
        return 0;
    }

    // Detect layout
    int face_w                                                               = 0, face_h = 0;
    enum Layout { HORIZONTAL, VERTICAL, L_3x2, L_4x3_CROSS, UNKNOWN } layout = UNKNOWN;

    if (W % 6 == 0 && W / 6 == H) {
        layout = HORIZONTAL;
        face_w = W / 6;
        face_h = H;
    } else if (H % 6 == 0 && H / 6 == W) {
        layout = VERTICAL;
        face_w = W;
        face_h = H / 6;
    } else if (W % 3 == 0 && H % 2 == 0 && W / 3 == H / 2) {
        layout = L_3x2;
        face_w = W / 3;
        face_h = H / 2;
    } else if (W % 4 == 0 && H % 3 == 0 && W / 4 == H / 3) {
        layout = L_4x3_CROSS;
        face_w = W / 4;
        face_h = H / 3;
    } else {
        spdlog::error("Unknown atlas layout: {}x{}", W, H);
        stbi_image_free(pixels);
        return 0;
    }

    if (face_w <= 0 || face_h <= 0) {
        spdlog::error("Invalid face dimensions: {}x{}", face_w, face_h);
        stbi_image_free(pixels);
        return 0;
    }

    spdlog::debug("Detected layout: {}, Face size: {}x{}", static_cast<int>(layout), face_w, face_h);

    // Define face rectangles based on layout
    struct Rect {
        int x, y, w, h;
    };
    std::array<Rect, 6> face_rects;

    if (layout == HORIZONTAL) {
        for (int i = 0; i < 6; ++i) {
            face_rects[i] = {i * face_w, 0, face_w, face_h};
        }
    } else if (layout == VERTICAL) {
        for (int i = 0; i < 6; ++i) {
            face_rects[i] = {0, i * face_h, face_w, face_h};
        }
    } else if (layout == L_3x2) {
        face_rects[0] = {0, 0, face_w, face_h}; // +X
        face_rects[1] = {1 * face_w, 0, face_w, face_h}; // -X
        face_rects[2] = {2 * face_w, 0, face_w, face_h}; // +Y
        face_rects[3] = {0, 1 * face_h, face_w, face_h}; // -Y
        face_rects[4] = {1 * face_w, 1 * face_h, face_w, face_h}; // +Z
        face_rects[5] = {2 * face_w, 1 * face_h, face_w, face_h}; // -Z
    } else {
        // L_4x3_CROSS
        face_rects[0] = {2 * face_w, 1 * face_h, face_w, face_h}; // +X
        face_rects[1] = {0, 1 * face_h, face_w, face_h}; // -X
        face_rects[2] = {1 * face_w, 0, face_w, face_h}; // +Y
        face_rects[3] = {1 * face_w, 2 * face_h, face_w, face_h}; // -Y
        face_rects[4] = {1 * face_w, 1 * face_h, face_w, face_h}; // +Z
        face_rects[5] = {3 * face_w, 1 * face_h, face_w, face_h}; // -Z
    }

    // Apply orientation adjustments
    switch (orient) {
    case CubemapOrientation::TOP:
        std::swap(face_rects[2], face_rects[3]); // +Y <-> -Y
        break;
    case CubemapOrientation::BOTTOM:
        std::swap(face_rects[2], face_rects[3]);
        break;
    case CubemapOrientation::FLIP_X:
        std::swap(face_rects[0], face_rects[1]);
        std::swap(face_rects[4], face_rects[5]);
        break;
    case CubemapOrientation::FLIP_Y:
        std::swap(face_rects[2], face_rects[3]);
        std::swap(face_rects[4], face_rects[5]);
        break;
    default:
        break;
    }

    // Validate rectangles
    for (int i = 0; i < 6; ++i) {
        const auto& r = face_rects[i];
        if (r.x < 0 || r.y < 0 || r.x + r.w > W || r.y + r.h > H) {
            spdlog::error("Face rect {} out of bounds: x={} y={} w={} h={} (atlas: {}x{})",
                          i, r.x, r.y, r.w, r.h, W, H);
            stbi_image_free(pixels);
            return 0;
        }
    }

    // Create cubemap texture
    GLuint texture_id;
    glGenTextures(1, &texture_id);
    glBindTexture(GL_TEXTURE_CUBE_MAP, texture_id);

    constexpr int BYTES_PER_PIXEL = 4;
    int pitch                     = W * BYTES_PER_PIXEL;
    std::vector<unsigned char> face_data(face_w * face_h * BYTES_PER_PIXEL);

    // Extract and upload each face
    for (int i = 0; i < 6; ++i) {
        const auto& r = face_rects[i];

        // Copy face data row by row
        for (int y = 0; y < r.h; ++y) {
            int row            = r.y + y;
            unsigned char* src = pixels + (row * pitch) + (r.x * BYTES_PER_PIXEL);
            unsigned char* dst = face_data.data() + (y * r.w * BYTES_PER_PIXEL);
            std::memcpy(dst, src, r.w * BYTES_PER_PIXEL);
        }

        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGBA,
                     r.w, r.h, 0, GL_RGBA, GL_UNSIGNED_BYTE, face_data.data());
    }

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

    stbi_image_free(pixels);

    spdlog::info("Loaded cubemap atlas {} ({}x{}) Layout {} Face {}x{} Texture ID: {}",
                 atlas_path, W, H, static_cast<int>(layout), face_w, face_h, texture_id);

    return texture_id;
}


WorldEnvironment* OpenGLRenderer::create_skybox_from_atlas(const std::string& atlas_path,
                                                           CubemapOrientation orient,
                                                           float brightness) {

    WorldEnvironment* world_environment = new WorldEnvironment();

    constexpr float skybox_vertices[] = {
        // positions
        -1.0f, 1.0f, -1.0f,
        -1.0f, -1.0f, -1.0f,
        1.0f, -1.0f, -1.0f,
        1.0f, -1.0f, -1.0f,
        1.0f, 1.0f, -1.0f,
        -1.0f, 1.0f, -1.0f,

        -1.0f, -1.0f, 1.0f,
        -1.0f, -1.0f, -1.0f,
        -1.0f, 1.0f, -1.0f,
        -1.0f, 1.0f, -1.0f,
        -1.0f, 1.0f, 1.0f,
        -1.0f, -1.0f, 1.0f,

        1.0f, -1.0f, -1.0f,
        1.0f, -1.0f, 1.0f,
        1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, -1.0f,
        1.0f, -1.0f, -1.0f,

        -1.0f, -1.0f, 1.0f,
        -1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, 1.0f,
        1.0f, -1.0f, 1.0f,
        -1.0f, -1.0f, 1.0f,

        -1.0f, 1.0f, -1.0f,
        1.0f, 1.0f, -1.0f,
        1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, 1.0f,
        -1.0f, 1.0f, 1.0f,
        -1.0f, 1.0f, -1.0f,

        -1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f, 1.0f,
        1.0f, -1.0f, -1.0f,
        1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f, 1.0f,
        1.0f, -1.0f, 1.0f
    };

    const std::vector<unsigned int> indices = {
        0, 1, 2, 2, 3, 0,
        4, 5, 6, 6, 7, 4,
        8, 9, 10, 10, 11, 8,
        12, 13, 14, 14, 15, 12,
        16, 17, 18, 18, 19, 16,
        20, 21, 22, 22, 23, 20
    };

    world_environment->vertex_buffer = this->allocate_gpu_buffer(GpuBufferType::VERTEX);
    world_environment->vertex_buffer->upload(skybox_vertices, sizeof(skybox_vertices));

    world_environment->index_buffer = this->allocate_gpu_buffer(GpuBufferType::INDEX);
    world_environment->index_buffer->upload(indices.data(), indices.size());

    std::vector<VertexAttribute> attributes = {
        {0, 3, DataType::FLOAT, false, 0}
    };

    world_environment->vertex_layout = this->create_vertex_layout(
        world_environment->vertex_buffer.get(),
        world_environment->index_buffer.get(),
        attributes,
        3 * sizeof(float)
        );


    spdlog::info("Skybox geometry initialized");

    GLuint cubemap = load_cubemap_from_atlas(atlas_path, orient);

    if (cubemap == 0) {
        spdlog::error("Failed to create skybox from atlas - loading failed");
        return world_environment;
    }

    world_environment->texture = cubemap;
    spdlog::info("Skybox created from atlas successfully");

    _textures[atlas_path] = cubemap;

    instance_buffer    = allocate_gpu_buffer(GpuBufferType::STORAGE);
    size_t buffer_size = max_instances * sizeof(glm::mat4);
    instance_buffer->upload(nullptr, buffer_size);

    return world_environment;
}

void OpenGLRenderer::setup_instance_matrix_attribute(GpuVertexLayout* vao) {
    vao->bind();
    instance_buffer->bind();

    for (int i = 0; i < 4; i++) {
        GLuint location = 3 + i;
        glEnableVertexAttribArray(location);
        glVertexAttribPointer(location, 4, GL_FLOAT, GL_FALSE,
                              sizeof(glm::mat4),
                              (void*) (i * sizeof(glm::vec4)));
        glVertexAttribDivisor(location, 1);
    }

    vao->unbind();
}

void OpenGLRenderer::setup_lights(const std::vector<DirectionalLight>& directional_lights,
                                  const std::vector<std::pair<Transform3D, SpotLight>>& spot_lights) {
    _default_shader->set_value("numDirLights", static_cast<int>(directional_lights.size()));
    if (!directional_lights.empty()) {
        for (int i = 0; i < static_cast<int>(directional_lights.size()); ++i) {
            _default_shader->set_value(fmt::format("dirLights[{}].direction", i),
                                       directional_lights[i].direction);
            _default_shader->set_value(fmt::format("dirLights[{}].color", i),
                                       directional_lights[i].color * directional_lights[i].intensity);
            _default_shader->set_value(fmt::format("dirLights[{}].cast_shadows", i),
                                       directional_lights[i].castShadows ? 1 : 0);
        }
    }

    _default_shader->set_value("numSpotLights", static_cast<int>(spot_lights.size()));
    if (!spot_lights.empty()) {
        for (int i = 0; i < static_cast<int>(spot_lights.size()); ++i) {
            const auto& [transform, light] = spot_lights[i];
            _default_shader->set_value(fmt::format("spotLights[{}].position", i),
                                       transform.position);
            _default_shader->set_value(fmt::format("spotLights[{}].direction", i),
                                       light.direction);
            _default_shader->set_value(fmt::format("spotLights[{}].color", i),
                                       light.color * light.intensity);
            _default_shader->set_value(fmt::format("spotLights[{}].inner_cut_off", i),
                                       glm::cos(glm::radians(light.cutOff)));
            _default_shader->set_value(fmt::format("spotLights[{}].outer_cut_off", i),
                                       glm::cos(glm::radians(light.outerCutOff)));
        }
    }
}

GLuint OpenGLRenderer::create_gl_texture(const unsigned char* data, int w, int h, int channels) {
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

OpenGLRenderer::~OpenGLRenderer() {
    OpenGLRenderer::cleanup();
}


bool OpenGLRenderer::initialize(int w, int h, SDL_Window* window) {

    _window = window;

#if defined(SDL_PLATFORM_IOS) || defined(SDL_PLATFORM_ANDROID) || defined(SDL_PLATFORM_EMSCRIPTEN)

    /* GLES 3.0 -> GLSL: 300 */
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);

#elif defined(SDL_PLATFORM_WINDOWS) || defined(SDL_PLATFORM_LINUX) || defined(SDL_PLATFORM_MACOS)

    /* OPENGL 3.3 -> GLSL: 330*/
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

#if defined(SDL_PLATFORM_MACOS)
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG);
#endif


#endif

    SDL_GLContext glContext = SDL_GL_CreateContext(GEngine->get_window());

    if (!glContext) {
        spdlog::critical("Failed to create GL context: {}", SDL_GetError());
        return false;
    }


#if defined(SDL_PLATFORM_IOS) || defined(SDL_PLATFORM_ANDROID) || defined(SDL_PLATFORM_EMSCRIPTEN)

    if (!gladLoadGLES2Loader((GLADloadproc) SDL_GL_GetProcAddress)) {
        spdlog::critical("Failed to initialize GLAD (GLES_FUNCTIONS)");
        return false;
    }

#else

    if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(SDL_GL_GetProcAddress))) {
        spdlog::critical("Failed to initialize GLAD (GL_FUNCTIONS)");
        return false;
    }

#endif
    _context = glContext;

    SDL_GL_SetSwapInterval(0);

    width  = w;
    height = h;

    GLint num_extensions = 0;
    std::vector<std::string_view> extensions;
    glGetIntegerv(GL_NUM_EXTENSIONS, &num_extensions);
    extensions.reserve(num_extensions);

    bool khr_debug_found = false;

    for (GLuint i = 0; i < num_extensions; i++) {
        const char* ext = reinterpret_cast<const char*>(glGetStringi(GL_EXTENSIONS, i));


        if (SDL_strcasecmp(ext, "GL_KHR_debug") == 0) {
            spdlog::debug("KHR_debug extension supported, enabling validation layers");
            glEnable(GL_DEBUG_OUTPUT);
            glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
            glDebugMessageCallback(ogl_validation_layer, nullptr);
            khr_debug_found = true;
            break;
        }
    }

    if (!khr_debug_found) {
        spdlog::warn("KHR_debug extensions not supported, validation layers disabled");
    }

    int major, minor;
    SDL_GL_GetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, &major);
    SDL_GL_GetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, &minor);

    const char* glsl_version = reinterpret_cast<const char*>(glGetString(GL_SHADING_LANGUAGE_VERSION));
    const char* gl_vendor    = reinterpret_cast<const char*>(glGetString(GL_VENDOR));
    const char* gl_renderer  = reinterpret_cast<const char*>(glGetString(GL_RENDERER));

    spdlog::info("GLSL Version: {}", glsl_version);
    spdlog::info("OpenGL Version: {}.{}", major, minor);
    spdlog::info("OpenGL Vendor: {}", gl_vendor);
    spdlog::info("OpenGL Renderer: {}", gl_renderer);

    glEnable(GL_DEPTH_TEST);
    glViewport(0, 0, width, height);

    _default_shader = std::make_unique<OpenglShader>("shaders/opengl/default.vert", "shaders/opengl/default.frag");
    _default_shader->set_value("USE_IBL", false);

    _shadow_shader      = std::make_unique<OpenglShader>("shaders/opengl/shadow.vert", "shaders/opengl/shadow.frag");
    _environment_shader = std::make_unique<OpenglShader>("shaders/opengl/skybox.vert", "shaders/opengl/skybox.frag");

    FramebufferSpecification spec;
    spec.height      = 8192;
    spec.width       = 8192;
    spec.attachments = {
        {FramebufferTextureFormat::DEPTH_COMPONENT}
    };

    shadow_map_fbo     = std::make_shared<OpenGLFramebuffer>(spec);
    _world_environment = create_skybox_from_atlas("res/environment_sky.png", CubemapOrientation::DEFAULT, 1.0f);
    return true;
}

std::shared_ptr<GpuBuffer> OpenGLRenderer::allocate_gpu_buffer(GpuBufferType type) {
    return std::make_shared<OpenglGpuBuffer>(type);
}

std::shared_ptr<GpuVertexLayout> OpenGLRenderer::create_vertex_layout(const GpuBuffer* vertex_buffer, const GpuBuffer* index_buffer,
                                                                      const std::vector<VertexAttribute>& attributes, uint32_t stride) {

    return std::make_unique<OpenglGpuVertexLayout>(vertex_buffer, index_buffer, attributes, stride);
}

GLuint OpenGLRenderer::load_texture_from_file(const std::string& path) {
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

GLuint OpenGLRenderer::load_embedded_texture(const unsigned char* buffer, size_t size, const std::string& name) {
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

GLuint OpenGLRenderer::load_texture_from_raw_data(const unsigned char* data, int w, int h, int channels, const std::string& name) {
    std::string key = name.empty() ? "raw_" + std::to_string(reinterpret_cast<size_t>(data)) : name;

    if (auto it = _textures.find(key); it != _textures.end())
        return it->second;

    GLuint texID   = create_gl_texture(data, w, h, channels);
    _textures[key] = texID;
    spdlog::info("Loaded raw Texture: {}, Path {}", texID, key);

    return texID;
}

void OpenGLRenderer::begin_frame() {
    for (auto& [key, batch] : _instanced_batches) {
        batch.clear();
    }

    for (auto& [mesh, batch] : shadow_batches) {
        batch.clear();
    }
}

void OpenGLRenderer::begin_shadow_pass() {
    shadow_map_fbo->bind();
    glEnable(GL_DEPTH_TEST);
    glClear(GL_DEPTH_BUFFER_BIT);
    _shadow_shader->activate();
}

void OpenGLRenderer::render_shadow_pass(const glm::mat4& light_space_matrix) {
    _shadow_shader->set_value("LIGHT_MATRIX", light_space_matrix, 1);

    for (auto& [mesh_ptr, batch] : shadow_batches) {
        if (batch.model_matrices.empty())
            continue;

        instance_buffer->bind();
        instance_buffer->upload(batch.model_matrices.data(),
                                batch.model_matrices.size() * sizeof(glm::mat4));

        setup_instance_matrix_attribute(batch.mesh->vertex_layout.get());

        batch.mesh->vertex_layout->bind();
        glDrawElementsInstanced(GL_TRIANGLES,
                                batch.mesh->index_count,
                                GL_UNSIGNED_INT,
                                0,
                                batch.model_matrices.size());
        batch.mesh->vertex_layout->unbind();
    }
}

void OpenGLRenderer::end_shadow_pass() {
    shadow_map_fbo->unbind();
    glCullFace(GL_BACK);
    glViewport(0, 0, width, height);
}

void OpenGLRenderer::begin_render_target() {
    glClearColor(_world_environment->color.r, _world_environment->color.g, _world_environment->color.b, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    _default_shader->activate();
    glViewport(0, 0, width, height);
}

void OpenGLRenderer::render_main_target(const Camera3D& camera,
                                        const Transform3D& camera_transform,
                                        const glm::mat4& light_space_matrix,
                                        const std::vector<DirectionalLight>& directional_lights,
                                        const std::vector<std::pair<Transform3D, SpotLight>>& spot_lights) {

    int total_instances = 0;
    int draw_calls      = 0;

    glm::mat4 view       = camera.get_view(camera_transform);
    glm::mat4 projection = camera.get_projection(width, height);

    _default_shader->set_value("VIEW", view);
    _default_shader->set_value("PROJECTION", projection);
    _default_shader->set_value("LIGHT_MATRIX", light_space_matrix);
    _default_shader->set_value("CAMERA_POSITION_WORLD", camera_transform.position);

    setup_lights(directional_lights, spot_lights);

    glActiveTexture(GL_TEXTURE0 + SHADOW_TEXTURE_UNIT);
    glBindTexture(GL_TEXTURE_2D, shadow_map_fbo->get_depth_attachment_id());
    _default_shader->set_value("SHADOW_MAP", SHADOW_TEXTURE_UNIT);

    glActiveTexture(GL_TEXTURE0 + ENVIRONMENT_TEXTURE_UNIT);
    glBindTexture(GL_TEXTURE_CUBE_MAP, _world_environment->texture);
    _default_shader->set_value("ENVIRONMENT_MAP", ENVIRONMENT_TEXTURE_UNIT);

    for (auto& [key, batch] : _instanced_batches) {
        if (batch.model_matrices.empty())
            continue;

        draw_calls++;
        total_instances += batch.model_matrices.size();

        instance_buffer->bind();
        instance_buffer->upload(batch.model_matrices.data(),
                                batch.model_matrices.size() * sizeof(glm::mat4));

        batch.material->bind(_default_shader.get());

        setup_instance_matrix_attribute(batch.mesh->vertex_layout.get());

        batch.mesh->vertex_layout->bind();
        glDrawElementsInstanced(GL_TRIANGLES,
                                batch.mesh->index_count,
                                GL_UNSIGNED_INT,
                                0,
                                batch.model_matrices.size());
        batch.mesh->vertex_layout->unbind();
    }

    // spdlog::info("Frame: {} draw calls, {} instances", draw_calls, total_instances);
}

void OpenGLRenderer::end_render_target() {

}

void OpenGLRenderer::begin_environment_pass() {
    glDepthFunc(GL_LEQUAL);
    _environment_shader->activate();

}

void OpenGLRenderer::render_environment_pass(const Camera3D& camera) {
    auto camera_query                   = GEngine->get_world().query<const Transform3D, const Camera3D>();
    const Transform3D& camera_transform = camera_query.first().get<Transform3D>();

    glm::mat4 view       = glm::mat4(glm::mat3(camera.get_view(camera_transform)));
    glm::mat4 projection = camera.get_projection(width, height);

    _environment_shader->set_value("VIEW", view);
    _environment_shader->set_value("PROJECTION", projection);


    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, _world_environment->texture);
    _environment_shader->set_value("TEXTURE", 0);

    _world_environment->vertex_layout->bind();
    glDrawArrays(GL_TRIANGLES, 0, 36);
    _world_environment->vertex_layout->unbind();

}

void OpenGLRenderer::end_environment_pass() {
    glDepthFunc(GL_LESS);
}

void OpenGLRenderer::add_to_render_batch(const Transform3D& transform, const MeshRef& mesh_ref, const MaterialRef& mat_ref) {
    MeshMaterialKey key{mesh_ref.mesh, mat_ref.material};
    auto& batch = _instanced_batches[key];

    batch.mesh     = mesh_ref.mesh;
    batch.material = mat_ref.material;
    batch.model_matrices.push_back(transform.get_matrix());
}

void OpenGLRenderer::add_to_shadow_batch(const Transform3D& transform, const MeshRef& mesh_ref) {
    auto& batch = shadow_batches[mesh_ref.mesh];
    batch.mesh  = mesh_ref.mesh;
    batch.model_matrices.push_back(transform.get_matrix());
}

void OpenGLRenderer::resize(int w, int h) {
    width  = w;
    height = h;
    glViewport(0, 0, width, height);
}

void OpenGLRenderer::cleanup() {
    // Clean up textures
    for (auto& [key, texID] : _textures)
        glDeleteTextures(1, &texID);

    _textures.clear();

    // TODO: create world enviroment entity
    delete _world_environment;
    _world_environment = nullptr;

    SDL_GL_DestroyContext(_context);

}

void OpenGLRenderer::swap_chain() {
    SDL_GL_SwapWindow(_window);
}
