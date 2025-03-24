#include "core/renderer/ember_gl.h"

#include <stb_image.h>


Renderer* CreateRenderer(SDL_Window* window, int view_width, int view_height) {

#if defined(SDL_PLATFORM_IOS) || defined(SDL_PLATFORM_ANDROID) || defined(SDL_PLATFORM_EMSCRIPTEN)

    /* GLES 3.0 -> GLSL: 300 */
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);

#elif defined(SDL_PLATFORM_WINDOWS) || defined(SDL_PLATFORM_LINUX) || defined(SDL_PLATFORM_MACOS)

    /* OPENGL 3.3 -> GLSL: 330*/
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

#endif

    SDL_GLContext glContext = SDL_GL_CreateContext(window);

    if (!glContext) {
        LOG_CRITICAL("Failed to create GL context");
        return nullptr;
    }

#if defined(SDL_PLATFORM_IOS) || defined(SDL_PLATFORM_ANDROID)

    if (!gladLoadGLLoader((GLADloadproc) SDL_GL_GetProcAddress)) {
        LOG_CRITICAL("Failed to initialize GLAD (GL_FUNCTIONS)");
        return nullptr;
    }

#else

    if (!gladLoadGLES2Loader((GLADloadproc) SDL_GL_GetProcAddress)) {
        LOG_CRITICAL("Failed to initialize GLAD (GLES_FUNCTIONS)");
        return nullptr;
    }

#endif

    Renderer* _renderer           = new Renderer; // TODO: use smart ptrs
    _renderer->OpenGL.viewport[0] = view_width;
    _renderer->OpenGL.viewport[1] = view_height;
    _renderer->window             = window;
    _renderer->OpenGL.context     = glContext;

    unsigned int shaderProgram      = CreateShaderProgram();
    _renderer->OpenGL.shaderProgram = shaderProgram;

    // Generate once and reuse xd
    float vertices[] = {
        // pos         // tex coords
        0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
        1.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f,
    };

    glGenVertexArrays(1, &_renderer->OpenGL.vao);
    glGenBuffers(1, &_renderer->OpenGL.vbo);

    /* Just a quick note, we create 1 and reuse many times (idk if it's bad or not but the purpose is to be simple) */
    glBindVertexArray(_renderer->OpenGL.vao);
    glBindBuffer(GL_ARRAY_BUFFER, _renderer->OpenGL.vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_DYNAMIC_DRAW);


    // TODO: if window resize, update viewport
    glViewport(0, 0, view_width, view_height);

    renderer = _renderer;

    return _renderer;
}

Renderer* GetRenderer() {
    LOG_INFO("Using backend %s",renderer->type == RendererType::OPENGL ? "OpenGL" : "Metal");
    return renderer;
}

void CloseWindow() {
    glDeleteProgram(renderer->OpenGL.shaderProgram);
    glDeleteVertexArrays(1, &renderer->OpenGL.vao);
    glDeleteBuffers(1, &renderer->OpenGL.vbo);

    SDL_GL_DestroyContext(renderer->OpenGL.context);

    SDL_DestroyWindow(renderer->window);

    delete renderer;
}

unsigned int CompileShader(unsigned int type, const char* src) {
    unsigned int shader = glCreateShader(type);
    glShaderSource(shader, 1, &src, nullptr);
    glCompileShader(shader);

    int success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char info_log[512];
        glGetShaderInfoLog(shader, 512, nullptr, info_log);
        const char* type_str = type == GL_VERTEX_SHADER ? "VERTEX" : "FRAGMENT";
        LOG_CRITICAL("[%s] - Shader compilation failed: %s", type_str, info_log);
    }

    LOG_INFO("Successfully compiled %s", type == GL_VERTEX_SHADER ? "VERTEX" : "FRAGMENT");

    return shader;
}

unsigned int CreateShaderProgram() {

    const auto vertexShaderSrc   = SHADER_HEADER + LoadAssetsFile("shaders/default_vert.glsl");
    const auto fragmentShaderSrc = SHADER_HEADER + LoadAssetsFile("shaders/default_frag.glsl");

    unsigned int vs = CompileShader(GL_VERTEX_SHADER, vertexShaderSrc.c_str());
    unsigned int fs = CompileShader(GL_FRAGMENT_SHADER, fragmentShaderSrc.c_str());

    unsigned int program = glCreateProgram();

    glAttachShader(program, vs);
    glAttachShader(program, fs);
    glLinkProgram(program);

    // WARN: since our project is simple, we don't need to delete shader's ( it help's with debugging )
    glDeleteShader(vs);
    glDeleteShader(fs);
    return program;
}

