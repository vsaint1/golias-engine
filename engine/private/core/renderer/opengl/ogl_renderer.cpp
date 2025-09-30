#include "core/renderer/opengl/ogl_renderer.h"

#include "core/engine.h"

void GLAPIENTRY ogl_validation_layer(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message,
                                     const void* userParam) {
    if (severity == GL_DEBUG_SEVERITY_NOTIFICATION) {
        return;
    }

    LOG_ERROR("ValidationLayer Type: 0x%x, Severity: 0x%x, ID: %u, Message: %s", type, severity, id, message);
}


#if defined(SDL_PLATFORM_ANDROID) || defined(SDL_PLATFORM_IOS) || defined(SDL_PLATFORM_EMSCRIPTEN)
#define SHADER_HEADER "#version 300 es\nprecision mediump float;\n"
#else
#define SHADER_HEADER "#version 330 core\n"
#endif

GLuint compile_shader(const std::string& src, GLenum type) {

    // LOG_INFO("Source :\n%s",  src.c_str());
    const GLuint s   = glCreateShader(type);
    const char* csrc = src.c_str();

    glShaderSource(s, 1, &csrc, nullptr);
    glCompileShader(s);
    GLint ok;
    glGetShaderiv(s, GL_COMPILE_STATUS, &ok);

    if (!ok) {
        char log[1024];
        glGetShaderInfoLog(s, 1024, nullptr, log);
        LOG_ERROR("Failed to compile Shader %s, Type %s", log, type == GL_VERTEX_SHADER ? "Vertex" : "Fragment");
        return 0;
    }

    return s;
}

GLuint load_cubemap_atlas(const std::string& atlasPath, CUBEMAP_ORIENTATION orient = CUBEMAP_ORIENTATION::DEFAULT) {
    std::string full = atlasPath;
    if (atlasPath.rfind("res/", 0) != 0) {
        full = ASSETS_PATH + atlasPath;
    }

    SDL_Surface* surf = IMG_Load(full.c_str());
    if (!surf) {
        LOG_ERROR("Failed to load %s -> %s", full.c_str(), SDL_GetError());
        return 0;
    }

    LOG_INFO("Loaded surface (pre-convert): %dx%d, Pitch: %d", surf->w, surf->h, surf->pitch);

    surf = SDL_ConvertSurface(surf, SDL_PIXELFORMAT_RGBA32);
    if (!surf) {
        LOG_ERROR("Conversion failed %s", full.c_str());
        return 0;
    }

    const int W = surf->w;
    const int H = surf->h;
    LOG_INFO("Converted surface: %dx%d, Pitch: %d, BytesPerPixel: %d", W, H, surf->pitch, SDL_BYTESPERPIXEL(surf->format));

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
        LOG_INFO("Uploading face %d: x=%d y=%d w=%d h=%d pitch=%d", i, r.x, r.y, r.w, r.h, pitch);

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

    LOG_INFO("Loaded Cubemap atlas %s (%dx%d), Layout %d, Face %dx%d, TexID: %d", full.c_str(), W, H, layout, face_w, face_h, texID);

    SDL_DestroySurface(surf);
    return texID;
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

    glGenVertexArrays(1, &skybox_vao);
    glGenBuffers(1, &skybox_vbo);
    glBindVertexArray(skybox_vao);
    glBindBuffer(GL_ARRAY_BUFFER, skybox_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skybox_vertices), skybox_vertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*) 0);
    glBindVertexArray(0);

    const std::string vertexSource   = SHADER_HEADER + load_assets_file("shaders/opengl/skybox.vert");
    const std::string fragmentSource = SHADER_HEADER + load_assets_file("shaders/opengl/skybox.frag");

    LOG_INFO("Compiling Environment shaders");
    GLuint v = compile_shader(vertexSource, GL_VERTEX_SHADER);
    GLuint f = compile_shader(fragmentSource, GL_FRAGMENT_SHADER);

    GLuint prog = glCreateProgram();
    glAttachShader(prog, v);
    glAttachShader(prog, f);
    glLinkProgram(prog);
    GLint ok;
    glGetProgramiv(prog, GL_LINK_STATUS, &ok);

    if (!ok) {
        char log[1024];
        glGetProgramInfoLog(prog, 1024, nullptr, log);
        LOG_ERROR("Failed to link Shaders %s", log);
        return;
    }


    glDeleteShader(v);
    glDeleteShader(f);

    cubemap_viewLoc   = glGetUniformLocation(prog, "VIEW");
    cubemap_projLoc   = glGetUniformLocation(prog, "PROJECTION");
    cubemap_skyboxLoc = glGetUniformLocation(prog, "TEXTURE");

    cubemap_shader_program = prog;

    LOG_INFO("Environment setup complete");
}


