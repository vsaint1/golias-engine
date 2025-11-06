#include "core/renderer/opengl/ogl_renderer.h"

#include "core/engine.h"

void APIENTRY ogl_validation_layer(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message,
                                   const void* userParam) {

    if (severity == GL_DEBUG_SEVERITY_NOTIFICATION) {
        return;
    }

    SDL_Log("ValidationLayer Type: 0x%x | Severity: 0x%x | ID: %u | Message: %s", type, severity, id, message);

    if (type == GL_DEBUG_TYPE_ERROR) {
        GOLIAS_ASSERT_BREAK();
    }
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
    int face_w = 0, face_h = 0;
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
            spdlog::error("Face rect {} out of bounds: x={} y={} w={} h={} (atlas: {}x{})", i, r.x, r.y, r.w, r.h, W, H);
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
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGBA, r.w, r.h, 0, GL_RGBA, GL_UNSIGNED_BYTE, face_data.data());
    }

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

    stbi_image_free(pixels);

    spdlog::info("Loaded cubemap atlas {} ({}x{}) Layout {} Face {}x{} Texture ID: {}", atlas_path, W, H, static_cast<int>(layout), face_w,
                 face_h, texture_id);

    return texture_id;
}


WorldEnvironment* OpenGLRenderer::create_skybox_from_atlas(const std::string& atlas_path, CubemapOrientation orient, float brightness) {

    WorldEnvironment* world_environment = new WorldEnvironment();

    constexpr float skybox_vertices[] = {
        // positions
        -1.0f, 1.0f,  -1.0f, -1.0f, -1.0f, -1.0f, 1.0f,  -1.0f, -1.0f, 1.0f,  -1.0f, -1.0f, 1.0f,  1.0f,  -1.0f, -1.0f, 1.0f,  -1.0f,

        -1.0f, -1.0f, 1.0f,  -1.0f, -1.0f, -1.0f, -1.0f, 1.0f,  -1.0f, -1.0f, 1.0f,  -1.0f, -1.0f, 1.0f,  1.0f,  -1.0f, -1.0f, 1.0f,

        1.0f,  -1.0f, -1.0f, 1.0f,  -1.0f, 1.0f,  1.0f,  1.0f,  1.0f,  1.0f,  1.0f,  1.0f,  1.0f,  1.0f,  -1.0f, 1.0f,  -1.0f, -1.0f,

        -1.0f, -1.0f, 1.0f,  -1.0f, 1.0f,  1.0f,  1.0f,  1.0f,  1.0f,  1.0f,  1.0f,  1.0f,  1.0f,  -1.0f, 1.0f,  -1.0f, -1.0f, 1.0f,

        -1.0f, 1.0f,  -1.0f, 1.0f,  1.0f,  -1.0f, 1.0f,  1.0f,  1.0f,  1.0f,  1.0f,  1.0f,  -1.0f, 1.0f,  1.0f,  -1.0f, 1.0f,  -1.0f,

        -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, 1.0f,  1.0f,  -1.0f, -1.0f, 1.0f,  -1.0f, -1.0f, -1.0f, -1.0f, 1.0f,  1.0f,  -1.0f, 1.0f};

    const std::vector<unsigned int> indices = {0,  1,  2,  2,  3,  0,  4,  5,  6,  6,  7,  4,  8,  9,  10, 10, 11, 8,
                                               12, 13, 14, 14, 15, 12, 16, 17, 18, 18, 19, 16, 20, 21, 22, 22, 23, 20};

    world_environment->vertex_buffer = this->allocate_gpu_buffer(GpuBufferType::VERTEX);
    world_environment->vertex_buffer->upload(skybox_vertices, sizeof(skybox_vertices));

    world_environment->index_buffer = this->allocate_gpu_buffer(GpuBufferType::INDEX);
    world_environment->index_buffer->upload(indices.data(), indices.size());

    std::vector<VertexAttribute> attributes = {{0, 3, DataType::FLOAT, false, 0}};

    world_environment->vertex_layout = this->create_vertex_layout(world_environment->vertex_buffer.get(),
                                                                  world_environment->index_buffer.get(), attributes, 3 * sizeof(float));


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
    size_t buffer_size = MAX_INSTANCES * sizeof(glm::mat4);
    instance_buffer->upload(nullptr, buffer_size);


    return world_environment;
}

void OpenGLRenderer::setup_instance_matrix_attribute(GpuVertexLayout* vao) {
    vao->bind();
    instance_buffer->bind();

    for (int i = 0; i < 4; i++) {
        GLuint location = 3 + i;
        glEnableVertexAttribArray(location);
        glVertexAttribPointer(location, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*) (i * sizeof(glm::vec4)));
        glVertexAttribDivisor(location, 1);
    }

    vao->unbind();
}

