#include "core/renderer/opengl/ogl_renderer.h"

#include "core/engine.h"
#include "core/io/file_system.h"
#include "core/renderer/base_struct.h"

void GLAPIENTRY ogl_validation_layer(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message,
                                     const void* userParam) {
    if (severity == GL_DEBUG_SEVERITY_NOTIFICATION) {
        return;
    }

    LOG_DEBUG("ValidationLayer Type: 0x%x | Severity: 0x%x | ID: %u | Message: %s", type, severity, id, message);
}


std::shared_ptr<OpenglMesh> generate_cube_mesh() {
    auto mesh = std::make_shared<OpenglMesh>();

    std::vector<Vertex> vertices = {
        // Front face
        {{-0.5f, -0.5f, 0.5f}, {0, 0, 1}, {0, 0}},
        {{0.5f, -0.5f, 0.5f}, {0, 0, 1}, {1, 0}},
        {{0.5f, 0.5f, 0.5f}, {0, 0, 1}, {1, 1}},
        {{-0.5f, 0.5f, 0.5f}, {0, 0, 1}, {0, 1}},

        // Back face
        {{0.5f, -0.5f, -0.5f}, {0, 0, -1}, {0, 0}},
        {{-0.5f, -0.5f, -0.5f}, {0, 0, -1}, {1, 0}},
        {{-0.5f, 0.5f, -0.5f}, {0, 0, -1}, {1, 1}},
        {{0.5f, 0.5f, -0.5f}, {0, 0, -1}, {0, 1}},

        // Left face
        {{-0.5f, -0.5f, -0.5f}, {-1, 0, 0}, {0, 0}},
        {{-0.5f, -0.5f, 0.5f}, {-1, 0, 0}, {1, 0}},
        {{-0.5f, 0.5f, 0.5f}, {-1, 0, 0}, {1, 1}},
        {{-0.5f, 0.5f, -0.5f}, {-1, 0, 0}, {0, 1}},

        // Right face
        {{0.5f, -0.5f, 0.5f}, {1, 0, 0}, {0, 0}},
        {{0.5f, -0.5f, -0.5f}, {1, 0, 0}, {1, 0}},
        {{0.5f, 0.5f, -0.5f}, {1, 0, 0}, {1, 1}},
        {{0.5f, 0.5f, 0.5f}, {1, 0, 0}, {0, 1}},

        // Top face
        {{-0.5f, 0.5f, 0.5f}, {0, 1, 0}, {0, 0}},
        {{0.5f, 0.5f, 0.5f}, {0, 1, 0}, {1, 0}},
        {{0.5f, 0.5f, -0.5f}, {0, 1, 0}, {1, 1}},
        {{-0.5f, 0.5f, -0.5f}, {0, 1, 0}, {0, 1}},

        // Bottom face
        {{-0.5f, -0.5f, -0.5f}, {0, -1, 0}, {0, 0}},
        {{0.5f, -0.5f, -0.5f}, {0, -1, 0}, {1, 0}},
        {{0.5f, -0.5f, 0.5f}, {0, -1, 0}, {1, 1}},
        {{-0.5f, -0.5f, 0.5f}, {0, -1, 0}, {0, 1}},
    };

    std::vector<unsigned int> indices = {
        0, 1, 2, 0, 2, 3, // Front
        4, 5, 6, 4, 6, 7, // Back
        8, 9, 10, 8, 10, 11, // Left
        12, 13, 14, 12, 14, 15, // Right
        16, 17, 18, 16, 18, 19, // Top
        20, 21, 22, 20, 22, 23 // Bottom
    };

    mesh->vertex_count = 24;
    mesh->index_count  = 36;


    glGenVertexArrays(1, &mesh->vao);
    glGenBuffers(1, &mesh->vbo);
    glGenBuffers(1, &mesh->ebo);

    glBindVertexArray(mesh->vao);

    glBindBuffer(GL_ARRAY_BUFFER, mesh->vbo);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*) offsetof(Vertex, position));
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*) offsetof(Vertex, normal));
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*) offsetof(Vertex, uv));
    glEnableVertexAttribArray(2);

    glBindVertexArray(0);

    return mesh;
}