bool OpenglRenderer::initialize(SDL_Window* window) {

#if defined(SDL_PLATFORM_IOS) || defined(SDL_PLATFORM_ANDROID) || defined(SDL_PLATFORM_EMSCRIPTEN)

    /* GLES 3.0 -> GLSL: 300 */
    const char* glsl_version = "#version 300 es";
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);

#elif defined(SDL_PLATFORM_WINDOWS) || defined(SDL_PLATFORM_LINUX) || defined(SDL_PLATFORM_MACOS)

    /* OPENGL 3.3 -> GLSL: 330*/
    const char* glsl_version = "#version 330";
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


    GLint num_extensions = 0;
    std::vector<std::string> extensions;
    glGetIntegerv(GL_NUM_EXTENSIONS, &num_extensions);
    for (GLuint i = 0; i < num_extensions; i++) {
        const char* ext = reinterpret_cast<const char*>(glGetStringi(GL_EXTENSIONS, i));

        if (SDL_strcasecmp(ext, "GL_KHR_debug") == 0) {
            LOG_INFO("KHR_debug extension supported, enabling validation layers");
            glEnable(GL_DEBUG_OUTPUT);
            glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
            glDebugMessageCallback(ogl_validation_layer, nullptr);
            break;
        }
    }

    LOG_WARN("KHR_debug extensions not supported, validation layers disabled");

    int major, minor;
    SDL_GL_GetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, &major);
    SDL_GL_GetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, &minor);
    LOG_INFO("Created GL Context version: %d.%d", major, minor);
    LOG_INFO("OpenGL Vendor: %s", glGetString(GL_VENDOR));
    LOG_INFO("OpenGL Renderer: %s", glGetString(GL_RENDERER));


    setup_default_shaders();

    // TODO: create api to handle environment setup
    setup_cubemap();
    skybox_texture = load_cubemap_atlas("environment_sky.png", CUBEMAP_ORIENTATION::DEFAULT);

    const auto& viewport = GEngine->get_config().get_viewport();
    LOG_INFO("Using backend: %s, Viewport: %dx%d", "Opengl/ES 3.X", viewport.width, viewport.height);

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    glEnable(GL_DEPTH_TEST);

    // glViewport(0, 0, viewport.width, viewport.height);


    return true;
}