void OpenGLRenderer::setup_lights(const std::vector<DirectionalLight3D>& directional_lights,
                                  const std::vector<std::pair<Transform3D, SpotLight3D>>& spot_lights) {
    _default_shader->set_value("numDirLights", static_cast<int>(directional_lights.size()));
    if (!directional_lights.empty()) {
        for (int i = 0; i < static_cast<int>(directional_lights.size()); ++i) {
            _default_shader->set_value(fmt::format("dirLights[{}].direction", i).c_str(), directional_lights[i].direction);
            _default_shader->set_value(fmt::format("dirLights[{}].color", i).c_str(),
                                       directional_lights[i].color * directional_lights[i].intensity);
            _default_shader->set_value(fmt::format("dirLights[{}].cast_shadows", i).c_str(), directional_lights[i].castShadows ? 1 : 0);
        }
    }

    _default_shader->set_value("numSpotLights", static_cast<int>(spot_lights.size()));
    if (!spot_lights.empty()) {
        for (int i = 0; i < static_cast<int>(spot_lights.size()); ++i) {
            const auto& [transform, light] = spot_lights[i];
            _default_shader->set_value(fmt::format("spotLights[{}].position", i).c_str(), transform.position);
            _default_shader->set_value(fmt::format("spotLights[{}].direction", i).c_str(), light.direction);
            _default_shader->set_value(fmt::format("spotLights[{}].color", i).c_str(), light.color * light.intensity);
            _default_shader->set_value(fmt::format("spotLights[{}].inner_cut_off", i).c_str(), glm::cos(glm::radians(light.cutOff)));
            _default_shader->set_value(fmt::format("spotLights[{}].outer_cut_off", i).c_str(), glm::cos(glm::radians(light.outerCutOff)));
        }
    }
}

GLuint OpenGLRenderer::create_gl_texture(const unsigned char* data, int w, int h, int channels) {
    GLuint texID = 0;
    glGenTextures(1, &texID);
    glBindTexture(GL_TEXTURE_2D, texID);

    GLenum format = GL_RGB;
    if (channels == 1) {
        format = GL_RED;
    } else if (channels == 3) {
        format = GL_RGB;
    } else if (channels == 4) {
        format = GL_RGBA;
    }

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

    auto& viewport  = GEngine->get_config().get_viewport();
    _virtual_width  = viewport.width;
    _virtual_height = viewport.height;

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
    glViewport(0, 0, _virtual_width, _virtual_height);

    _default_shader = std::make_unique<OpenglShader>("shaders/opengl/default.vert", "shaders/opengl/default.frag");
    _default_shader->set_value("USE_IBL", false);

    _shadow_shader      = std::make_unique<OpenglShader>("shaders/opengl/shadow.vert", "shaders/opengl/shadow.frag");
    _environment_shader = std::make_unique<OpenglShader>("shaders/opengl/skybox.vert", "shaders/opengl/skybox.frag");

    FramebufferSpecification depth_spec;
    depth_spec.height      = 8192;
    depth_spec.width       = 8192;
    depth_spec.attachments = {{FramebufferTextureFormat::DEPTH_COMPONENT}};

    shadow_map_fbo = std::make_shared<OpenGLFramebuffer>(depth_spec);

    FramebufferSpecification main_spec;

    main_spec.width       = _virtual_width;
    main_spec.height      = _virtual_height;
    main_spec.attachments = {{FramebufferTextureFormat::RGBA8}, {FramebufferTextureFormat::DEPTH24STENCIL8}};

    main_fbo = std::make_shared<OpenGLFramebuffer>(main_spec);

    initialize_2d_rendering();

    _world_environment = create_skybox_from_atlas("res/environment_sky.png", CubemapOrientation::DEFAULT, 1.0f);
    return true;
}

std::shared_ptr<GpuImage> OpenGLRenderer::create_texture_2d(const std::string& path, const TextureDesc& desc) {
    // return std::make_shared<OpenglGpuImage>(path, desc);
    return nullptr;
}

std::shared_ptr<GpuBuffer> OpenGLRenderer::allocate_gpu_buffer(GpuBufferType type) {
    return std::make_shared<OpenglGpuBuffer>(type);
}

std::shared_ptr<GpuVertexLayout> OpenGLRenderer::create_vertex_layout(const GpuBuffer* vertex_buffer, const GpuBuffer* index_buffer,
                                                                      const std::vector<VertexAttribute>& attributes, Uint32 stride) {

    return std::make_unique<OpenglGpuVertexLayout>(vertex_buffer, index_buffer, attributes, stride);
}

