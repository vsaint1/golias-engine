#include "core/renderer/ember_gl.h"

#include <stb_image.h>


Renderer* CreateRendererGL(SDL_Window* window, int view_width, int view_height) {

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
        LOG_CRITICAL("Failed to create GL context, %s", SDL_GetError());
        return nullptr;
    }

#if defined(SDL_PLATFORM_IOS) || defined(SDL_PLATFORM_ANDROID) || defined(SDL_PLATFORM_EMSCRIPTEN)

    if (!gladLoadGLES2Loader((GLADloadproc) SDL_GL_GetProcAddress)) {
        LOG_CRITICAL("Failed to initialize GLAD (GLES_FUNCTIONS)");
        return nullptr;
    }

#else

    if (!gladLoadGLLoader((GLADloadproc) SDL_GL_GetProcAddress)) {
        LOG_CRITICAL("Failed to initialize GLAD (GL_FUNCTIONS)");
        return nullptr;
    }

#endif

    Renderer* _renderer    = new Renderer; // TODO: use smart ptrs
    _renderer->viewport[0] = view_width;
    _renderer->viewport[1] = view_height;
    _renderer->window      = window;
    _renderer->SetContext(glContext);

    _renderer->default_shader = Shader("shaders/default_vert.glsl", "shaders/default_frag.glsl");

    Vertex default_quad[] = {
        {glm::vec3(0.0f, 0.0f, 0.0f), glm::vec2(0.0f, 0.0f)}, // bl
        {glm::vec3(1.0f, 0.0f, 0.0f), glm::vec2(1.0f, 0.0f)}, // br
        {glm::vec3(1.0f, 1.0f, 0.0f), glm::vec2(1.0f, 1.0f)}, // tr

        {glm::vec3(0.0f, 0.0f, 0.0f), glm::vec2(0.0f, 0.0f)}, // bl
        {glm::vec3(1.0f, 1.0f, 0.0f), glm::vec2(1.0f, 1.0f)}, // tr
        {glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2(0.0f, 1.0f)}, // tl
    };

    glGenVertexArrays(1, &_renderer->VAO);
    glGenBuffers(1, &_renderer->VBO);

    glBindVertexArray(_renderer->VAO);
    glBindBuffer(GL_ARRAY_BUFFER, _renderer->VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(default_quad), default_quad, GL_DYNAMIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*) offsetof(Vertex, position));
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*) offsetof(Vertex, texCoord));
    glEnableVertexAttribArray(1);

    glViewport(0, 0, view_width, view_height);

    renderer = _renderer;

    return _renderer;
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
        glm::ortho(0.0f, (float) renderer->viewport[0], (float) renderer->viewport[1], 0.0f, -1.0f, 1.0f);

    glm::mat4 view = glm::mat4(1.0f);

    GetRenderer()->default_shader.Use();
    GetRenderer()->default_shader.SetValue("u_View", view);
    GetRenderer()->default_shader.SetValue("u_Projection", projection);
}

void EndDrawing() {
    SDL_GL_SwapWindow(renderer->window);
}

Texture LoadTexture(const std::string& file_path) {
    int w, h, channels;

    stbi_set_flip_vertically_on_load(false);

    const auto buffer = LoadFileIntoMemory(file_path);

    unsigned char* data = stbi_load_from_memory((unsigned char*) buffer.data(), buffer.size(), &w, &h, &channels, 4);

    if (!data) {
        LOG_ERROR("Failed to load texture with path: %s", file_path.c_str());
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

    LOG_INFO("Loaded texture with ID: %d, path: %s", texId, file_path.c_str());
    LOG_INFO(" > Width %d, Height %d", w, h);
    LOG_INFO(" > Num. Channels %d", channels);

    return {texId, w, h};
}


void UnloadTexture(Texture texture) {

    if (texture.id == 0) {
        return;
    }

    LOG_INFO("Unloading texture with ID: %d", texture.id);
    glDeleteTextures(1, &texture.id);
}


Font LoadFont(const std::string& file_path, int font_size) {
    Font font = {};

    const auto font_buffer = LoadFileIntoMemory(file_path);

    if (font_buffer.empty()) {
        LOG_ERROR("Failed to load font file into memory %s", file_path.c_str());
        return font;
    }

    stbtt_fontinfo font_info;
    stbtt_InitFont(&font_info, (unsigned char*) font_buffer.data(),
                   stbtt_GetFontOffsetForIndex((unsigned char*) font_buffer.data(), 0));

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

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    LOG_INFO("Loaded font with ID: %d, path: %s", font.texture.id, file_path.c_str());
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

void DrawText(Font& font, const std::string& text, Transform& transform, Color color, float kerning) {

    static Mesh mesh;

    if (text.empty() || !font.IsValid()) {
        static std::once_flag log_once;
        std::call_once(log_once, []() { LOG_WARN("Font not loaded, skipping draw!!!"); });
        return;
    }

    kerning = 0.0f; // TODO: fix kerning

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, font.texture.id);

    GetRenderer()->default_shader.SetValue("u_Texture", 0);

    glm::vec4 norm_color = {
        color.r / 255.0f,
        color.g / 255.0f,
        color.b / 255.0f,
        color.a / 255.0f,
    };

    GetRenderer()->default_shader.SetValue("u_Color", norm_color);

    glm::mat4 model = transform.GetModelMatrix();

    GetRenderer()->default_shader.SetValue("u_Model", model);

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

        glm::vec3 scale = transform.scale;
        float scaledX0  = x0 * scale.x;
        float scaledY0  = y0 * scale.y;
        float scaledX1  = x1 * scale.x;
        float scaledY1  = y1 * scale.y;

        vertices.push_back({{scaledX0, scaledY0, 0.0f}, {u0, v0}});
        vertices.push_back({{scaledX1, scaledY0, 0.0f}, {u1, v0}});
        vertices.push_back({{scaledX1, scaledY1, 0.0f}, {u1, v1}});

        vertices.push_back({{scaledX0, scaledY0, 0.0f}, {u0, v0}});
        vertices.push_back({{scaledX1, scaledY1, 0.0f}, {u1, v1}});
        vertices.push_back({{scaledX0, scaledY1, 0.0f}, {u0, v1}});

        cursor_x += g.advance + kerning;
    }


    mesh.Update(vertices);

    mesh.Draw(GL_TRIANGLES);
}