void OpenglRenderer::clear(glm::vec4 color) {

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


    // TODO: refactor using FileAccess api
    // FileAccess file(path, ModeFlags::READ);
    std::string full_path = path;

    if (path.rfind("res/", 0) != 0) {
        full_path = ASSETS_PATH + path;
    }


    SDL_Surface* surf = IMG_Load(full_path.c_str());

    if (!surf) {
        LOG_ERROR("Failed to load texture: %s, Error: %s", full_path.c_str(), SDL_GetError());
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

    LOG_INFO("Texture Info: Id %d, Size %dx%d, Path: %s", texture->id, surf->w, surf->h, full_path.c_str());

    _textures[name] = texture;

    SDL_DestroySurface(surf);
    return texture;
}


std::shared_ptr<Model> OpenglRenderer::load_model(const char* path) {
    std::string base_dir = ASSETS_PATH + path;

    if (_models.contains(base_dir)) {
        return _models[base_dir];
    }

    Assimp::Importer importer;

    const aiScene* scene = importer.ReadFile(base_dir.c_str(), aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_FlipUVs
                                                               | aiProcess_JoinIdenticalVertices);
    if (!scene || !scene->mRootNode) {
        LOG_ERROR("Failed to load Model: %s, Error: %s", path, importer.GetErrorString());
        return nullptr;
    }

    auto model  = std::make_shared<Model>();
    model->path = base_dir;


    for (unsigned int i = 0; i < scene->mNumMaterials; i++) {
        aiMaterial* mat = scene->mMaterials[i];
        aiString name;

        if (mat->Get(AI_MATKEY_NAME, name) == AI_SUCCESS) {
            LOG_INFO("Material  %d/%d Name: %s", i + 1, scene->mNumMaterials, name.C_Str());
        }
    }

    base_dir = base_dir.substr(0, base_dir.find_last_of("/\\") + 1);
    for (unsigned int i = 0; i < scene->mNumMeshes; i++) {
        LOG_INFO("Loading Mesh %d/%d, Name: %s", i + 1, scene->mNumMeshes, scene->mMeshes[i]->mName.C_Str());
        model->meshes.push_back(load_meshes(scene->mMeshes[i], scene, base_dir));
    }

    _models[path] = model;
    LOG_INFO("Loaded Model: %s, Mesh Count:  %zu", path, model->meshes.size());

    return model;
}


void OpenglRenderer::draw_line_3d(const glm::vec3& from, const glm::vec3& to, const glm::vec4& color) {

}

void OpenglRenderer::draw_triangle_3d(const glm::vec3& v1, const glm::vec3& v2, const glm::vec3& v3, const glm::vec4& color,
                                      bool is_filled) {

  
}

Mesh OpenglRenderer::load_meshes(aiMesh* mesh, const aiScene* scene, const std::string& base_dir) {
    Mesh m;

    if (scene->mNumMaterials > mesh->mMaterialIndex) {
        aiMaterial* mat = scene->mMaterials[mesh->mMaterialIndex];
        aiColor3D c(0.5f, 0.5f, 0.5f);
        mat->Get(AI_MATKEY_COLOR_DIFFUSE, c);
        m.diffuse_color = {c.r, c.g, c.b};

        aiString texPath;
        if (mat->GetTexture(aiTextureType_DIFFUSE, 0, &texPath) == AI_SUCCESS) {
            const std::string full = base_dir + texPath.C_Str();
            const auto tex         = load_texture(texPath.C_Str(), full);
            m.texture_id           = tex ? tex->id : 0;
            m.has_texture          = m.texture_id != 0;
        }
    }

    std::vector<float> verts;
    verts.reserve(mesh->mNumFaces * 3 * 8);

    m.vertices.reserve(mesh->mNumVertices);

    for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
        aiFace face = mesh->mFaces[i];
        for (unsigned int j = 0; j < face.mNumIndices; j++) {
            unsigned int idx = face.mIndices[j];

            const aiVector3D& v = mesh->mVertices[idx];
            m.vertices.push_back(glm::vec3(v.x, v.y, v.z));

            verts.push_back(v.x);
            verts.push_back(v.y);
            verts.push_back(v.z);

            if (mesh->HasNormals()) {
                verts.push_back(mesh->mNormals[idx].x);
                verts.push_back(mesh->mNormals[idx].y);
                verts.push_back(mesh->mNormals[idx].z);
            } else {
                verts.push_back(0);
                verts.push_back(0);
                verts.push_back(0);
            }

            if (mesh->HasTextureCoords(0)) {
                verts.push_back(mesh->mTextureCoords[0][idx].x);
                verts.push_back(mesh->mTextureCoords[0][idx].y);
            } else {
                verts.push_back(0);
                verts.push_back(0);
            }
        }
    }

    m.vertex_count = verts.size() / 8;

    glGenVertexArrays(1, &m.vao);
    glGenBuffers(1, &m.vbo);
    glBindVertexArray(m.vao);
    glBindBuffer(GL_ARRAY_BUFFER, m.vbo);
    glBufferData(GL_ARRAY_BUFFER, verts.size() * sizeof(float), verts.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*) 0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*) (3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*) (6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    glBindVertexArray(0);

    return m;
}

void OpenglRenderer::draw_model(const Transform3D& t, const Model* model, const glm::mat4& view, const glm::mat4& projection,
                                const glm::vec3& viewPos) {

    if (!model) {
        return;
    }

    // TODO: wireframe mode
    const auto draw_mode = GEngine->get_config().is_debug ? GL_LINES : GL_TRIANGLES;

    glUseProgram(default_shader_program);
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(t.get_model_matrix()));
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));
    glUniform3f(viewPosLoc, viewPos.x, viewPos.y, viewPos.z);

    for (const auto& mesh : model->meshes) {

        glUniform3f(materialDiffuseLoc, mesh.diffuse_color.r, mesh.diffuse_color.g, mesh.diffuse_color.b);
        glUniform1i(useTextureLoc, mesh.has_texture);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, mesh.texture_id);
        glBindVertexArray(mesh.vao);
        glDrawArrays(draw_mode, 0, mesh.vertex_count);
        glBindVertexArray(0);
    }
}