void ClearBackground(Color color) {
    glm::vec4 norm_color = {color.r / 255.0f, color.g / 255.0f, color.b / 255.0f, color.a / 255.0f};

    glClearColor(norm_color.r, norm_color.g, norm_color.b, norm_color.a);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}


void BeginDrawing() {

    core.Time.current  = SDL_GetTicks() / 1000.f;
    core.Time.previous = core.Time.current;

    glm::mat4 projection =
        glm::ortho(0.0f, (float) renderer->OpenGL.viewport[0], (float) renderer->OpenGL.viewport[1], 0.0f);

    glUseProgram(renderer->OpenGL.shaderProgram);
    glUniformMatrix4fv(glGetUniformLocation(renderer->OpenGL.shaderProgram, "u_Projection"), 1, GL_FALSE,
                       glm::value_ptr(projection));
}

void EndDrawing() {
    SDL_GL_SwapWindow(renderer->window);
}

Texture LoadTexture(const std::string& file_path) {
    int w, h, channels;

    stbi_set_flip_vertically_on_load(false);

    auto path = (ASSETS_PATH + file_path);

    unsigned char* data = stbi_load(path.c_str(), &w, &h, &channels, 4);

    if (!data) {
        LOG_ERROR("Failed to load texture with path: %s", path.c_str());
        return {};
    }

    // TODO: we should create a simple texture if failed loading.
    unsigned int texId;
    glGenTextures(1, &texId);
    glBindTexture(GL_TEXTURE_2D, texId);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST); // GL_NEAREST is better for pixel art style

    stbi_image_free(data);
    LOG_INFO("Loaded texture with ID: %d, path: %s", texId, path.c_str());
    LOG_INFO(" > Width %d, Height %d", w, h);
    LOG_INFO(" > Num. Channels %d", channels);

    return {texId, w, h};
}


void UnloadTexture(Texture texture) {
    glDeleteTextures(1, &texture.id);
}


Font LoadFont(const std::string& file_path, int font_size) {
    Font font = {};

    auto path = ASSETS_PATH + file_path;

    SDL_IOStream* file_rw = SDL_IOFromFile(path.c_str(), "rb");
    if (!file_rw) {
        LOG_ERROR("Failed to open font file %s", path.c_str());
        return font;
    }

    Sint64 size = SDL_GetIOSize(file_rw);
    if (size <= 0) {
        LOG_ERROR("Failed to get file size %s", path.c_str());
        SDL_CloseIO(file_rw);
        return font;
    }

    std::vector<unsigned char> font_buffer(size);
    if (SDL_ReadIO(file_rw, font_buffer.data(), size) != size) {
        LOG_ERROR("Failed to read file %s", path.c_str());
        SDL_CloseIO(file_rw);
        return font;
    }

    SDL_CloseIO(file_rw);

    stbtt_fontinfo font_info;
    stbtt_InitFont(&font_info, font_buffer.data(), stbtt_GetFontOffsetForIndex(font_buffer.data(), 0));

    float scale = stbtt_ScaleForPixelHeight(&font_info, (float) font_size);
    int ascent, descent, lineGap;
    stbtt_GetFontVMetrics(&font_info, &ascent, &descent, &lineGap);

    int atlas_w = 512;
    int atlas_h = 512;
    std::vector<unsigned char> bitmap(atlas_w * atlas_h, 0);

    int x = 0, y = 0, max_row_height = 0;

    for (char c = 32; c < 127; ++c) {
        int ax, lsb;
        stbtt_GetCodepointHMetrics(&font_info, c, &ax, &lsb);

        int x0, y0, x1, y1;
        stbtt_GetCodepointBitmapBox(&font_info, c, scale, scale, &x0, &y0, &x1, &y1);

        int gw = x1 - x0;
        int gh = y1 - y0;

        if (x + gw >= atlas_w) {
            x = 0;
            y += max_row_height;
            max_row_height = 0;
        }

        max_row_height = SDL_max(max_row_height, gh);

        stbtt_MakeCodepointBitmap(&font_info, &bitmap[y * atlas_w + x], gw, gh, atlas_w, scale, scale, c);

        Glyph glyph;
        glyph.x0       = (float) x / atlas_w;
        glyph.y0       = (float) y / atlas_h;
        glyph.x1       = (float) (x + gw) / atlas_w;
        glyph.y1       = (float) (y + gh) / atlas_h;
        glyph.w        = gw;
        glyph.h        = gh;
        glyph.x_offset = x0;
        glyph.y_offset = y0;
        glyph.advance  = (int) (ax * scale);

        font.glyphs[c] = glyph;

        x += gw;
    }

    std::vector<unsigned char> rgba_buffer(atlas_w * atlas_h * 4);

    for (int i = 0; i < atlas_w * atlas_h; ++i) {
        unsigned char gray     = bitmap[i];
        rgba_buffer[i * 4 + 0] = gray; // R
        rgba_buffer[i * 4 + 1] = gray; // G
        rgba_buffer[i * 4 + 2] = gray; // B
        rgba_buffer[i * 4 + 3] = gray; // A
    }

    glGenTextures(1, &font.texture.id);
    glBindTexture(GL_TEXTURE_2D, font.texture.id);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, atlas_w, atlas_h, 0, GL_RGBA, GL_UNSIGNED_BYTE, rgba_buffer.data());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    LOG_INFO("Loaded font with ID: %d, path: %s", font.texture.id, path.c_str());
    LOG_INFO(" > Width %d, Height %d", atlas_w, atlas_h);
    LOG_INFO(" > Num. Glyphs %zu", font.glyphs.size());

    font.ascent         = ascent;
    font.descent        = descent;
    font.line_gap       = lineGap;
    font.scale          = scale;
    font.font_size      = font_size;
    font.texture.width  = atlas_w;
    font.texture.height = atlas_h;

    return font;
}