void DrawTexture(Texture texture, ember::Rectangle rect, Color color) {


    if (texture.id == 0) {
        static std::once_flag log_once;
        std::call_once(log_once, []() { LOG_WARN("Texture not loaded, skipping draw!!!"); });
        return;
    }

    static Mesh mesh;

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture.id);

    GetRenderer()->default_shader.SetValue("u_Texture", 0);

    glm::vec4 norm_color = {
        color.r / 255.0f,
        color.g / 255.0f,
        color.b / 255.0f,
        color.a / 255.0f,
    };

    glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(rect.x, rect.y, 0.0f));
    model           = glm::scale(model, glm::vec3(rect.width, rect.height, 1.0f));

    GetRenderer()->default_shader.SetValue("u_Color", norm_color);


    GetRenderer()->default_shader.SetValue("u_Model", model);


    std::vector<Vertex> vertices = {
        {glm::vec3(0.0f, 0.0f, 0.0f), glm::vec2(0.0f, 0.0f)},
        {glm::vec3(1.0f, 0.0f, 0.0f), glm::vec2(1.0f, 0.0f)},
        {glm::vec3(1.0f, 1.0f, 0.0f), glm::vec2(1.0f, 1.0f)},
        {glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2(0.0f, 1.0f)},
    };


    mesh.Update(vertices);

    mesh.Draw(GL_TRIANGLE_FAN);
}


void DrawTextureEx(Texture texture, ember::Rectangle source, ember::Rectangle dest, glm::vec2 origin, float rotation,
                   Color color) {

    if (texture.id == 0) {
        static std::once_flag log_once;
        std::call_once(log_once, []() { LOG_WARN("Texture not loaded, skipping draw!!!"); });
        return;
    }

    static Mesh mesh;

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture.id);

    GetRenderer()->default_shader.SetValue("u_Texture", 0);


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

    GetRenderer()->default_shader.SetValue("u_Color", norm_color);

    float texLeft   = (float) source.x / texture.width;
    float texRight  = (float) (source.x + source.width) / texture.width;
    float texTop    = (float) source.y / texture.height;
    float texBottom = (float) (source.y + source.height) / texture.height;

    GetRenderer()->default_shader.SetValue("u_Model", model);

    std::vector<Vertex> vertices = {
        {glm::vec3(0.0f, 0.0f, 0.0f), glm::vec2(texLeft, texTop)},
        {glm::vec3(1.0f, 0.0f, 0.0f), glm::vec2(texRight, texTop)},
        {glm::vec3(1.0f, 1.0f, 0.0f), glm::vec2(texRight, texBottom)},
        {glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2(texLeft, texBottom)},
    };


    mesh.Update(vertices);

    mesh.Draw(GL_TRIANGLE_FAN);
}

void DrawLine(glm::vec2 start, glm::vec2 end, Color color, float thickness) {


    glm::vec4 norm_color = {
        color.r / 255.0f,
        color.g / 255.0f,
        color.b / 255.0f,
        color.a / 255.0f,
    };

    glm::mat4 model = glm::mat4(1.0f);

    GetRenderer()->default_shader.SetValue("u_Color", norm_color);

    GetRenderer()->default_shader.SetValue("u_Model", model);


    std::vector<Vertex> vertices = {{glm::vec3(start, 0.0f), glm::vec2(0.0f, 0.0f)},
                                    {glm::vec3(end, 0.0f), glm::vec2(0.0f, 0.0f)}};

    static Mesh mesh(vertices);
    mesh.Update(vertices);

    glLineWidth(thickness);

    mesh.Draw(GL_LINES);
}

void BeginMode2D(Camera2D& camera) {
    GetRenderer()->default_shader.Use();

    const glm::mat4& view = camera.GetViewMatrix();

    GetRenderer()->default_shader.SetValue("u_View", view);
}

void EndMode2D() {
    glm::mat4 view = glm::mat4(1.0f);
    GetRenderer()->default_shader.SetValue("u_View", view);
}
