#include "core/renderer/opengl/ogl_renderer.h"

#include "core/engine.h"


void GLAPIENTRY
ogl_validation_layer(GLenum source,
                     GLenum type,
                     GLuint id,
                     GLenum severity,
                     GLsizei length,
                     const GLchar* message,
                     const void* userParam) {
    LOG_DEBUG("ValidationLayer: %s type = 0x%x, severity = 0x%x, message = %s\n",
              (type == GL_DEBUG_TYPE_ERROR ? "(OGL_ERROR)" : "(OGL_WARN)"),
              type, severity, message);
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
        return nullptr;
    }

#else

    if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(SDL_GL_GetProcAddress))) {
        LOG_CRITICAL("Failed to initialize GLAD (GL_FUNCTIONS)");
        return false;
    }

#endif

    _context = glContext;
    _window  = window;


    default_shader_program = setup_default_shaders();

    if (default_shader_program == 0) {
        LOG_CRITICAL("Failed to setup default shaders");
        return false;
    }

    const auto& viewport = GEngine->get_config().get_viewport();
    LOG_INFO("Using backend: %s, Viewport: %dx%d", "Opengl/ES 3.X", viewport.width, viewport.height);


    glEnable(GL_DEBUG_OUTPUT);
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
    glDebugMessageCallback(ogl_validation_layer, nullptr);

    glEnable(GL_CULL_FACE_MODE);
    glCullFace(GL_BACK);

    glEnable(GL_DEPTH_TEST);

    glViewport(0, 0, viewport.width, viewport.height);

    return true;
}

void OpenglRenderer::clear(glm::vec4 color) {

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
    std::string full_path  = path;

    if (path.rfind("res/", 0) != 0) {
        full_path = ASSETS_PATH + path;
    }


    SDL_Surface* surf = IMG_Load(full_path.c_str());

    if (!surf) {
        LOG_ERROR("Failed to load texture: %s, Error: %s", full_path.c_str(), SDL_GetError());
        return nullptr;
    }

    auto texture = std::make_shared<Texture>();

    SDL_Surface* conv = SDL_ConvertSurface(surf, SDL_PIXELFORMAT_RGBA32);

    if (!conv) {
        LOG_ERROR("Failed to convert texture to RGBA32: %s", SDL_GetError());
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
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, conv->w, conv->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, conv->pixels);
    glGenerateMipmap(GL_TEXTURE_2D);

    texture->id     = texID;
    texture->width  = conv->w;
    texture->height = conv->h;
    texture->path   = path;

    LOG_INFO("Texture Info: Id %d, Size %dx%d, Path: %s", texture->id, conv->w, conv->h, full_path.c_str());

    _textures[name] = texture;

    SDL_DestroySurface(conv);
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

    base_dir = base_dir.substr(0, base_dir.find_last_of("/\\") + 1);
    for (unsigned int i = 0; i < scene->mNumMeshes; i++) {
        model->meshes.push_back(load_meshes(scene->mMeshes[i], scene, base_dir));
    }


    _models[path] = model;
    LOG_INFO("Loaded Model: %s, Mesh Count: %llu", path, model->meshes.size());

    return model;
}

std::shared_ptr<Mesh> OpenglRenderer::load_meshes(aiMesh* mesh, const aiScene* scene, const std::string& base_dir) {
    std::shared_ptr<Mesh> m = std::make_shared<Mesh>();
    if (scene->mNumMaterials > mesh->mMaterialIndex) {
        aiMaterial* mat = scene->mMaterials[mesh->mMaterialIndex];
        aiColor3D c(0.5f, 0.5f, 0.5f);
        mat->Get(AI_MATKEY_COLOR_DIFFUSE, c);
        m->diffuse_color = {c.r, c.g, c.b};
        aiString texPath;
        if (mat->GetTexture(aiTextureType_DIFFUSE, 0, &texPath) == AI_SUCCESS) {
            const std::string full = base_dir + texPath.C_Str();
            const auto tex         = load_texture(texPath.C_Str(), full);
            m->texture_id          = tex ? tex->id : 0;
            m->has_texture         = m->texture_id != 0;
        }
    }

    std::vector<float> verts;
    verts.reserve(mesh->mNumFaces * 3 * 8);
    for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
        aiFace face = mesh->mFaces[i];
        for (unsigned int j = 0; j < face.mNumIndices; j++) {
            unsigned int idx = face.mIndices[j];
            verts.push_back(mesh->mVertices[idx].x);
            verts.push_back(mesh->mVertices[idx].y);
            verts.push_back(mesh->mVertices[idx].z);
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

    m->vertex_count = verts.size() / 8;
    glGenVertexArrays(1, &m->vao);
    glGenBuffers(1, &m->vbo);
    glBindVertexArray(m->vao);
    glBindBuffer(GL_ARRAY_BUFFER, m->vbo);
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

    if (!model)
        return;

    glUseProgram(default_shader_program);
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(t.get_model_matrix()));
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));
    glUniform3f(viewPosLoc, viewPos.x, viewPos.y, viewPos.z);

    for (const auto& mesh : model->meshes) {

        glUniform3f(materialDiffuseLoc, mesh->diffuse_color.r, mesh->diffuse_color.g, mesh->diffuse_color.b);
        glUniform1i(useTextureLoc, mesh->has_texture);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, mesh->texture_id);
        glBindVertexArray(mesh->vao);
        glDrawArrays(GL_TRIANGLES, 0, mesh->vertex_count);
        glBindVertexArray(0);

    }
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

    glDeleteProgram(default_shader_program);
    SDL_GL_DestroyContext(_context);
}


#if defined(SDL_PLATFORM_ANDROID) || defined(SDL_PLATFORM_IOS) || defined(SDL_PLATFORM_EMSCRIPTEN)
#define SHADER_HEADER "#version 300 es\nprecision mediump float;\n"
#else
#define SHADER_HEADER "#version 330 core\n"
#endif

GLuint OpenglRenderer::setup_default_shaders() {

    EMBER_TIMER_START();

    auto compile = [](const std::string& src, GLenum type) -> GLuint {
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
    };

    const std::string vertexSource   = SHADER_HEADER + load_assets_file("shaders/opengl/default.vert");
    const std::string fragmentSource = SHADER_HEADER + load_assets_file("shaders/opengl/default.frag");

    GLuint v = compile(vertexSource, GL_VERTEX_SHADER);
    GLuint f = compile(fragmentSource, GL_FRAGMENT_SHADER);

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
        return 0;
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

    glUseProgram(prog);
    glUniform3f(lightPosLoc, 10, -30, 10);
    glUniform3f(lightColorLoc, 1, 1, 1);
    glUniform1i(textureSamplerLoc, 0);

    EMBER_TIMER_END("Baking default shaders");

    return prog;
}

std::string OpenglRenderer::vformat(const char* fmt, va_list args) {

    return "Hello";
}


std::vector<Tokens> OpenglRenderer::parse_text(const std::string& text) {

    return {};
}

void OpenglRenderer::draw_text_internal(const glm::vec2& pos, const glm::vec4& color, const std::string& font_name,
                                        const std::string& text) {
}