GLuint OpenGLRenderer::load_texture_from_file(const std::string& path) {
    if (auto it = _textures.find(path); it != _textures.end()) {
        return it->second;
    }

    int w, h, channels;
    unsigned char* data = stbi_load(path.c_str(), &w, &h, &channels, 0);
    if (!data) {
        spdlog::error("Failed to load texture: {}", path);
        return 0;
    }

    GLuint texID = create_gl_texture(data, w, h, channels);
    stbi_image_free(data);

    _textures[path]            = texID;
    _texture_info_cache[texID] = {texID, w, h}; // Cache texture dimensions
    spdlog::info("Loaded Texture: {}", path);
    return texID;
}

GLuint OpenGLRenderer::load_embedded_texture(const unsigned char* buffer, size_t size, const std::string& name) {
    std::string key = name.empty() ? "embedded_tex_" + std::to_string(reinterpret_cast<size_t>(buffer)) : name;

    if (auto it = _textures.find(key); it != _textures.end()) {
        return it->second;
    }

    int w, h, channels;
    unsigned char* data = stbi_load_from_memory(buffer, (int) size, &w, &h, &channels, 0);
    if (!data) {
        spdlog::error("Failed to load texture from memory: {}", key);
        return 0;
    }

    GLuint texID = create_gl_texture(data, w, h, channels);
    stbi_image_free(data);

    _textures[key]             = texID;
    _texture_info_cache[texID] = {texID, w, h}; // Cache texture dimensions
    spdlog::info("Loaded embedded Texture: {}, Path {}", texID, key);
    return texID;
}

void OpenGLRenderer::begin_frame() {
    for (auto& [key, batch] : _instanced_batches) {
        batch.clear();
    }

    for (auto& [mesh, batch] : _shadow_batches) {
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

    for (auto& [mesh_ptr, batch] : _shadow_batches) {
        if (batch.model_matrices.empty()) {
            continue;
        }

        instance_buffer->bind();
        instance_buffer->upload(batch.model_matrices.data(), batch.model_matrices.size() * sizeof(glm::mat4));

        setup_instance_matrix_attribute(batch.mesh->vertex_layout.get());

        batch.mesh->vertex_layout->bind();
        glDrawElementsInstanced(GL_TRIANGLES, batch.mesh->index_count, GL_UNSIGNED_INT, 0, batch.model_matrices.size());
    }

    glBindVertexArray(0);
}

void OpenGLRenderer::end_shadow_pass() {
    shadow_map_fbo->unbind();
    glCullFace(GL_BACK);
    glViewport(0, 0, _virtual_width, _virtual_height);
}

void OpenGLRenderer::begin_render_target() {
    main_fbo->bind();

    int render_w = (_render_width > 0 && _render_height > 0) ? _render_width : _virtual_width;
    int render_h = (_render_width > 0 && _render_height > 0) ? _render_height : _virtual_height;

    glViewport(0, 0, render_w, render_h);

    glClearColor(_world_environment->color.r, _world_environment->color.g, _world_environment->color.b, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ZERO);
    glCullFace(GL_BACK);
    glEnable(GL_CULL_FACE);
    glDisable(GL_SCISSOR_TEST);
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    glBindVertexArray(0);
    glUseProgram(0);

    // TODO: Reset other states as needed

    glActiveTexture(GL_TEXTURE0);

    _default_shader->activate();
}

void OpenGLRenderer::render_main_target(const Camera3D& camera, const Transform3D& camera_transform, const glm::mat4& light_space_matrix,
                                        const std::vector<DirectionalLight3D>& directional_lights,
                                        const std::vector<std::pair<Transform3D, SpotLight3D>>& spot_lights) {

    int total_instances = 0;
    int draw_calls      = 0;

    int render_w = (_render_width > 0 && _render_height > 0) ? _render_width : _virtual_width;
    int render_h = (_render_width > 0 && _render_height > 0) ? _render_height : _virtual_height;

    glm::mat4 view       = camera.get_view(camera_transform);
    glm::mat4 projection = camera.get_projection(render_w, render_h);

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
        if (batch.model_matrices.empty()) {
            continue;
        }

        draw_calls++;
        total_instances += batch.model_matrices.size();

        instance_buffer->bind();
        instance_buffer->upload(batch.model_matrices.data(), batch.model_matrices.size() * sizeof(glm::mat4));

        batch.material->bind(_default_shader.get());

        setup_instance_matrix_attribute(batch.mesh->vertex_layout.get());

        batch.mesh->vertex_layout->bind();
        glDrawElementsInstanced(GL_TRIANGLES, batch.mesh->index_count, GL_UNSIGNED_INT, 0, batch.model_matrices.size());

        batch.mesh->vertex_layout->unbind();
    }

    glBindVertexArray(0); // just for safety
}

void OpenGLRenderer::end_render_target() {
    // Don't blit yet - keep the framebuffer bound so 2D can render on top
    // The blit will happen after 2D rendering is complete
    // This allows compositing 2D on top of 3D
}