void DrawText(Font& font, const std::string& text, glm::vec2 position, Color color, float scale, float kerning) {

    if (text.empty()) {
        return;
    }

    scale = SDL_clamp(scale, 0.0f, 1.0f);

    glActiveTexture(GL_TEXTURE0 + font.texture.id);
    glBindTexture(GL_TEXTURE_2D, font.texture.id);
    glUniform1i(glGetUniformLocation(renderer->OpenGL.shaderProgram, "u_Texture"), font.texture.id);

    glm::vec4 norm_color = {
        color.r / 255.0f,
        color.g / 255.0f,
        color.b / 255.0f,
        color.a / 255.0f,
    };

    glUniform4fv(glGetUniformLocation(renderer->OpenGL.shaderProgram, "u_Color"), 1, glm::value_ptr(norm_color));

    glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(position.x, position.y, 0.0f));
    model           = glm::scale(model, glm::vec3(scale, scale, 1.0f));

    glUniformMatrix4fv(glGetUniformLocation(renderer->OpenGL.shaderProgram, "u_Model"), 1, GL_FALSE,
                       glm::value_ptr(model));

    float cursor_x = 0.0f;
    float cursor_y = 0.0f;
    float start_x  = cursor_x;

    std::vector<Vertex> vertices;
    vertices.reserve(text.size() * 6);

    for (char c : text) {
        if (c == '\n') {
            cursor_x = start_x;
            cursor_y += font.font_size;
            continue;
        }

        if (font.glyphs.find(c) == font.glyphs.end()) {
            continue;
        }

        const Glyph& g = font.glyphs[c];

        float x0 = cursor_x + g.x_offset;
        float y0 = cursor_y + g.y_offset;
        float x1 = x0 + g.w;
        float y1 = y0 + g.h;

        float u0 = g.x0;
        float v0 = g.y0;
        float u1 = g.x1;
        float v1 = g.y1;

        vertices.push_back({{x0, y0, 0.0f}, {u0, v0}});
        vertices.push_back({{x1, y0, 0.0f}, {u1, v0}});
        vertices.push_back({{x1, y1, 0.0f}, {u1, v1}});

        vertices.push_back({{x0, y0, 0.0f}, {u0, v0}});
        vertices.push_back({{x1, y1, 0.0f}, {u1, v1}});
        vertices.push_back({{x0, y1, 0.0f}, {u0, v1}});

        cursor_x += g.advance + kerning;
    }

    glBindBuffer(GL_ARRAY_BUFFER, renderer->OpenGL.vbo);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*) offsetof(Vertex, position));
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*) offsetof(Vertex, texCoord));
    glEnableVertexAttribArray(1);

    glBindVertexArray(renderer->OpenGL.vao);
    glDrawArrays(GL_TRIANGLES, 0, vertices.size());

    glBindTexture(GL_TEXTURE_2D, 0);  

}


