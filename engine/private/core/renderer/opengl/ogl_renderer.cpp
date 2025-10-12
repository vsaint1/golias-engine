#include "core/renderer/opengl/ogl_renderer.h"

#include "core/engine.h"
#include "core/io/file_system.h"
#include "core/renderer/base_struct.h"

void GLAPIENTRY ogl_validation_layer(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message,
                                     const void* userParam) {
    if (severity == GL_DEBUG_SEVERITY_NOTIFICATION) {
        return;
    }

    LOG_WARN("ValidationLayer Type: 0x%x | Severity: 0x%x | ID: %u | Message: %s", type, severity, id, message);
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
        0,  1,  2,  0,  2,  3, // Front
        4,  5,  6,  4,  6,  7, // Back
        8,  9,  10, 8,  10, 11, // Left
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


GLuint load_cubemap_atlas(const std::string& atlasPath, CUBEMAP_ORIENTATION orient = CUBEMAP_ORIENTATION::DEFAULT) {

    LOG_INFO("Loading cubemap atlas: %s", atlasPath.c_str());


    FileAccess file = FileAccess(atlasPath, ModeFlags::READ);

    if (!file.is_open()) {
        LOG_ERROR("Failed to open file %s", atlasPath.c_str());
        return 0;
    }

    SDL_Surface* surf = IMG_Load_IO(file.get_handle(), false);

    if (!surf) {
        LOG_ERROR("Failed to load %s -> %s", atlasPath.c_str(), SDL_GetError());
        return 0;
    }

    LOG_INFO("Loaded surface (pre-convert): %dx%d, Pitch: %d", surf->w, surf->h, surf->pitch);

    surf = SDL_ConvertSurface(surf, SDL_PIXELFORMAT_RGBA32);
    if (!surf) {
        LOG_ERROR("Conversion failed %s", atlasPath.c_str());
        return 0;
    }

    const int W = surf->w;
    const int H = surf->h;
    LOG_INFO("Converted surface: %dx%d Pitch: %d BytesPerPixel: %d", W, H, surf->pitch, SDL_BYTESPERPIXEL(surf->format));

    if (W <= 0 || H <= 0) {
        LOG_ERROR("Invalid surface dimensions %dx%d", W, H);
        SDL_DestroySurface(surf);
        return 0;
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
        SDL_DestroySurface(surf);
        return 0;
    }

    if (face_w <= 0 || face_h <= 0) {
        LOG_ERROR("Invalid face dimensions computed: %dx%d", face_w, face_h);
        SDL_DestroySurface(surf);
        return 0;
    }

    LOG_INFO("Detected layout %d, Face size: %dx%d", layout, face_w, face_h);

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
        auto R       = [&](int cx, int cy) { return SDL_Rect{cx * fw, cy * fh, fw, fh}; };
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
            SDL_DestroySurface(surf);
            return 0;
        }
    }

    Uint8* pixels           = static_cast<Uint8*>(surf->pixels);
    int pitch               = surf->pitch;
    const int bytesPerPixel = 4;
    const int surfaceBytes  = pitch * H;

    GLuint texID = 0;
    glGenTextures(1, &texID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, texID);

    for (int i = 0; i < 6; ++i) {
        const SDL_Rect& r = faceRects[i];
        // LOG_INFO("Uploading face %d: x=%d y=%d w=%d h=%d pitch=%d", i, r.x, r.y, r.w, r.h, pitch);

        // allocate buffer for face
        std::vector<Uint8> faceData(r.w * r.h * bytesPerPixel);

        for (int y = 0; y < r.h; ++y) {
            int row = r.y + y;
            // compute byte offsets
            long start = static_cast<long>(row) * pitch + static_cast<long>(r.x) * bytesPerPixel;
            long end   = start + static_cast<long>(r.w) * bytesPerPixel;

            if (start < 0 || end > surfaceBytes) {
                LOG_ERROR("Index out of bounds detected while copying face %d row %d: start=%ld end=%ld surfaceBytes=%d (rect x=%d y=%d "
                          "w=%d h=%d pitch=%d)",
                          i, y, start, end, surfaceBytes, r.x, r.y, r.w, r.h, pitch);
                SDL_DestroySurface(surf);
                glDeleteTextures(1, &texID);
                return 0;
            }

            // copy one row
            Uint8* src = pixels + start;
            Uint8* dst = faceData.data() + (y * r.w * bytesPerPixel);
            SDL_memcpy(dst, src, r.w * bytesPerPixel);
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

    LOG_INFO("Loaded Cubemap atlas %s (%dx%d) Layout %d Face %dx%d Texture Handle: %d", atlasPath.c_str(), W, H, layout, face_w, face_h,
             texID);

    SDL_DestroySurface(surf);
    return texID;
}


void OpenglRenderer::setup_cubemap() {

    // CUBE MAP POS
    constexpr float skybox_vertices[] = {
        -1.0f, 1.0f,  -1.0f, -1.0f, -1.0f, -1.0f, 1.0f,  -1.0f, -1.0f, 1.0f,  -1.0f, -1.0f, 1.0f,  1.0f,  -1.0f, -1.0f, 1.0f,  -1.0f,

        -1.0f, -1.0f, 1.0f,  -1.0f, -1.0f, -1.0f, -1.0f, 1.0f,  -1.0f, -1.0f, 1.0f,  -1.0f, -1.0f, 1.0f,  1.0f,  -1.0f, -1.0f, 1.0f,

        1.0f,  -1.0f, -1.0f, 1.0f,  -1.0f, 1.0f,  1.0f,  1.0f,  1.0f,  1.0f,  1.0f,  1.0f,  1.0f,  1.0f,  -1.0f, 1.0f,  -1.0f, -1.0f,

        -1.0f, -1.0f, 1.0f,  -1.0f, 1.0f,  1.0f,  1.0f,  1.0f,  1.0f,  1.0f,  1.0f,  1.0f,  1.0f,  -1.0f, 1.0f,  -1.0f, -1.0f, 1.0f,

        -1.0f, 1.0f,  -1.0f, 1.0f,  1.0f,  -1.0f, 1.0f,  1.0f,  1.0f,  1.0f,  1.0f,  1.0f,  -1.0f, 1.0f,  1.0f,  -1.0f, 1.0f,  -1.0f,

        -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, 1.0f,  1.0f,  -1.0f, -1.0f, 1.0f,  -1.0f, -1.0f, -1.0f, -1.0f, 1.0f,  1.0f,  -1.0f, 1.0f};

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

    skybox_mesh->texture_id = load_cubemap_atlas("res://environment_sky.png", CUBEMAP_ORIENTATION::DEFAULT);

    LOG_INFO("Environment setup complete");
}

struct BatchGL {
    GLuint instanceVBO;
    GLuint colorVBO;
    size_t capacity; // how many instances this buffer can hold
};

static BatchGL ogl_batch;

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

    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);

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

    for (GLuint i = 0; i < num_extensions; i++) {
        const char* ext = reinterpret_cast<const char*>(glGetStringi(GL_EXTENSIONS, i));

        if (SDL_strcasecmp(ext, "GL_KHR_debug") == 0) {
            LOG_INFO("KHR_debug extension supported, enabling validation layers");
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

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    glEnable(GL_DEPTH_TEST);
    // glEnable(GL_STENCIL_TEST);
    // glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);

    // glViewport(0, 0, viewport.width, viewport.height);


    return true;
}

void OpenglRenderer::clear(glm::vec4 color) {

    // NOTE: Nuklear disables depth test, so we need to re-enable it each frame
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);

    // TODO: handle viewport/window changes
    const auto& window = GEngine->get_config().get_window();
    glViewport(0, 0, window.width, window.height);
    glClearColor(color.r, color.g, color.b, color.a);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void OpenglRenderer::present() {

    SDL_GL_SwapWindow(_window);
}


bool OpenglRenderer::load_font(const std::string& name, const std::string& path, int size) {
    return true;
}

std::shared_ptr<Texture> OpenglRenderer::load_texture(const std::string& name, const std::string& path) {


    if (_textures.contains(name)) {
        return _textures[name];
    }

    LOG_INFO("Loading texture: %s", path.c_str());

    FileAccess file_access(path, ModeFlags::READ);

    if (!file_access.is_open()) {
        LOG_ERROR("Failed to open texture file: %s", path.c_str());
        return nullptr;
    }

    SDL_Surface* surf = IMG_Load_IO(file_access.get_handle(), false);
    // SDL_Surface* surf = IMG_Load(path.c_str());

    if (!surf) {
        LOG_ERROR("Failed to load texture: %s, Error: %s", path.c_str(), SDL_GetError());
        return nullptr;
    }

    auto texture = std::make_shared<Texture>();

    surf = SDL_ConvertSurface(surf, SDL_PIXELFORMAT_RGBA32);

    if (!surf) {
        LOG_ERROR("Failed to convert texture to RGBA32: %s", SDL_GetError());
        SDL_DestroySurface(surf);
        return nullptr;
    }

    GLuint texID;
    glGenTextures(1, &texID);
    glBindTexture(GL_TEXTURE_2D, texID);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, surf->w, surf->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, surf->pixels);
    glGenerateMipmap(GL_TEXTURE_2D);

    texture->id     = texID;
    texture->width  = surf->w;
    texture->height = surf->h;
    texture->path   = path;

    LOG_INFO("Texture Info: Id %d Size %dx%d Path: %s", texture->id, surf->w, surf->h, path.c_str());

    _textures[name] = texture;

    SDL_DestroySurface(surf);
    return texture;
}