void OpenGLRenderer::begin_environment_pass() {
    glDepthFunc(GL_LEQUAL);
    _environment_shader->activate();
}

void OpenGLRenderer::render_environment_pass(const Camera3D& camera) {
    auto camera_query                   = GEngine->get_world().query<const Transform3D, const Camera3D>();
    const Transform3D& camera_transform = camera_query.first().get<Transform3D>();

    glm::mat4 view       = glm::mat4(glm::mat3(camera.get_view(camera_transform)));
    glm::mat4 projection = camera.get_projection(_virtual_width, _virtual_height);

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
    auto& batch = _shadow_batches[mesh_ref.mesh];
    batch.mesh  = mesh_ref.mesh;
    batch.model_matrices.push_back(transform.get_matrix());
}


void OpenGLRenderer::set_render_resolution(int width, int height) {
    if (width <= 0 || height <= 0) {
        spdlog::warn("Invalid render resolution: {}x{}", width, height);
        return;
    }

    _render_width  = width;
    _render_height = height;


    main_fbo->resize(width, height);

    spdlog::debug("Set render resolution to {}x{} (Framebuffer resized, will scale to {}x{} on blit)", _render_width, _render_height,
                  _virtual_width, _virtual_height);
}

void OpenGLRenderer::cleanup() {

    for (auto& [key, texID] : _textures) {
        glDeleteTextures(1, &texID);
    }

    _textures.clear();

    for (auto& [key, cached_tex] : _cached_text_textures) {
        glDeleteTextures(1, &cached_tex.texture_id);
    }
    _cached_text_textures.clear();

    // Clean up fonts
    for (auto& [name, font] : _fonts) {
        if (font) {
            TTF_CloseFont(font);
        }
    }
    _fonts.clear();

    // TODO: create world enviroment entity
    delete _world_environment;
    _world_environment = nullptr;

    SDL_GL_DestroyContext(_context);
}

void OpenGLRenderer::swap_chain() {
    glBindFramebuffer(GL_READ_FRAMEBUFFER, main_fbo->get_fbo_id());
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

    int render_w = (_render_width > 0 && _render_height > 0) ? _render_width : _virtual_width;
    int render_h = (_render_width > 0 && _render_height > 0) ? _render_height : _virtual_height;

    auto window_size = GEngine->get_config().get_window();

    glBlitFramebuffer(0, 0, render_w, render_h, // source (main_fbo at render resolution)
                      0, 0, window_size.width, window_size.height, // destination (screen at window resolution)
                      GL_COLOR_BUFFER_BIT, // copy color buffer
                      GL_NEAREST // nearest neighbor
    );

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    SDL_GL_SwapWindow(_window);
}

// ============================================================================
// 2D Rendering Implementation
// ============================================================================

void OpenGLRenderer::initialize_2d_rendering() {

    spdlog::info("Initializing 2D rendering system...");

    _shader_2d = std::make_unique<OpenglShader>("shaders/opengl/default_2d.vert", "shaders/opengl/default_2d.frag");

    _vertex_buffer_2d = allocate_gpu_buffer(GpuBufferType::VERTEX);
    _index_buffer_2d  = allocate_gpu_buffer(GpuBufferType::INDEX);

    _vertex_buffer_2d->upload(nullptr, MAX_VERTICES_2D * sizeof(Vertex2D));
    _index_buffer_2d->upload(nullptr, MAX_INDICES_2D * sizeof(uint32_t));

    std::vector<VertexAttribute> attributes = {
        {0, 2, DataType::FLOAT, false, offsetof(Vertex2D, position)}, // position
        {1, 2, DataType::FLOAT, false, offsetof(Vertex2D, tex_coord)}, // tex_coord
        {2, 4, DataType::FLOAT, false, offsetof(Vertex2D, color)} // color
    };

    _vertex_layout_2d = create_vertex_layout(_vertex_buffer_2d.get(), _index_buffer_2d.get(), attributes, sizeof(Vertex2D));

    _batch_vertices_2d.reserve(MAX_VERTICES_2D);
    _batch_indices_2d.reserve(MAX_INDICES_2D);


    spdlog::info("2D rendering system initialized");
}


void OpenGLRenderer::begin_frame_2d() {

    _draw_commands_2d.clear();
    _batch_vertices_2d.clear();
    _batch_indices_2d.clear();
}