std::shared_ptr<OpenglTexture> load_cubemap_atlas(const std::string& atlasPath, CUBEMAP_ORIENTATION orient = CUBEMAP_ORIENTATION::DEFAULT) {

    auto cubemap_texture = std::make_shared<OpenglTexture>();
    LOG_DEBUG("Loading cubemap atlas: %s", atlasPath.c_str());


    FileAccess file = FileAccess(atlasPath, ModeFlags::READ);

    if (!file.is_open()) {
        LOG_ERROR("Failed to open file %s", atlasPath.c_str());
        return cubemap_texture;
    }

    int W, H, channels;

    stbi_uc* pixels = stbi_load_from_memory(
        (stbi_uc*) file.get_file_as_bytes().data(),
        file.get_file_as_bytes().size(),
        &W,
        &H,
        &channels,
        STBI_rgb_alpha
        );

    if (!pixels) {
        LOG_ERROR("Failed to load texture from file %s", atlasPath.c_str());
        return cubemap_texture;
    }

    LOG_DEBUG("Converted surface: %dx%d Pitch: %d BytesPerPixel: %d", W, H, W * 4, 4);

    if (W <= 0 || H <= 0) {
        LOG_ERROR("Invalid surface dimensions %dx%d", W, H);
        stbi_image_free(pixels);
        return cubemap_texture;
    }

    int face_w = 0, face_h = 0;

    enum CUBE_LAYOUT { L_HORIZONTAL, L_VERTICAL, L_3x2, L_4x3_CROSS, L_UNKNOWN } layout = L_UNKNOWN;

    if (W % 6 == 0 && W / 6 == H) {
        layout = L_HORIZONTAL;
        face_w = W / 6;
        face_h = H;
    } else if (H % 6 == 0 && H / 6 == W) {
        layout = L_VERTICAL;
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
        LOG_WARN("Unknown atlas layout (%dx%d).", W, H);
        stbi_image_free(pixels);
        return cubemap_texture;
    }

    if (face_w <= 0 || face_h <= 0) {
        LOG_ERROR("Invalid face dimensions computed: %dx%d", face_w, face_h);
        stbi_image_free(pixels);
        return cubemap_texture;
    }

    LOG_DEBUG("Detected layout %d, Face size: %dx%d", layout, face_w, face_h);

    std::array<SDL_Rect, 6> faceRects;

    if (layout == L_HORIZONTAL) {
        for (int i = 0; i < 6; ++i) {
            faceRects[i] = SDL_Rect{i * face_w, 0, face_w, face_h};
        }
    } else if (layout == L_VERTICAL) {
        for (int i = 0; i < 6; ++i) {
            faceRects[i] = SDL_Rect{0, i * face_h, face_w, face_h};
        }
    } else if (layout == L_3x2) {
        faceRects[0] = SDL_Rect{0, 0, face_w, face_h}; // +X
        faceRects[1] = SDL_Rect{1 * face_w, 0, face_w, face_h}; // -X
        faceRects[2] = SDL_Rect{2 * face_w, 0, face_w, face_h}; // +Y
        faceRects[3] = SDL_Rect{0, 1 * face_h, face_w, face_h}; // -Y
        faceRects[4] = SDL_Rect{1 * face_w, 1 * face_h, face_w, face_h}; // +Z
        faceRects[5] = SDL_Rect{2 * face_w, 1 * face_h, face_w, face_h}; // -Z
    } else {
        int fw = face_w, fh = face_h;
        auto R = [&](int cx, int cy) {
            return SDL_Rect{cx * fw, cy * fh, fw, fh};
        };
        faceRects[0] = R(2, 1); // +X
        faceRects[1] = R(0, 1); // -X
        faceRects[2] = R(1, 0); // +Y
        faceRects[3] = R(1, 2); // -Y
        faceRects[4] = R(1, 1); // +Z
        faceRects[5] = R(3, 1); // -Z
    }

    // Orientation adjustments
    switch (orient) {
    case CUBEMAP_ORIENTATION::TOP:
        std::swap(faceRects[2], faceRects[3]); // +Y <-> -Y
        break;
    case CUBEMAP_ORIENTATION::BOTTOM:
        std::swap(faceRects[2], faceRects[3]);
        break;
    case CUBEMAP_ORIENTATION::FLIP_X:
        std::swap(faceRects[0], faceRects[1]);
        std::swap(faceRects[4], faceRects[5]);
        break;
    case CUBEMAP_ORIENTATION::FLIP_Y:
        std::swap(faceRects[2], faceRects[3]);
        std::swap(faceRects[4], faceRects[5]);
        break;
    default:
        break;
    }

    // Validate rects are fully inside surface
    for (int i = 0; i < 6; ++i) {
        const SDL_Rect& r = faceRects[i];
        if (r.x < 0 || r.y < 0 || r.x + r.w > W || r.y + r.h > H) {
            LOG_ERROR("Face rect %d out of surface bounds: x=%d y=%d w=%d h=%d surface=%dx%d", i, r.x, r.y, r.w, r.h, W, H);
            stbi_image_free(pixels);
            return cubemap_texture;
        }
    }

    constexpr int BYTES_PER_PIXEL = 4;
    int pitch               = W * BYTES_PER_PIXEL;
    const int surfaceBytes  = pitch * H;

    GLuint texID = 0;
    glGenTextures(1, &texID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, texID);

    // Allocate single reusable buffer for all faces (memory optimization)
    std::vector<Uint8> faceData(face_w * face_h * BYTES_PER_PIXEL);

    for (int i = 0; i < 6; ++i) {
        const SDL_Rect& r = faceRects[i];
        // LOG_INFO("Uploading face %d: x=%d y=%d w=%d h=%d pitch=%d", i, r.x, r.y, r.w, r.h, pitch);

        for (int y = 0; y < r.h; ++y) {
            int row = r.y + y;
            // compute byte offsets
            long start = static_cast<long>(row) * pitch + static_cast<long>(r.x) * BYTES_PER_PIXEL;
            long end   = start + static_cast<long>(r.w) * BYTES_PER_PIXEL;

            if (start < 0 || end > surfaceBytes) {
                LOG_ERROR("Index out of bounds detected while copying face %d row %d: start=%ld end=%ld surfaceBytes=%d (rect x=%d y=%d "
                          "w=%d h=%d pitch=%d)",
                          i, y, start, end, surfaceBytes, r.x, r.y, r.w, r.h, pitch);
                stbi_image_free(pixels);
                glDeleteTextures(1, &texID);
                return cubemap_texture;
            }

            // copy one row
            Uint8* src = pixels + start;
            Uint8* dst = faceData.data() + (y * r.w * BYTES_PER_PIXEL);
            SDL_memcpy(dst, src, r.w * BYTES_PER_PIXEL);
        }

        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGBA, r.w, r.h, 0, GL_RGBA, GL_UNSIGNED_BYTE, faceData.data());
    }

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

    LOG_DEBUG("Loaded Cubemap atlas %s (%dx%d) Layout %d Face %dx%d Texture Handle: %d", atlasPath.c_str(), W, H, layout, face_w, face_h,
              texID);

    cubemap_texture->id     = texID;
    cubemap_texture->width  = face_w;
    cubemap_texture->height = face_h;
    cubemap_texture->pitch = W * channels;
    cubemap_texture->path   = atlasPath;
    cubemap_texture->target = ETextureTarget::TEXTURE_CUBE_MAP;

    stbi_image_free(pixels);

    return cubemap_texture;
}