void OpenglRenderer::draw_cube(const Transform3D& transform, const glm::mat4& view, const glm::mat4& proj, Uint32 shader) {
}

void OpenglRenderer::draw_environment(const glm::mat4& view, const glm::mat4& projection) {


    glDepthFunc(GL_LEQUAL);

    glUseProgram(cubemap_shader_program);

    glUniformMatrix4fv(cubemap_viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(cubemap_projLoc, 1, GL_FALSE, glm::value_ptr(projection));

    glBindVertexArray(skybox_vao);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, skybox_texture);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);

    glDepthFunc(GL_LESS);
}

void OpenglRenderer::draw_texture(const Transform2D& transform, Texture* texture, const glm::vec4& dest, const glm::vec4& source,
                                  bool flip_h, bool flip_v, const glm::vec4& color) {
}

void OpenglRenderer::draw_text(const Transform2D& transform, const glm::vec4& color, const std::string& font_name, const char* fmt, ...) {
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


    // cubemap resources
    glDeleteVertexArrays(1, &skybox_vao);
    glDeleteBuffers(1, &skybox_vbo);
    glDeleteTextures(1, &skybox_texture);
    glDeleteProgram(cubemap_shader_program);

    // default shader
    glDeleteProgram(default_shader_program);
    SDL_GL_DestroyContext(_context);
}

void OpenglRenderer::setup_default_shaders() {

    EMBER_TIMER_START();

    LOG_INFO("Compiling Default shaders");
    const std::string vertexSource   = SHADER_HEADER + load_assets_file("shaders/opengl/default.vert");
    const std::string fragmentSource = SHADER_HEADER + load_assets_file("shaders/opengl/default.frag");

    GLuint v = compile_shader(vertexSource, GL_VERTEX_SHADER);
    GLuint f = compile_shader(fragmentSource, GL_FRAGMENT_SHADER);

    if (!v || !f) {
        LOG_ERROR("Failed to compile default shaders");
        return;
    }

    GLuint prog = glCreateProgram();
    glAttachShader(prog, v);
    glAttachShader(prog, f);
    glLinkProgram(prog);
    GLint ok;
    glGetProgramiv(prog, GL_LINK_STATUS, &ok);

    if (!ok) {
        char log[1024];
        glGetProgramInfoLog(prog, 1024, nullptr, log);
        LOG_ERROR("Failed to link Shaders %s", log);
        return;
    }


    glDeleteShader(v);
    glDeleteShader(f);

    modelLoc           = glGetUniformLocation(prog, "MODEL");
    viewLoc            = glGetUniformLocation(prog, "VIEW");
    projLoc            = glGetUniformLocation(prog, "PROJECTION");
    viewPosLoc         = glGetUniformLocation(prog, "CAMERA_POSITION");
    lightPosLoc        = glGetUniformLocation(prog, "LIGHT_POSITION");
    lightColorLoc      = glGetUniformLocation(prog, "LIGHT_COLOR");
    materialDiffuseLoc = glGetUniformLocation(prog, "DIFFUSE");
    textureSamplerLoc  = glGetUniformLocation(prog, "TEXTURE");
    useTextureLoc      = glGetUniformLocation(prog, "USE_TEXTURE");

    default_shader_program = prog;

    glUseProgram(prog);
    glUniform3f(lightPosLoc, 10, 10, 10);
    glUniform3f(lightColorLoc, 1, 1, 1);
    glUniform1i(textureSamplerLoc, 0);

    EMBER_TIMER_END("Baking default shaders");
}


std::vector<Tokens> OpenglRenderer::parse_text(const std::string& text) {

    return {};
}

void OpenglRenderer::draw_text_internal(const glm::vec2& pos, const glm::vec4& color, const std::string& font_name,
                                        const std::string& text) {
}