void OpenGLRenderer::end_frame_2d(const Camera2D& camera, const Transform2D& camera_transform) {

    // Render to main_fbo (on top of 3D)
    main_fbo->bind();

    if (!_draw_commands_2d.empty()) {
        glm::mat4 projection      = camera.get_projection_matrix();
        glm::mat4 view            = camera.get_view_matrix();
        glm::mat4 view_projection = projection * view;

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glBlendEquation(GL_FUNC_ADD);

        glDisable(GL_DEPTH_TEST);
        glDepthMask(GL_FALSE);
        glDisable(GL_CULL_FACE);
        glDisable(GL_SCISSOR_TEST);

        _shader_2d->activate();
        _shader_2d->set_value("VIEW_PROJECTION", view_projection);

        uint32_t vertex_offset = 0;
        uint32_t index_offset  = 0;

        int draw_call = 0;
        for (size_t i = 0; i < _draw_commands_2d.size(); ++i) {
            const auto& cmd = _draw_commands_2d[i];

            DrawMode2D current_mode  = cmd.mode;
            GLuint current_texture   = cmd.texture_id;
            bool current_use_texture = cmd.use_texture;

            _batch_vertices_2d.clear();
            _batch_indices_2d.clear();

            size_t batch_start = i;

            for (size_t j = i; j < _draw_commands_2d.size(); ++j) {
                const auto& batch_cmd = _draw_commands_2d[j];

                bool can_batch = (batch_cmd.mode == current_mode && batch_cmd.texture_id == current_texture
                                  && batch_cmd.use_texture == current_use_texture);

                if (batch_cmd.mode == DrawMode2D::CIRCLE_FILLED || batch_cmd.mode == DrawMode2D::CIRCLE_OUTLINE) {
                    can_batch = false;
                }

                if (!can_batch && j > i) {
                    break;
                }

                Uint32 base_vertex = _batch_vertices_2d.size();

                _batch_vertices_2d.insert(_batch_vertices_2d.end(), batch_cmd.vertices.begin(), batch_cmd.vertices.end());


                for (auto idx : batch_cmd.indices) {
                    _batch_indices_2d.push_back(idx + base_vertex);
                }

                i = j;

                if (batch_cmd.mode == DrawMode2D::CIRCLE_FILLED || batch_cmd.mode == DrawMode2D::CIRCLE_OUTLINE) {
                    break;
                }
            }

            if (!_batch_vertices_2d.empty()) {
                render_batched_2d(current_mode, current_texture, current_use_texture, _draw_commands_2d[batch_start]);
            }

            draw_call++;
        }

        // spdlog::debug("2D Rendering complete: {} draw calls", draw_call);
    }

    glBindVertexArray(0);
    glUseProgram(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);


    glActiveTexture(GL_TEXTURE0);

    // Restore 3D rendering state
    glDisable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ZERO); // Reset to default
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glDepthMask(GL_TRUE);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glDisable(GL_SCISSOR_TEST);
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
}


void OpenGLRenderer::render_batched_2d(DrawMode2D mode, GLuint texture_id, bool use_texture, const DrawCommand2D& first_cmd) {
    if (_batch_vertices_2d.empty()) {
        return;
    }

    // Set draw mode
    _shader_2d->set_value("DRAW_MODE", static_cast<int>(mode));
    _shader_2d->set_value("USE_TEXTURE", use_texture);

    if (use_texture && texture_id != 0) {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture_id);
        _shader_2d->set_value("TEXTURE", 0);
    } else {
        // Ensure texture is not bound if not using it
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, 0);
    }

    if (mode == DrawMode2D::CIRCLE_FILLED || mode == DrawMode2D::CIRCLE_OUTLINE) {
        _shader_2d->set_value("CIRCLE_CENTER", first_cmd.position);
        _shader_2d->set_value("CIRCLE_RADIUS", first_cmd.radius);

        if (mode == DrawMode2D::CIRCLE_OUTLINE) {
            _shader_2d->set_value("CIRCLE_THICKNESS", first_cmd.thickness);
        }
    }

    _vertex_buffer_2d->bind();
    _vertex_buffer_2d->upload(_batch_vertices_2d.data(), _batch_vertices_2d.size() * sizeof(Vertex2D));

    _index_buffer_2d->bind();
    _index_buffer_2d->upload(_batch_indices_2d.data(), _batch_indices_2d.size() * sizeof(uint32_t));

    _vertex_layout_2d->bind();
    glDrawElements(GL_TRIANGLES, _batch_indices_2d.size(), GL_UNSIGNED_INT, 0);
    _vertex_layout_2d->unbind();
}