void OpenglRenderer::setup_cubemap() {

    // CUBE MAP POS
    constexpr float skybox_vertices[] = {
        -1.0f, 1.0f, -1.0f, -1.0f, -1.0f, -1.0f, 1.0f, -1.0f, -1.0f, 1.0f, -1.0f, -1.0f, 1.0f, 1.0f, -1.0f, -1.0f, 1.0f, -1.0f,

        -1.0f, -1.0f, 1.0f, -1.0f, -1.0f, -1.0f, -1.0f, 1.0f, -1.0f, -1.0f, 1.0f, -1.0f, -1.0f, 1.0f, 1.0f, -1.0f, -1.0f, 1.0f,

        1.0f, -1.0f, -1.0f, 1.0f, -1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, -1.0f, 1.0f, -1.0f, -1.0f,

        -1.0f, -1.0f, 1.0f, -1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, -1.0f, 1.0f, -1.0f, -1.0f, 1.0f,

        -1.0f, 1.0f, -1.0f, 1.0f, 1.0f, -1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, -1.0f, 1.0f, 1.0f, -1.0f, 1.0f, -1.0f,

        -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, 1.0f, 1.0f, -1.0f, -1.0f, 1.0f, -1.0f, -1.0f, -1.0f, -1.0f, 1.0f, 1.0f, -1.0f, 1.0f};

    skybox_mesh               = new OpenglMesh();
    skybox_mesh->vertex_count = 36;

    glGenVertexArrays(1, &skybox_mesh->vao);
    glGenBuffers(1, &skybox_mesh->vbo);
    glBindVertexArray(skybox_mesh->vao);
    glBindBuffer(GL_ARRAY_BUFFER, skybox_mesh->vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skybox_vertices), skybox_vertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*) 0);
    glBindVertexArray(0);

    skybox_shader = new OpenglShader("shaders/opengl/skybox.vert", "shaders/opengl/skybox.frag");

    skybox_mesh->material->albedo_texture = load_cubemap_atlas("res://environment_sky.png", CUBEMAP_ORIENTATION::DEFAULT);

    LOG_DEBUG("Environment setup complete");
}

// TODO: refactor this when make the Framebuffer class
Uint32 shadowFBO   = 0;
Uint32 shadowTexID = 0;
Uint32 shadowWidth = 8192, shadowHeight = 8192;


bool OpenglRenderer::initialize(SDL_Window* window) {


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
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_COMPATIBILITY);


#endif

    SDL_GLContext glContext = SDL_GL_CreateContext(window);

    if (!glContext) {
        LOG_CRITICAL("Failed to create GL context, %s", SDL_GetError());
        return false;
    }


#if defined(SDL_PLATFORM_IOS) || defined(SDL_PLATFORM_ANDROID) || defined(SDL_PLATFORM_EMSCRIPTEN)

    if (!gladLoadGLES2Loader((GLADloadproc) SDL_GL_GetProcAddress)) {
        LOG_CRITICAL("Failed to initialize GLAD (GLES_FUNCTIONS)");
        return false;
    }

#else

    if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(SDL_GL_GetProcAddress))) {
        LOG_CRITICAL("Failed to initialize GLAD (GL_FUNCTIONS)");
        return false;
    }