void DrawTexture(Texture2D texture, Rectangle rect, Color color) {

    glActiveTexture(GL_TEXTURE0 + texture.id);
    glBindTexture(GL_TEXTURE_2D, texture.id);
    glUniform1i(glGetUniformLocation(renderer->OpenGL.shaderProgram, "u_Texture"), texture.id);

    glm::vec4 norm_color = {
        color.r / 255.0f,
        color.g / 255.0f,
        color.b / 255.0f,
        color.a / 255.0f,
    };

    glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(rect.x, rect.y, 0.0f));
    model           = glm::scale(model, glm::vec3(rect.width, rect.height, 1.0f));

    glm::mat4 projection =
        glm::ortho(0.0f, (float) renderer->OpenGL.viewport[0], (float) renderer->OpenGL.viewport[1], 0.0f);

    glUniformMatrix4fv(glGetUniformLocation(renderer->OpenGL.shaderProgram, "u_Model"), 1, GL_FALSE,
                       glm::value_ptr(model));

    glUniform4fv(glGetUniformLocation(renderer->OpenGL.shaderProgram, "u_Color"), 1, glm::value_ptr(norm_color));

     // TODO: change to vertex struct
    float vertices[] = {
        // pos         // tex coords
        0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
        1.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f,
    };

    glBindBuffer(GL_ARRAY_BUFFER, renderer->OpenGL.vbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*) 0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*) (3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(renderer->OpenGL.vao);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

    glBindTexture(GL_TEXTURE_2D, 0);  

}


void DrawTextureEx(Texture texture, Rectangle source, Rectangle dest, glm::vec2 origin, float rotation, Color color) {

    glActiveTexture(GL_TEXTURE0 + texture.id);
    glBindTexture(GL_TEXTURE_2D, texture.id);
    glUniform1i(glGetUniformLocation(renderer->OpenGL.shaderProgram, "u_Texture"), texture.id);

    glm::mat4 model = glm::mat4(1.0f);
    model           = glm::translate(model, glm::vec3(dest.x, dest.y, 0.0f));
    model           = glm::translate(model, glm::vec3(origin.x, origin.y, 0.0f));
    model           = glm::rotate(model, glm::radians(rotation), glm::vec3(0.0f, 0.0f, 1.0f));
    model           = glm::translate(model, glm::vec3(-origin.x, -origin.y, 0.0f));
    model           = glm::scale(model, glm::vec3(dest.width, dest.height, 1.0f));

    glm::vec4 norm_color = {
        color.r / 255.0f,
        color.g / 255.0f,
        color.b / 255.0f,
        color.a / 255.0f,
    };

    float texLeft   = (float) source.x / texture.width;
    float texRight  = (float) (source.x + source.width) / texture.width;
    float texTop    = (float) source.y / texture.height;
    float texBottom = (float) (source.y + source.height) / texture.height;

    glUniformMatrix4fv(glGetUniformLocation(renderer->OpenGL.shaderProgram, "u_Model"), 1, GL_FALSE,
                       glm::value_ptr(model));

    glUniform4fv(glGetUniformLocation(renderer->OpenGL.shaderProgram, "u_Color"), 1, glm::value_ptr(norm_color));

    // TODO: change to vertex struct
    float vertices[] = {
        // pos         // tex coords
        0.0f, 0.0f, 0.0f, texLeft,  texTop,    1.0f, 0.0f, 0.0f, texRight, texTop,
        1.0f, 1.0f, 0.0f, texRight, texBottom, 0.0f, 1.0f, 0.0f, texLeft,  texBottom,
    };

    glBindBuffer(GL_ARRAY_BUFFER, renderer->OpenGL.vbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);


    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*) 0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*) (3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(renderer->OpenGL.vao);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

    glBindTexture(GL_TEXTURE_2D, 0);  

}


void DrawLine(glm::vec2 start, glm::vec2 end, Color color, float thickness) {
    
   
    glm::vec4 norm_color = {
        color.r / 255.0f,
        color.g / 255.0f,
        color.b / 255.0f,
        color.a / 255.0f,
    };

    glm::mat4 model = glm::mat4(1.0f);

    glUniformMatrix4fv(glGetUniformLocation(renderer->OpenGL.shaderProgram, "u_Model"), 1, GL_FALSE,
                       glm::value_ptr(model));

    glUniform4fv(glGetUniformLocation(renderer->OpenGL.shaderProgram, "u_Color"), 1, glm::value_ptr(norm_color));


    Vertex vertices[2] = {
        { glm::vec3(start, 0.0f), glm::vec2(0.0f, 0.0f) },
        { glm::vec3(end,   0.0f), glm::vec2(0.0f, 0.0f) }  
    };

    glBindBuffer(GL_ARRAY_BUFFER, renderer->OpenGL.vbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*) offsetof(Vertex, position));
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*) offsetof(Vertex, texCoord));
    glEnableVertexAttribArray(1);

    glBindVertexArray(renderer->OpenGL.vao);

    glLineWidth(thickness);
    glDrawArrays(GL_LINES, 0, 2);
}