void OpenGLRenderer::draw_rect_2d(const glm::vec2& position, const glm::vec2& size, const glm::vec4& color, bool filled) {
    DrawCommand2D cmd;
    cmd.type        = DrawType2D::RECTANGLE;
    cmd.mode        = filled ? DrawMode2D::FILLED : DrawMode2D::LINE;
    cmd.color       = color;
    cmd.use_texture = false;
    cmd.texture_id  = 0;
    cmd.position    = position;
    cmd.size        = size;
    cmd.filled      = filled;
    cmd.thickness   = 1.0f;

    if (filled) {

        cmd.vertices = {{{position.x, position.y}, {0.0f, 0.0f}, color},
                        {{position.x + size.x, position.y}, {1.0f, 0.0f}, color},
                        {{position.x + size.x, position.y + size.y}, {1.0f, 1.0f}, color},
                        {{position.x, position.y + size.y}, {0.0f, 1.0f}, color}};

        cmd.indices = {0, 1, 2, 2, 3, 0};
    } else {

        float thickness = 1.0f;
        glm::vec2 p1    = position;
        glm::vec2 p2    = position + glm::vec2(size.x, 0);
        glm::vec2 p3    = position + size;
        glm::vec2 p4    = position + glm::vec2(0, size.y);

        // Create 4 thick lines
        auto create_line = [&](const glm::vec2& start, const glm::vec2& end) {
            glm::vec2 dir = glm::normalize(end - start);
            glm::vec2 perp(-dir.y, dir.x);
            glm::vec2 offset = perp * (thickness * 0.5f);

            uint32_t base = cmd.vertices.size();
            cmd.vertices.push_back({start - offset, {0, 0}, color});
            cmd.vertices.push_back({start + offset, {1, 0}, color});
            cmd.vertices.push_back({end + offset, {1, 1}, color});
            cmd.vertices.push_back({end - offset, {0, 1}, color});

            cmd.indices.insert(cmd.indices.end(), {base, base + 1, base + 2, base + 2, base + 3, base});
        };

        create_line(p1, p2);
        create_line(p2, p3);
        create_line(p3, p4);
        create_line(p4, p1);
    }

    _draw_commands_2d.push_back(cmd);
}

void OpenGLRenderer::draw_texture_2d(Uint32 texture_id, const glm::vec2& position, const glm::vec2& size, const Rect2D& src, FlipMode flip,
                                     const glm::vec4& color) {
    DrawCommand2D cmd;
    cmd.type        = DrawType2D::RECTANGLE;
    cmd.mode        = DrawMode2D::FILLED;
    cmd.color       = color;
    cmd.use_texture = true;
    cmd.texture_id  = texture_id;
    cmd.position    = position;
    cmd.size        = size;
    cmd.filled      = true;


    float u_min = 0.0f, v_min = 0.0f;
    float u_max = 1.0f, v_max = 1.0f;

    if (!src.is_zero()) {
        auto it = _texture_info_cache.find(texture_id);
        if (it != _texture_info_cache.end()) {
            int tex_width  = it->second.width;
            int tex_height = it->second.height;

            if (tex_width > 0 && tex_height > 0) {
                u_min = src.x / tex_width;
                v_min = src.y / tex_height;
                u_max = (src.x + src.width) / tex_width;
                v_max = (src.y + src.height) / tex_height;
            }
        }
    }

    if (flip == FlipMode::HORIZONTAL || flip == FlipMode::BOTH) {
        std::swap(u_min, u_max);
    }

    if (flip == FlipMode::VERTICAL || flip == FlipMode::BOTH) {
        std::swap(v_min, v_max);
    }

    cmd.vertices = {{{position.x, position.y}, {u_min, v_min}, color},
                    {{position.x + size.x, position.y}, {u_max, v_min}, color},
                    {{position.x + size.x, position.y + size.y}, {u_max, v_max}, color},
                    {{position.x, position.y + size.y}, {u_min, v_max}, color}};

    cmd.indices = {0, 1, 2, 2, 3, 0};

    _draw_commands_2d.push_back(cmd);
}

void OpenGLRenderer::draw_line_2d(const glm::vec2& start, const glm::vec2& end, const glm::vec4& color, float thickness) {
    DrawCommand2D cmd;
    cmd.type        = DrawType2D::LINE;
    cmd.mode        = DrawMode2D::LINE;
    cmd.color       = color;
    cmd.use_texture = false;
    cmd.texture_id  = 0;
    cmd.thickness   = thickness;

    glm::vec2 dir = glm::normalize(end - start);
    glm::vec2 perp(-dir.y, dir.x);
    glm::vec2 offset = perp * (thickness * 0.5f);

    cmd.vertices = {
        {start - offset, {0, 0}, color}, {start + offset, {1, 0}, color}, {end + offset, {1, 1}, color}, {end - offset, {0, 1}, color}};

    cmd.indices = {0, 1, 2, 2, 3, 0};

    _draw_commands_2d.push_back(cmd);
}