#endif

    _context = glContext;
    _window  = window;


    SDL_GL_SetSwapInterval(0);

    GLint num_extensions = 0;
    std::vector<std::string> extensions;
    glGetIntegerv(GL_NUM_EXTENSIONS, &num_extensions);
    extensions.reserve(num_extensions);

    bool khr_debug_found = false;

    // LOG_DEBUG("OpenGL Extensions:");
    for (GLuint i = 0; i < num_extensions; i++) {
        const char* ext = reinterpret_cast<const char*>(glGetStringi(GL_EXTENSIONS, i));

        // LOG_DEBUG("\t%s", ext);

        if (SDL_strcasecmp(ext, "GL_KHR_debug") == 0) {
            LOG_DEBUG("KHR_debug extension supported, enabling validation layers");
            glEnable(GL_DEBUG_OUTPUT);
            glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
            glDebugMessageCallback(ogl_validation_layer, nullptr);
            khr_debug_found = true;
            break;
        }
    }

    if (!khr_debug_found) {
        LOG_WARN("KHR_debug extensions not supported, validation layers disabled");
    }

    int major, minor;
    SDL_GL_GetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, &major);
    SDL_GL_GetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, &minor);
    LOG_INFO("OpenGL Version: %d.%d", major, minor);
    LOG_INFO("OpenGL Vendor: %s", glGetString(GL_VENDOR));
    LOG_INFO("OpenGL Renderer: %s", glGetString(GL_RENDERER));


    setup_default_shaders();

    // TODO: create api to handle environment setup
    setup_cubemap();

    const auto& viewport = GEngine->get_config().get_viewport();
    // LOG_INFO("Using backend: %s, Viewport: %dx%d", viewport.width, viewport.height);

    cube_mesh = generate_cube_mesh();

    int msaa_buffers = 0, msaa_samples = 0;
    SDL_GL_GetAttribute(SDL_GL_MULTISAMPLEBUFFERS, &msaa_buffers);
    SDL_GL_GetAttribute(SDL_GL_MULTISAMPLESAMPLES, &msaa_samples);
    LOG_INFO("MSAA: %dx (buffers=%d, samples=%d) %s", msaa_samples, msaa_buffers, msaa_samples,
             glIsEnabled(GL_MULTISAMPLE) ? "ENABLED" : "DISABLED");

    // glEnable(GL_STENCIL_TEST);
    // glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);

    // glViewport(0, 0, viewport.width, viewport.height);
    glGenFramebuffers(1, &shadowFBO);

    glGenTextures(1, &shadowTexID);
    glBindTexture(GL_TEXTURE_2D, shadowTexID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, shadowWidth, shadowHeight, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

    float borderColor[] = {1.0f, 1.0f, 1.0f, 1.0f};
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

    LOG_INFO("Shadow Map: %dx%d with GL_LINEAR", shadowWidth, shadowHeight);

    glBindFramebuffer(GL_FRAMEBUFFER, shadowFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, shadowTexID, 0);

    glDrawBuffers(0, nullptr);
    glReadBuffer(GL_NONE);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        LOG_ERROR("Shadow Framebuffer not complete!");
    } else {
        LOG_INFO("Shadow Framebuffer completed");
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);


    return true;
}

void OpenglRenderer::clear(glm::vec4 color) {

    // NOTE: Nuklear disables depth test, so we need to re-enable it each frame
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    // TODO: handle viewport/window changes
    // const auto& window = GEngine->get_config().get_window();
    // glViewport(0, 0, window.width, window.height);
    // glClearColor(color.r, color.g, color.b, color.a);
    // glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void OpenglRenderer::present() {

    SDL_GL_SwapWindow(_window);
}


bool OpenglRenderer::load_font(const std::string& name, const std::string& path, int size) {
    return true;
}

std::shared_ptr<Texture> OpenglRenderer::load_texture(const std::string& name, const std::string& path, const aiTexture* ai_embedded_tex) {


    if (_textures.contains(name)) {
        return _textures[name];
    }

    auto texture = Renderer::load_texture(name, path, ai_embedded_tex);

    if (!texture || !texture->pixels) {
        LOG_ERROR("Failed to load texture surface: %s", name.c_str());
        return nullptr;
    }


    texture->target = ETextureTarget::TEXTURE_2D;

    GLuint texID;
    glGenTextures(1, &texID);
    glBindTexture(GL_TEXTURE_2D, texID);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, texture->width, texture->height, 0, GL_RGBA, GL_UNSIGNED_BYTE, texture->pixels);
    glGenerateMipmap(GL_TEXTURE_2D);

    texture->id     = texID;
    _textures[name] = texture;

    LOG_DEBUG("Successfully uploaded texture '%s' to GPU with ID=%u (size=%dx%d)", name.c_str(), texID, texture->width, texture->height);

    free(texture->pixels);
    texture->pixels = nullptr;

    return texture;
}


