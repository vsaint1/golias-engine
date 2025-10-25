#include  "core/renderer/opengl/ogl_renderer.h"
#include  "core/engine.h"

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
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_COMPATIBILITY);


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

    width  = w;
    height = h;

    if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(SDL_GL_GetProcAddress))) {
        spdlog::error("Failed to initialize GLAD");
        return false;
    }

    glEnable(GL_DEPTH_TEST);
    glViewport(0, 0, width, height);

    _default_shader = std::make_unique<OpenglShader>("shaders/opengl/default.vert", "shaders/opengl/default.frag");
    _shadow_shader  = std::make_unique<OpenglShader>("shaders/opengl/shadow.vert", "shaders/opengl/shadow.frag");

    FramebufferSpecification spec;
    spec.height      = 2048;
    spec.width       = 2048;
    spec.attachments = {
        {FramebufferTextureFormat::DEPTH_COMPONENT}
    };

    shadow_map_fbo = std::make_shared<OpenGLFramebuffer>(spec);

    return true;
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

GLuint OpenGLRenderer::load_texture_from_memory(const unsigned char* buffer, size_t size, const std::string& name) {
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

void OpenGLRenderer::begin_shadow_pass() {
    shadow_map_fbo->bind();
    glEnable(GL_DEPTH_TEST);
    glClear(GL_DEPTH_BUFFER_BIT);
    _shadow_shader->activate();
}

void OpenGLRenderer::render_shadow_pass(const Transform3D& transform, const MeshInstance3D& mesh, const glm::mat4& light_space_matrix) {
    glm::mat4 model = transform.get_matrix();

    _shadow_shader->set_value("LIGHT_MATRIX", light_space_matrix, 1);
    _shadow_shader->set_value("MODEL", model, 1);

    glBindVertexArray(mesh.VAO);
    glDrawElements(GL_TRIANGLES, mesh.indexCount, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

void OpenGLRenderer::end_shadow_pass() {
    shadow_map_fbo->unbind();
    glCullFace(GL_BACK);
    glViewport(0, 0, width, height);
}

void OpenGLRenderer::begin_render_target() {
    glClearColor(0.1f, 0.1f, 0.15f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    _default_shader->activate();
    glViewport(0, 0, width, height);
}

void OpenGLRenderer::render_entity(const Transform3D& transform, const MeshInstance3D& mesh, const Material& material,
                                   const Camera3D& camera,
                                   const glm::mat4& light_space_matrix, const std::vector<DirectionalLight>& directional_lights,
                                   const std::vector<std::pair<Transform3D, SpotLight>>& spot_lights) {
    glm::mat4 model = transform.get_matrix();

    // TODO:  PASS camera transform directly
    auto camera_query = GEngine->get_world().query<const Transform3D, const Camera3D>();

    const Transform3D& camera_transform = camera_query.first().get<Transform3D>();

    glm::mat4 view       = camera.get_view(camera_transform);
    glm::mat4 projection = camera.get_projection(width, height);

    _default_shader->set_value("MODEL", model);
    _default_shader->set_value("VIEW", view);
    _default_shader->set_value("PROJECTION", projection);
    _default_shader->set_value("LIGHT_MATRIX", light_space_matrix);

    // Camera
    _default_shader->set_value("CAMERA_POSITION_WORLD", camera_transform.position);


    material.bind(_default_shader.get());


    // REFACTORED: Directional lights using DirectionalLight class
    _default_shader->set_value("numDirLights", static_cast<int>(directional_lights.size()));
    if (!directional_lights.empty()) {
        std::vector<glm::vec3> directions;
        std::vector<glm::vec3> colors;
        std::vector<int> castShadows;

        for (const auto& light : directional_lights) {
            directions.push_back(light.direction);
            colors.push_back(light.color * light.intensity);
            castShadows.push_back(light.castShadows ? 1 : 0);
        }

        for (int i = 0; i < static_cast<int>(directional_lights.size()); ++i) {
            _default_shader->set_value(fmt::format("dirLights[{}].direction", i), directions[i]);
            _default_shader->set_value(fmt::format("dirLights[{}].color", i), colors[i]);
            _default_shader->set_value(fmt::format("dirLights[{}].cast_shadows", i), castShadows[i]);
        }

    }

    _default_shader->set_value("numSpotLights", static_cast<int>(spot_lights.size()));
    if (!spot_lights.empty()) {
        std::vector<glm::vec3> positions;
        std::vector<glm::vec3> directions;
        std::vector<glm::vec3> colors;
        std::vector<float> cutOffs;
        std::vector<float> outerCutOffs;

        for (const auto& [transform, light] : spot_lights) {
            positions.push_back(transform.position);
            directions.push_back(light.direction);
            colors.push_back(light.color * light.intensity);
            cutOffs.push_back(glm::cos(glm::radians(light.cutOff)));
            outerCutOffs.push_back(glm::cos(glm::radians(light.outerCutOff)));
        }

        for (int i = 0; i < static_cast<int>(spot_lights.size()); ++i) {
            _default_shader->set_value(fmt::format("spotLights[{}].position", i), positions[i]);
            _default_shader->set_value(fmt::format("spotLights[{}].direction", i), directions[i]);
            _default_shader->set_value(fmt::format("spotLights[{}].color", i), colors[i]);
            _default_shader->set_value(fmt::format("spotLights[{}].inner_cut_off", i), cutOffs[i]);
            _default_shader->set_value(fmt::format("spotLights[{}].outer_cut_off", i), outerCutOffs[i]);
        }

    }

    glActiveTexture(GL_TEXTURE0 + SHADOW_TEXTURE_UNIT);
    glBindTexture(GL_TEXTURE_2D, shadow_map_fbo->get_depth_attachment_id());
    _default_shader->set_value("SHADOW_MAP", SHADOW_TEXTURE_UNIT);

    glBindVertexArray(mesh.VAO);
    glDrawElements(GL_TRIANGLES, mesh.indexCount, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

void OpenGLRenderer::end_render_target() {
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

    _default_shader->destroy();
    _shadow_shader->destroy();

    SDL_GL_DestroyContext(_context);

}

void OpenGLRenderer::swap_chain() {
    SDL_GL_SwapWindow(_window);
}