void OpenGLRenderer::draw_circle_2d(const glm::vec2& center, float radius, const glm::vec4& color, bool filled, int segments) {
    DrawCommand2D cmd;
    cmd.type        = DrawType2D::CIRCLE;
    cmd.mode        = filled ? DrawMode2D::CIRCLE_FILLED : DrawMode2D::CIRCLE_OUTLINE;
    cmd.color       = color;
    cmd.use_texture = false;
    cmd.texture_id  = 0;
    cmd.position    = center;
    cmd.radius      = radius;
    cmd.segments    = segments;
    cmd.filled      = filled;
    cmd.thickness   = 1.0f;

    glm::vec2 min = center - glm::vec2(radius);
    glm::vec2 max = center + glm::vec2(radius);

    cmd.vertices = {
        {{min.x, min.y}, {0, 0}, color}, {{max.x, min.y}, {1, 0}, color}, {{max.x, max.y}, {1, 1}, color}, {{min.x, max.y}, {0, 1}, color}};

    cmd.indices = {0, 1, 2, 2, 3, 0};

    _draw_commands_2d.push_back(cmd);
}

void OpenGLRenderer::draw_circle_outline_2d(const glm::vec2& center, float radius, const glm::vec4& color, float thickness, int segments) {

    DrawCommand2D cmd;
    cmd.type        = DrawType2D::CIRCLE;
    cmd.mode        = DrawMode2D::CIRCLE_OUTLINE;
    cmd.color       = color;
    cmd.use_texture = false;
    cmd.texture_id  = 0;
    cmd.position    = center;
    cmd.radius      = radius;
    cmd.thickness   = thickness;
    cmd.segments    = segments;
    cmd.filled      = false;

    float outer_radius = radius + thickness * 0.5f;
    glm::vec2 min      = center - glm::vec2(outer_radius);
    glm::vec2 max      = center + glm::vec2(outer_radius);

    cmd.vertices = {
        {{min.x, min.y}, {0, 0}, color}, {{max.x, min.y}, {1, 0}, color}, {{max.x, max.y}, {1, 1}, color}, {{min.x, max.y}, {0, 1}, color}};

    cmd.indices = {0, 1, 2, 2, 3, 0};

    _draw_commands_2d.push_back(cmd);
}

void OpenGLRenderer::draw_triangle_2d(const glm::vec2& p1, const glm::vec2& p2, const glm::vec2& p3, const glm::vec4& color, bool filled) {
    DrawCommand2D cmd;
    cmd.type        = DrawType2D::TRIANGLE;
    cmd.mode        = filled ? DrawMode2D::FILLED : DrawMode2D::LINE;
    cmd.color       = color;
    cmd.use_texture = false;
    cmd.texture_id  = 0;
    cmd.filled      = filled;

    if (filled) {
        cmd.vertices = {{p1, {0, 0}, color}, {p2, {1, 0}, color}, {p3, {0.5f, 1}, color}};

        cmd.indices = {0, 1, 2};
    } else {
        constexpr float THICKNESS = 1.0f;

        auto create_line = [&](const glm::vec2& start, const glm::vec2& end) {
            glm::vec2 dir = glm::normalize(end - start);
            glm::vec2 perp(-dir.y, dir.x);
            glm::vec2 offset = perp * (THICKNESS * 0.5f);

            uint32_t base = cmd.vertices.size();
            cmd.vertices.push_back({start - offset, {0, 0}, color});
            cmd.vertices.push_back({start + offset, {1, 0}, color});
            cmd.vertices.push_back({end + offset, {1, 1}, color});
            cmd.vertices.push_back({end - offset, {0, 1}, color});

            cmd.indices.insert(cmd.indices.end(), {base, base + 1, base + 2, base + 2, base + 3, base});
        };

        create_line(p1, p2);
        create_line(p2, p3);
        create_line(p3, p1);
    }

    _draw_commands_2d.push_back(cmd);
}