std::shared_ptr<Model> OpenglRenderer::load_model(const char* path) {

    if (_models.contains(path)) {
        return _models[path];
    }

    auto model = Renderer::load_model(path);

    if (model) {
        _models[path] = model;
    }

    return model;
}


void OpenglRenderer::draw_line_3d(const glm::vec3& from, const glm::vec3& to, const glm::vec4& color) {
}

void OpenglRenderer::draw_triangle_3d(const glm::vec3& v1, const glm::vec3& v2, const glm::vec3& v3, const glm::vec4& color,
                                      bool is_filled) {
}

std::unique_ptr<Mesh> OpenglRenderer::load_mesh(aiMesh* mesh, const aiScene* scene, const std::string& base_dir) {
    auto ogl_mesh = std::make_unique<OpenglMesh>();

    parse_meshes(mesh, scene, base_dir, *ogl_mesh);

    // TODO: we should create a parse_materials function to reduce code duplication
    if (scene->mNumMaterials > mesh->mMaterialIndex) {
        aiMaterial* mat = scene->mMaterials[mesh->mMaterialIndex];

        aiString texPath;
        if (mat->GetTexture(aiTextureType_DIFFUSE, 0, &texPath) == AI_SUCCESS) {
            std::string texPathStr = texPath.C_Str();


            // Check if it's an embedded texture (path starts with '*')
            if (texPathStr[0] == '*') {
                // Embedded texture - extract index
                int texIndex = std::atoi(texPathStr.c_str() + 1);

                if (texIndex >= 0 && static_cast<unsigned int>(texIndex) < scene->mNumTextures) {
                    const aiTexture* embeddedTex = scene->mTextures[texIndex];

                    // Use base_dir to make key unique per model
                    texPathStr = base_dir + "embedded_tex_" + std::to_string(texIndex);

                    // Load embedded texture (now OpenGL context is available)
                    ogl_mesh->material->albedo_texture = load_texture(texPathStr, texPathStr, embeddedTex);
                } else {
                    LOG_WARN("Embedded texture index %d out of range (scene has %u textures)", texIndex, scene->mNumTextures);
                }

            } else {
                // Load external texture file
                const std::string texture_path     = base_dir + texPathStr;
                ogl_mesh->material->albedo_texture = load_texture(texture_path, texture_path, nullptr);
            }
        }
    }

    std::vector<glm::ivec4> bone_ids;
    std::vector<glm::vec4> bone_weights;
    parse_bones(mesh, bone_ids, bone_weights, *ogl_mesh);

    // TODO: improve this setup
    glGenVertexArrays(1, &ogl_mesh->vao);
    glGenBuffers(1, &ogl_mesh->vbo);
    glGenBuffers(1, &ogl_mesh->ebo);

    glBindVertexArray(ogl_mesh->vao);

    // Upload vertex data (position, normal, uv)
    glBindBuffer(GL_ARRAY_BUFFER, ogl_mesh->vbo);
    glBufferData(GL_ARRAY_BUFFER, ogl_mesh->vertices.size() * sizeof(Vertex), ogl_mesh->vertices.data(), GL_STATIC_DRAW);

    // Upload index data
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ogl_mesh->ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, ogl_mesh->indices.size() * sizeof(unsigned int), ogl_mesh->indices.data(), GL_STATIC_DRAW);

    // Vertex attributes
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*) offsetof(Vertex, position));
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*) offsetof(Vertex, normal));
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*) offsetof(Vertex, uv));
    glEnableVertexAttribArray(2);

    // Upload bone data if present
    if (mesh->HasBones()) {

        // Bone IDs (ivec4 at location 8)
        glGenBuffers(1, &ogl_mesh->bone_id_vbo);
        glBindBuffer(GL_ARRAY_BUFFER, ogl_mesh->bone_id_vbo);
        glBufferData(GL_ARRAY_BUFFER, bone_ids.size() * sizeof(glm::ivec4), bone_ids.data(), GL_DYNAMIC_DRAW);
        glVertexAttribIPointer(8, 4, GL_INT, sizeof(glm::ivec4), (void*) 0);
        glEnableVertexAttribArray(8);

        // Bone Weights (vec4 at location 9)
        glGenBuffers(1, &ogl_mesh->bone_weight_vbo);
        glBindBuffer(GL_ARRAY_BUFFER, ogl_mesh->bone_weight_vbo);
        glBufferData(GL_ARRAY_BUFFER, bone_weights.size() * sizeof(glm::vec4), bone_weights.data(), GL_DYNAMIC_DRAW);
        glVertexAttribPointer(9, 4, GL_FLOAT, GL_FALSE, sizeof(glm::vec4), (void*) 0);
        glEnableVertexAttribArray(9);
    }

    glBindVertexArray(0);


    return ogl_mesh;
}