std::shared_ptr<Model> OpenglRenderer::load_model(const char* path) {

    if (_models.contains(path)) {
        return _models[path];
    }

    std::string base_dir = ASSETS_PATH + path;
    // FileAccess file(path, ModeFlags::READ);

    // std::string base_dir = file.get_path();
    // if (!file.is_open()) {
    //     LOG_ERROR("Failed to open model file: %s", path);
    //     return nullptr;
    // }

    // const auto buffer = file.get_file_as_bytes();

    // if (buffer.empty()) {
    //     LOG_ERROR("Failed to import Model");
    //     return nullptr;
    // }

    Assimp::Importer importer;

    std::string ext = base_dir.substr(base_dir.find_last_of(".") + 1);

    const auto ASSIMP_FLAGS = aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_JoinIdenticalVertices | aiProcess_GenSmoothNormals
                            | aiProcess_OptimizeMeshes | aiProcess_OptimizeGraph;

    // const aiScene* scene = importer.ReadFileFromMemory(buffer.data(), buffer.size(), ASSIMP_FLAGS, ext.c_str());

    // NOTE: on android this doesnt work, needs to read from memory buffer
    const aiScene* scene = importer.ReadFile(base_dir, ASSIMP_FLAGS);

    if (!scene || !scene->mRootNode) {
        LOG_ERROR("Failed to load Model: %s, Error: %s", path, importer.GetErrorString());
        return nullptr;
    }

    auto model  = std::make_shared<Model>();
    model->path = path;

    for (unsigned int i = 0; i < scene->mNumMaterials; i++) {
        aiMaterial* mat = scene->mMaterials[i];
        aiString name;
        if (mat->Get(AI_MATKEY_NAME, name) == AI_SUCCESS) {
            LOG_DEBUG("Material %d/%d Name: %s", i + 1, scene->mNumMaterials, name.C_Str());
        }
    }

    base_dir = base_dir.substr(0, base_dir.find_last_of("/\\") + 1);
    for (unsigned int i = 0; i < scene->mNumMeshes; i++) {
        LOG_DEBUG("Loading Mesh %d/%d  Name: %s", i + 1, scene->mNumMeshes, scene->mMeshes[i]->mName.C_Str());
        model->meshes.push_back(load_mesh(scene->mMeshes[i], scene, base_dir));
    }

    _models[path] = model;
    LOG_INFO("Loaded Model: %s | Mesh Count: %zu | FileFormat: %s", path, model->meshes.size(), ext.c_str());

    return model;
}