void OpenGLRenderer::draw_text_2d(const std::string& text, const glm::vec2& position, const glm::vec4& color, float scale) {
    if (text.empty()) {
        return;
    }

    if (!_default_font) {
        spdlog::warn("No default font loaded for text rendering");
        return;
    }

    std::vector<TextMesh> meshes;


    size_t i = 0;
    while (i < text.length()) {
        unsigned char c = text[i];
        int char_len    = 1;
        bool is_emoji   = false;

        if ((c & 0x80) == 0) {
            char_len = 1; // ASCII
        } else if ((c & 0xE0) == 0xC0) {
            char_len = 2;
        } else if ((c & 0xF0) == 0xE0) {
            char_len = 3;
            is_emoji = true;
        } else if ((c & 0xF8) == 0xF0) {
            char_len = 4;
            is_emoji = true;
        }

        TTF_Font* char_font = is_emoji && _emoji_font ? _emoji_font : _default_font;

        if (!meshes.empty() && meshes.back().font == char_font) {
            meshes.back().text += text.substr(i, char_len);
        } else {
            meshes.push_back({text.substr(i, char_len), char_font, i});
        }

        i += char_len;
    }

    float x_offset = 0.0f;

    for (const auto& mesh : meshes) {
        Uint32 run_texture = render_text_to_texture(mesh.text, mesh.font);
        if (run_texture == 0) {
            continue;
        }

        std::string cache_key = std::to_string(reinterpret_cast<uintptr_t>(mesh.font)) + "_" + mesh.text;
        auto it               = _cached_text_textures.find(cache_key);
        if (it == _cached_text_textures.end()) {
            continue;
        }

        int run_width  = it->second.width;
        int run_height = it->second.height;


        DrawCommand2D cmd;
        cmd.type        = DrawType2D::TEXT;
        cmd.mode        = DrawMode2D::TEXT;
        cmd.color       = color;
        cmd.use_texture = true;
        cmd.texture_id  = run_texture;

        glm::vec2 run_pos  = position + glm::vec2(x_offset, 0.0f);
        glm::vec2 run_size = glm::vec2(run_width * scale, run_height * scale);

        cmd.position = run_pos;
        cmd.size     = run_size;

        cmd.vertices = {{{run_pos.x, run_pos.y}, {0.0f, 0.0f}, color},
                        {{run_pos.x + run_size.x, run_pos.y}, {1.0f, 0.0f}, color},
                        {{run_pos.x + run_size.x, run_pos.y + run_size.y}, {1.0f, 1.0f}, color},
                        {{run_pos.x, run_pos.y + run_size.y}, {0.0f, 1.0f}, color}};

        cmd.indices = {0, 1, 2, 2, 3, 0};

        _draw_commands_2d.push_back(cmd);

        x_offset += run_width * scale;
    }
}

Uint32 OpenGLRenderer::render_text_to_texture(const std::string& text, TTF_Font* font) {

    const std::string cache_key = std::to_string(reinterpret_cast<uintptr_t>(font)) + "_" + text;

    auto it = _cached_text_textures.find(cache_key);

    if (it != _cached_text_textures.end()) {
        return it->second.texture_id;
    }


    constexpr SDL_Color white_color = {255, 255, 255, 255};

    SDL_Surface* surface = TTF_RenderText_Blended(font, text.c_str(), text.length(), white_color);

    if (!surface) {
        spdlog::error("Failed to render text '{}': {}", text, SDL_GetError());
        return 0;
    }

    SDL_Surface* rgba_surface = SDL_ConvertSurface(surface, SDL_PIXELFORMAT_RGBA32);

    SDL_DestroySurface(surface);

    if (!rgba_surface) {
        spdlog::error("Failed to convert text surface to RGBA: {}", SDL_GetError());
        return 0;
    }

    GLuint texture_id;
    glGenTextures(1, &texture_id);
    glBindTexture(GL_TEXTURE_2D, texture_id);


    glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_SRGB8_ALPHA8, rgba_surface->w, rgba_surface->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, rgba_surface->pixels);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // // Generate mipmaps for better quality at different scales
    // glGenerateMipmap(GL_TEXTURE_2D);

    // GLfloat max_anisotropy = 0.0f;
    // glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &max_anisotropy);
    // if (max_anisotropy > 1.0f) {
    //     glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, std::min(max_anisotropy, 4.0f));
    // }
    //

    int width  = rgba_surface->w;
    int height = rgba_surface->h;

    SDL_DestroySurface(rgba_surface);

    _cached_text_textures[cache_key] = {texture_id, width, height};

    return texture_id;
}

bool OpenGLRenderer::load_font(const std::string& font_path, int point_size, const std::string& font_name) {
    std::string name = font_name.empty() ? font_path : font_name;

    if (_fonts.contains(name)) {
        spdlog::warn("Font '{}' already loaded", name);
        return true;
    }

    TTF_Font* font = TTF_OpenFont(font_path.c_str(), point_size);

    if (!font) {
        spdlog::error("Failed to load font '{}': {}", font_path, SDL_GetError());
        return false;
    }

    TTF_SetFontHinting(font, TTF_HINTING_NORMAL);

    TTF_SetFontKerning(font, true);

    TTF_SetFontStyle(font, TTF_STYLE_NORMAL);

    TTF_SetFontDirection(font, TTF_DIRECTION_LTR);

    _fonts[name] = font;

    if (name.find("emoji") != std::string::npos || name.find("Emoji") != std::string::npos || name.find("Twemoji") != std::string::npos
        || name.find("twemoji") != std::string::npos) {
        _emoji_font = font;
        spdlog::info("Loaded emoji font: '{}' ({}pt) as '{}'", font_path, point_size, name);
    } else {
        if (!_default_font) {
            _default_font = font;
        }
        spdlog::info("Loaded text font: '{}' ({}pt) as '{}'", font_path, point_size, name);
    }

    return true;
}