void OpenglRenderer::draw_model(const Transform3D& t, const Model* model) {


    if (!model || !default_shader) {
        return;
    }

    for (auto& mesh : model->meshes) {
        if (!mesh) {
            continue;
        }

        auto& batch  = _instanced_batches[mesh.get()];
        batch.mesh   = mesh.get();
        batch.shader = default_shader;
        batch.models.push_back(t.get_model_matrix());
        batch.colors.push_back(glm::vec3(1.0f));
        batch.mode = GEngine->get_config().is_debug ? EDrawMode::LINES : EDrawMode::TRIANGLES;
    }
}

void OpenglRenderer::draw_animated_model(const Transform3D& t, const Model* model, const glm::mat4* bone_transforms, int bone_count) {
    if (!model || !default_shader) {
        return;
    }


    bone_count = bone_count > MAX_BONES ? SDL_min(bone_count, MAX_BONES) : bone_count;

    for (auto& mesh : model->meshes) {
        if (!mesh || !mesh->has_bones) {
            continue;
        }

        auto& batch  = _instanced_batches[mesh.get()];
        batch.mesh   = mesh.get();
        batch.shader = default_shader;
        batch.models.push_back(t.get_model_matrix());
        batch.colors.push_back(glm::vec3(1.0f));
        batch.mode = GEngine->get_config().is_debug ? EDrawMode::LINES : EDrawMode::TRIANGLES;


        batch.bone_transforms = bone_transforms;
        batch.bone_count      = bone_count;
    }
}

void OpenglRenderer::flush(const glm::mat4& view, const glm::mat4& projection) {

    // Simple directional light setup
    // Light direction: vector pointing FROM scene UP TO the sun
    glm::vec3 to_light = glm::normalize(glm::vec3(1.0f, 2.5f, 1.0f)); // Sun higher in sky

    // TODO: Calculate dynamic scene bounds from all rendered objects
    // For now, using larger fixed bounds to capture more of the scene
    glm::vec3 scene_center   = glm::vec3(0.0f, 5.0f, 10.0f); // Center of your scene
    glm::vec3 light_position = scene_center + to_light * 100.0f; // Far enough to act as directional

    // For shader: direction light comes FROM (opposite of to_light)
    glm::vec3 lightDir = -to_light;

    // Larger orthographic bounds to capture full scene (adjust these if shadows get cut off)
    float shadow_extent           = 120.0f; // Increased from 80 to capture more
    glm::mat4 orthgonalProjection = glm::ortho(-shadow_extent, shadow_extent, -shadow_extent, shadow_extent, 0.1f, 1000.0f);
    glm::mat4 lightView           = glm::lookAt(light_position, scene_center, glm::vec3(0.0f, 1.0f, 0.0f));
    glm::mat4 lightProjection     = orthgonalProjection * lightView;

    glDisable(GL_MULTISAMPLE);
#pragma region SHADOW_PASS
    glEnable(GL_DEPTH_TEST);

    glViewport(0, 0, shadowWidth, shadowHeight);
    glBindFramebuffer(GL_FRAMEBUFFER, shadowFBO);
    glClear(GL_DEPTH_BUFFER_BIT);

    shadow_shader->activate();
    shadow_shader->set_value("LIGHT_PROJECTION", lightProjection);


    // Render all batches to shadow map (instanced rendering)
    for (auto& [_, batch] : _instanced_batches) {
        const Mesh* mesh = batch.mesh;
        auto& models     = batch.models;

        if (!mesh || models.empty()) {
            continue;
        }

        const OpenglMesh* ogl_mesh = static_cast<const OpenglMesh*>(mesh);
        if (!ogl_mesh) {
            continue;
        }

        auto& buffers = _buffers[mesh];
        if (buffers.instance_buffer == 0) {
            glGenBuffers(1, &buffers.instance_buffer);
            glGenBuffers(1, &buffers.color_buffer);
        }


        glBindVertexArray(ogl_mesh->vao);

        // Upload instance model matrices
        glBindBuffer(GL_ARRAY_BUFFER, buffers.instance_buffer);
        glBufferData(GL_ARRAY_BUFFER, models.size() * sizeof(glm::mat4), models.data(), GL_STREAM_DRAW);

        // Set up instanced model matrix attributes (location 3-6 for mat4)
        // CRITICAL: Buffer must be bound when setting up attributes
        for (int i = 0; i < 4; i++) {
            glEnableVertexAttribArray(3 + i);
            glVertexAttribPointer(3 + i, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*) (i * sizeof(glm::vec4)));
            glVertexAttribDivisor(3 + i, 1);
        }

        if (mesh->has_bones) {
            shadow_shader->set_value("USE_SKELETON", 1);

            int count = batch.bone_count < MAX_BONES ? batch.bone_count : MAX_BONES;

            if (batch.bone_transforms && batch.bone_count > 0) {
                shadow_shader->set_value("BONES", batch.bone_transforms, count);
            }

        } else {
            shadow_shader->set_value("USE_SKELETON", 0);
        }

        glDrawElementsInstanced(GL_TRIANGLES, ogl_mesh->index_count, GL_UNSIGNED_INT, 0, models.size());
    }


    glBindFramebuffer(GL_FRAMEBUFFER, 0);