void OpenglRenderer::draw_line_3d(const glm::vec3& from, const glm::vec3& to, const glm::vec4& color) {
}

void OpenglRenderer::draw_triangle_3d(const glm::vec3& v1, const glm::vec3& v2, const glm::vec3& v3, const glm::vec4& color,
                                      bool is_filled) {
}

std::unique_ptr<Mesh> OpenglRenderer::load_mesh(aiMesh* mesh, const aiScene* scene, const std::string& base_dir) {
    auto m  = std::make_unique<OpenglMesh>();
    m->name = mesh->mName.C_Str();


    if (scene->mNumMaterials > mesh->mMaterialIndex) {
        aiMaterial* mat = scene->mMaterials[mesh->mMaterialIndex];

        aiColor3D c(0.5f, 0.5f, 0.5f);
        mat->Get(AI_MATKEY_COLOR_DIFFUSE, c);
        m->material.albedo = {c.r, c.g, c.b};

        aiString texPath;
        if (mat->GetTexture(aiTextureType_DIFFUSE, 0, &texPath) == AI_SUCCESS) {
            const std::string texture_path = base_dir + texPath.C_Str();
            const auto tex                 = load_texture(texPath.C_Str(), texture_path);
            m->texture_id                  = tex ? tex->id : 0;
        }

        // m->material.shininess = mat->Get(AI_MATKEY_SHININESS, m->material.shininess) == AI_SUCCESS ? m->material.shininess : 1.0f;
    }

    std::vector<float> verts; // vertex attributes
    std::vector<unsigned int> indices; // element indices

    verts.reserve(mesh->mNumVertices * 8);
    m->vertices.reserve(mesh->mNumVertices);
    indices.reserve(mesh->mNumFaces * 3);

    for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
        const aiVector3D& v = mesh->mVertices[i];
        verts.push_back(v.x);
        verts.push_back(v.y);
        verts.push_back(v.z);
        m->vertices.push_back(glm::vec3(v.x, v.y, v.z));

        if (mesh->HasNormals()) {
            verts.push_back(mesh->mNormals[i].x);
            verts.push_back(mesh->mNormals[i].y);
            verts.push_back(mesh->mNormals[i].z);
        } else {
            verts.push_back(0);
            verts.push_back(0);
            verts.push_back(0);
        }

        if (mesh->HasTextureCoords(0)) {
            verts.push_back(mesh->mTextureCoords[0][i].x);
            verts.push_back(mesh->mTextureCoords[0][i].y);
        } else {
            verts.push_back(0);
            verts.push_back(0);
        }
    }

    for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
        const aiFace& face = mesh->mFaces[i];
        for (unsigned int j = 0; j < face.mNumIndices; j++) {
            indices.push_back(face.mIndices[j]);
        }
    }

    m->vertex_count = mesh->mNumVertices;
    m->index_count  = indices.size();

    glGenVertexArrays(1, &m->vao);
    glGenBuffers(1, &m->vbo);
    glGenBuffers(1, &m->ebo);

    glBindVertexArray(m->vao);

    glBindBuffer(GL_ARRAY_BUFFER, m->vbo);
    glBufferData(GL_ARRAY_BUFFER, verts.size() * sizeof(float), verts.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m->ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*) offsetof(Vertex, position));
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*) offsetof(Vertex, normal));
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*) offsetof(Vertex, uv));
    glEnableVertexAttribArray(2);

    glBindVertexArray(0);

    return m;
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

void OpenglRenderer::flush(const glm::mat4& view, const glm::mat4& projection) {
    // DrawInstanced
    for (auto& [_, batch] : _instanced_batches) {
        const Mesh* mesh = batch.mesh;
        auto shader      = batch.shader;
        auto& models     = batch.models;

        if (!mesh || !shader || models.empty()) {
            continue;
        }

        OpenglShader* ogl_shader = static_cast<OpenglShader*>(shader);
        shader->activate();
        ogl_shader->set_value("VIEW", view);
        ogl_shader->set_value("PROJECTION", projection);

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

        if (ogl_mesh->has_texture()) {
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, ogl_mesh->texture_id);
            ogl_shader->set_value("USE_TEXTURE", 1);
            ogl_shader->set_value("material.albedo", glm::vec3(1.0f));
        } else {


            ogl_shader->set_value("USE_TEXTURE", 0);

            // Hack fix??
            if (batch.command == EDrawCommand::MODEL) {
                ogl_shader->set_value("material.albedo", mesh->material.albedo);
            } else {
                ogl_shader->set_value("material.albedo", glm::vec3(1.0f, 1.0f, 1.0f));
            }
        }


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

    glBindVertexArray(0);

    _instanced_batches.clear();
    draw_environment(view, projection);
}


void OpenglRenderer::draw_mesh(const Transform3D& transform, const MeshInstance3D& mesh, const Shader* shader) {

    if (!cube_mesh) {
        return;
    }

    Transform3D temp = transform;
    temp.scale       = mesh.size;

    auto& batch                 = _instanced_batches[cube_mesh.get()];
    batch.mesh                  = cube_mesh.get();
    batch.mesh->material.albedo = mesh.material.albedo;
    batch.shader                = default_shader;
    batch.models.push_back(temp.get_model_matrix());
    batch.colors.push_back(mesh.material.albedo);
    batch.command = EDrawCommand::MESH;
    batch.mode    = GEngine->get_config().is_debug ? EDrawMode::LINES : EDrawMode::TRIANGLES;
}

void OpenglRenderer::draw_environment(const glm::mat4& view, const glm::mat4& projection) {


    if (skybox_mesh == nullptr || skybox_mesh->texture_id == 0) {
        return;
    }

    glDepthFunc(GL_LEQUAL);

    skybox_shader->activate();

    skybox_shader->set_value("VIEW", view);
    skybox_shader->set_value("PROJECTION", projection);

    skybox_mesh->bind();
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, skybox_mesh->texture_id);

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

    _buffers.clear();

    // cubemap resources
    delete skybox_mesh;
    skybox_mesh = nullptr;

    delete skybox_shader;
    skybox_shader = nullptr;

    // default shader
    delete default_shader;
    default_shader = nullptr;
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
    default_shader->set_value("LIGHT_POSITION", glm::vec3(0, 100, 0));
    default_shader->set_value("LIGHT_COLOR", glm::vec3(1, 1, 1));
}


std::vector<Tokens> OpenglRenderer::parse_text(const std::string& text) {

    return {};
}

void OpenglRenderer::draw_text_internal(const glm::vec2& pos, const glm::vec4& color, const std::string& font_name,
                                        const std::string& text) {
}