#pragma endregion

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_MULTISAMPLE);
#pragma region RENDER_PASS

    const auto& window = GEngine->get_config().get_window();
    glViewport(0, 0, window.width, window.height);
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // LOG_DEBUG("MSAA during render: %s", glIsEnabled(GL_MULTISAMPLE) ? "ON" : "OFF");

    for (auto& [_, batch] : _instanced_batches) {
        const Mesh* mesh = batch.mesh;
        auto shader      = batch.shader;
        auto& models     = batch.models;

        if (!mesh || !shader || models.empty()) {
            continue;
        }

        OpenglShader* ogl_shader = static_cast<OpenglShader*>(shader);
        ogl_shader->activate();
        ogl_shader->set_value("VIEW", view);
        ogl_shader->set_value("PROJECTION", projection);
        ogl_shader->set_value("CAMERA_POSITION", glm::vec3(glm::inverse(view)[3]));

        // Set up directional light (sun)
        ogl_shader->set_value("LIGHT_DIRECTION", lightDir); // Direction light comes FROM
        ogl_shader->set_value("LIGHT_PROJECTION", lightProjection);

        const OpenglMesh* ogl_mesh = static_cast<const OpenglMesh*>(mesh);
        if (!ogl_shader->is_valid() || !ogl_mesh) {
            continue;
        }


        auto& buffers = _buffers[mesh];

        if (buffers.instance_buffer == 0) {
            glGenBuffers(1, &buffers.instance_buffer);
            glGenBuffers(1, &buffers.color_buffer);
        }

        glBindBuffer(GL_ARRAY_BUFFER, buffers.instance_buffer);
        glBufferData(GL_ARRAY_BUFFER, models.size() * sizeof(glm::mat4), models.data(), GL_STREAM_DRAW);

        glBindVertexArray(ogl_mesh->vao);

        // TODO: refactor to send SSBO for bones
        if (mesh->has_bones) {
            ogl_shader->set_value("USE_SKELETON", 1);

            int count = batch.bone_count < MAX_BONES ? batch.bone_count : MAX_BONES;

            if (batch.bone_transforms && batch.bone_count > 0) {
                ogl_shader->set_value("BONES", batch.bone_transforms, count);
            }

        } else {
            ogl_shader->set_value("USE_SKELETON", 0);
        }


        glm::vec3 albedo   = glm::vec3(1.0f);
        glm::vec3 ambient  = glm::vec3(0.0f);
        glm::vec3 specular = glm::vec3(0.0f);
        float metallic_val = 0.0f;
        int use_texture    = 0;

        // If the mesh has a valid material, use it
        if (mesh->material && mesh->material->is_valid()) {
            mesh->material->bind();

            albedo       = mesh->material->albedo;
            ambient      = mesh->material->ambient;
            specular     = mesh->material->metallic.specular;
            metallic_val = mesh->material->metallic.value;
            use_texture  = mesh->material->albedo_texture ? 1 : 0;
        }
        // If material is invalid but exists (MODEL fallback)
        else if (mesh->material && batch.command == EDrawCommand::MODEL) {
            albedo       = mesh->material->albedo;
            ambient      = mesh->material->ambient;
            specular     = mesh->material->metallic.specular;
            metallic_val = mesh->material->metallic.value;
        }

        ogl_shader->set_value("USE_TEXTURE", use_texture);
        ogl_shader->set_value("material.albedo", albedo);
        // ogl_shader->set_value("material.ambient", ambient);
        // ogl_shader->set_value("material.metallic.specular", specular);
        // ogl_shader->set_value("material.metallic.value", metallic_val);
        // ogl_shader->set_value("material.roughness", 0.5f);

        // Bind textures
        if (use_texture && mesh->material && mesh->material->albedo_texture) {
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, mesh->material->albedo_texture->id);
            ogl_shader->set_value("TEXTURE", 0);
        }

        // Bind shadow map to texture unit 1
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, shadowTexID);
        ogl_shader->set_value("SHADOW_TEXTURE", 1);


        // instanced model matrix (4 vec4)
        for (int i = 0; i < 4; i++) {
            glEnableVertexAttribArray(3 + i);
            glVertexAttribPointer(3 + i, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*) (i * sizeof(glm::vec4)));
            glVertexAttribDivisor(3 + i, 1);
        }

        // instanced colors
        if (!batch.colors.empty()) {
            glBindBuffer(GL_ARRAY_BUFFER, buffers.color_buffer);
            glBufferData(GL_ARRAY_BUFFER, batch.colors.size() * sizeof(glm::vec3), batch.colors.data(), GL_STREAM_DRAW);

            glEnableVertexAttribArray(7);
            glVertexAttribPointer(7, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*) 0);
            glVertexAttribDivisor(7, 1);
        }


        auto mode = batch.mode == EDrawMode::LINES ? GL_LINES : GL_TRIANGLES;
        glDrawElementsInstanced(mode, ogl_mesh->index_count, GL_UNSIGNED_INT, 0, models.size());
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    glBindVertexArray(0);
    _instanced_batches.clear();

#pragma endregion
    glDisable(GL_BLEND);

#pragma region ENVIRONMENT_PASS
    draw_environment(view, projection);

#pragma endregion
}


void OpenglRenderer::draw_mesh(const Transform3D& transform, const MeshInstance3D& mesh, const Shader* shader) {

    if (!cube_mesh) {
        return;
    }

    Transform3D temp = transform;
    temp.scale       = mesh.size;

    auto& batch                  = _instanced_batches[cube_mesh.get()];
    batch.mesh                   = cube_mesh.get();
    batch.mesh->material->albedo = mesh.material.albedo;
    batch.shader                 = default_shader;
    batch.models.push_back(temp.get_model_matrix());
    batch.colors.push_back(mesh.material.albedo);
    batch.command = EDrawCommand::MESH;
    batch.mode    = GEngine->get_config().is_debug ? EDrawMode::LINES : EDrawMode::TRIANGLES;
}

void OpenglRenderer::draw_environment(const glm::mat4& view, const glm::mat4& projection) {


    if (skybox_mesh == nullptr || !skybox_mesh->material->is_valid()) {
        return;
    }

    glDepthFunc(GL_LEQUAL);

    skybox_shader->activate();

    skybox_shader->set_value("VIEW", view);
    skybox_shader->set_value("PROJECTION", projection);

    skybox_mesh->bind();

    skybox_mesh->material->bind();

    skybox_mesh->draw(EDrawMode::TRIANGLES);

    skybox_mesh->unbind();

    glDepthFunc(GL_LESS);
}

void OpenglRenderer::draw_texture(const Transform2D& transform, Texture* texture, const glm::vec4& dest, const glm::vec4& source,
                                  bool flip_h, bool flip_v, const glm::vec4& color) {
}

void OpenglRenderer::draw_text(const Transform2D& transform, const glm::vec4& color, const std::string& font_name, const char* fmt, ...) {
}

void OpenglRenderer::draw_text_3d(const Transform3D& transform, const glm::mat4& view, const glm::mat4& projection, const glm::vec4& color,
                                  const std::string& font_name, const char* fmt, ...) {
}

void OpenglRenderer::draw_rect(const Transform2D& transform, float w, float h, glm::vec4 color, bool is_filled) {
}

void OpenglRenderer::draw_triangle(const Transform2D& transform, float size, glm::vec4 color, bool is_filled) {
}

void OpenglRenderer::draw_line(const Transform2D& transform, glm::vec2 end, glm::vec4 color) {
}

void OpenglRenderer::draw_circle(const Transform2D& transform, float radius, glm::vec4 color, bool is_filled) {
}

void OpenglRenderer::draw_polygon(const Transform2D& transform, const std::vector<glm::vec2>& points, glm::vec4 color, bool is_filled) {
}


OpenglRenderer::~OpenglRenderer() {

    for (auto& [_, buffer] : _buffers) {
        if (buffer.instance_buffer) {
            glDeleteBuffers(1, &buffer.instance_buffer);
            buffer.instance_buffer = 0;
        }
        if (buffer.color_buffer) {
            glDeleteBuffers(1, &buffer.color_buffer);
            buffer.color_buffer = 0;
        }
    }

    glDeleteTextures(1, &shadowTexID);
    glDeleteFramebuffers(1, &shadowFBO);

    _buffers.clear();

    // cubemap resources
    delete skybox_mesh;
    skybox_mesh = nullptr;

    delete skybox_shader;
    skybox_shader = nullptr;

    // default shader
    delete default_shader;
    default_shader = nullptr;

    delete shadow_shader;
    shadow_shader = nullptr;

    SDL_GL_DestroyContext(_context);
}

void OpenglRenderer::setup_default_shaders() {


    default_shader = new OpenglShader("shaders/opengl/default.vert", "shaders/opengl/default.frag");
    if (!default_shader->is_valid()) {
        LOG_ERROR("Failed to create default shader");
        delete default_shader;
        default_shader = nullptr;
        return;
    }

    default_shader->activate();
    default_shader->set_value("LIGHT_COLOR", glm::vec3(1.0f, 0.95f, 0.8f)); // Warm sun color
    // default_shader->set_value("TEXTURE", 0);
    // default_shader->set_value("SHADOW_TEXTURE", 1);

    shadow_shader = new OpenglShader("shaders/opengl/shadow.vert", "shaders/opengl/shadow.frag");
    if (!shadow_shader->is_valid()) {
        LOG_ERROR("Failed to create shadow shader");
        delete shadow_shader;
        shadow_shader = nullptr;
        return;
    }
}


std::vector<Tokens> OpenglRenderer::parse_text(const std::string& text) {

    return {};
}

void OpenglRenderer::draw_text_internal(const glm::vec2& pos, const glm::vec4& color, const std::string& font_name,
                                        const std::string& text) {
}
