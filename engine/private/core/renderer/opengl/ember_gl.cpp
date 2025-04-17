#include "core/renderer/opengl/ember_gl.h"

#include <stb_image.h>

Renderer* CreateRendererGL(SDL_Window* window, int view_width, int view_height) {

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
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);


#endif

    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);

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

    if (!SDL_GL_SetSwapInterval(0)) {
        LOG_CRITICAL("Failed to set swap interval, %s", SDL_GetError());
    }

    Renderer* glRenderer    = new Renderer;
    glRenderer->viewport[0] = view_width;
    glRenderer->viewport[1] = view_height;
    glRenderer->window      = window;
    glRenderer->SetContext(glContext);

    glRenderer->default_shader = OpenglShader("shaders/default_vert.glsl", "shaders/default_frag.glsl");


    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    (void) io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
    io.IniFilename = nullptr;

    ImGui::StyleColorsDark();
    ImGui_ImplSDL3_InitForOpenGL(window, glContext);
    ImGui_ImplOpenGL3_Init(glsl_version);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glViewport(0, 0, view_width, view_height);

    SDL_ShowWindow(window);

    renderer = glRenderer;

    return renderer;
}

void ClearBackground(const Color& color) {
    const glm::vec4 norm_color = color.GetNormalizedColor();

    glClearColor(norm_color.r, norm_color.g, norm_color.b, norm_color.a);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}


void BeginDrawing() {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL3_NewFrame();
    ImGui::NewFrame();

    const glm::mat4 projection =
        glm::ortho(0.0f, (float) renderer->viewport[0], (float) renderer->viewport[1], 0.0f, -1.0f, 1.0f);

    const glm::mat4 view = glm::mat4(1.0f);

    GetRenderer()->default_shader.Bind();
    GetRenderer()->default_shader.SetValue("View", view);
    GetRenderer()->default_shader.SetValue("Projection", projection);
}

void EndDrawing() {
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    SDL_GL_SwapWindow(renderer->window);
}

Texture LoadTexture(const std::string& file_path) {
    int w, h, channels;

    stbi_set_flip_vertically_on_load(true);

    const auto buffer = LoadFileIntoMemory(file_path);

    unsigned char* data = stbi_load_from_memory((unsigned char*) buffer.data(), buffer.size(), &w, &h, &channels, 4);

    bool error_texture = false;

    if (!data) {
        LOG_ERROR("Failed to load texture with path: %s", file_path.c_str());
        w    = 128;
        h    = 128;
        data = new unsigned char[w * h * 4];

        auto fallback_error_texture = [w, h, data]() {
            for (int y = 0; y < h; y++) {
                for (int x = 0; x < w; x++) {
                    int i        = (y * w + x) * 4;
                    bool is_pink = ((x / 8) % 2) == ((y / 8) % 2);

                    data[i + 0] = is_pink ? 180 : 0;
                    data[i + 1] = is_pink ? 0 : 0;
                    data[i + 2] = is_pink ? 180 : 0;
                    data[i + 3] = 255;
                }
            }
        };

        fallback_error_texture();
        error_texture = true;
    }

    unsigned int texId;
    glGenTextures(1, &texId);
    glBindTexture(GL_TEXTURE_2D, texId);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);

    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    stbi_image_free(data);

    if (!error_texture) {
        LOG_INFO("Loaded texture with ID: %d, path: %s", texId, file_path.c_str());
        LOG_INFO(" > Width %d, Height %d", w, h);
        LOG_INFO(" > Num. Channels %d", channels);
    }

    return {texId, w, h};
}

void UnloadFont(const Font& font) {
    UnloadTexture(font.texture);
}

void UnloadTexture(const Texture& texture) {

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

    unsigned char* bitmap = new unsigned char[atlas_w * atlas_h];

    int x = 0, y = 0, max_row_height = 0;
    
    // ASCII range 32-126
    Glyph glyph;

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

    unsigned char* rgba_buffer = new unsigned char[atlas_w * atlas_h * 4];


    for (int i = 0; i < atlas_w * atlas_h; ++i) {
        unsigned char gray     = bitmap[i];
        rgba_buffer[i * 4 + 0] = gray; // R
        rgba_buffer[i * 4 + 1] = gray; // G
        rgba_buffer[i * 4 + 2] = gray; // B
        rgba_buffer[i * 4 + 3] = gray; // A
    }


    glGenTextures(1, &font.texture.id);
    glBindTexture(GL_TEXTURE_2D, font.texture.id);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, atlas_w, atlas_h, 0, GL_RGBA, GL_UNSIGNED_BYTE, rgba_buffer);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

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
    font.info           = font_info;

    delete[] bitmap;
    delete[] rgba_buffer;

    return font;
}


void DrawTextInternal(const Font& font, const std::string& text, Transform transform, Color color, float font_size,
                      float kerning) {

    if (text.empty() || !font.IsValid()) {
        return;
    }


    kerning = 0.0f; // TODO: fix kerning

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, font.texture.id);

    GetRenderer()->default_shader.SetValue("Color", color.GetNormalizedColor());

    const glm::mat4 model = transform.GetModelMatrix2D();

    GetRenderer()->default_shader.SetValue("Model", model);
    float scale_factor = (font_size > 0.0f) ? (font_size / font.font_size) : 1.0f;

    float cursor_x = 0.0f;
    float cursor_y = 0.0f;
    float start_x  = cursor_x;

    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;

    uint32_t index_offset = 0;

    for (char c : text) {

        if (c == '\n') {
            cursor_x = start_x;
            cursor_y += font.font_size * scale_factor;
            continue;
        }

        if (font.glyphs.find(c) == font.glyphs.end()) {
            continue;
        }

        const Glyph& g = font.glyphs.at(c);

        float x0 = cursor_x + g.x_offset * scale_factor;
        float y0 = cursor_y + g.y_offset * scale_factor;
        float x1 = x0 + g.w * scale_factor;
        float y1 = y0 + g.h * scale_factor;

        float u0 = g.x0;
        float v0 = g.y0;
        float u1 = g.x1;
        float v1 = g.y1;

        glm::vec3 scale = transform.scale;
        float scaledX0  = x0 * scale.x;
        float scaledY0  = y0 * scale.y;
        float scaledX1  = x1 * scale.x;
        float scaledY1  = y1 * scale.y;

        vertices.push_back({glm::vec3(x0, y0, 0.0f), glm::vec2(u0, v0)}); // TL
        vertices.push_back({glm::vec3(x1, y0, 0.0f), glm::vec2(u1, v0)}); // TR
        vertices.push_back({glm::vec3(x1, y1, 0.0f), glm::vec2(u1, v1)}); // BR
        vertices.push_back({glm::vec3(x0, y1, 0.0f), glm::vec2(u0, v1)}); // TL

        indices.push_back(index_offset + 0);
        indices.push_back(index_offset + 1);
        indices.push_back(index_offset + 2);

        indices.push_back(index_offset + 0);
        indices.push_back(index_offset + 2);
        indices.push_back(index_offset + 3);

        index_offset += 4;
        cursor_x += (g.advance + kerning) * scale_factor;
    }


    static Mesh mesh(vertices, indices);

    mesh.Update(vertices);

    mesh.Draw(GL_TRIANGLES);
}

void DrawText(const Font& font, const std::string& text, Transform transform, Color color, float font_size,
              float kerning) {

    DrawTextInternal(font, text, transform, color, font_size, kerning);
}


void DrawTexture(const Texture& texture, ember::Rectangle rect, Color color) {

    if (texture.id == 0) {
        return;
    }

    static Mesh mesh;

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture.id);


    glm::mat4 model = glm::mat4(1.0f);

    model = glm::translate(model, glm::vec3(rect.x, rect.y, 0.0f));
    model = glm::scale(model, glm::vec3(rect.width, rect.height, 1.0f));

    GetRenderer()->default_shader.SetValue("Color", color.GetNormalizedColor());

    GetRenderer()->default_shader.SetValue("Model", model);

    std::vector<Vertex> vertices = {
        {glm::vec3(0.0f, 0.0f, 0.0f), glm::vec2(0.0f, 1.0f)},
        {glm::vec3(1.0f, 0.0f, 0.0f), glm::vec2(1.0f, 1.0f)},
        {glm::vec3(1.0f, 1.0f, 0.0f), glm::vec2(1.0f, 0.0f)},
        {glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2(0.0f, 0.0f)},
    };

    mesh.Update(vertices);


    mesh.Draw(GL_TRIANGLE_FAN);
}


void DrawTextureEx(const Texture& texture, const ember::Rectangle& source, const ember::Rectangle& dest,
                   glm::vec2 origin, float rotation, const Color& color) {

    if (texture.id == 0) {
        return;
    }


    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture.id);


    GetRenderer()->default_shader.SetValue("Texture", 0);

    glm::mat4 model = glm::mat4(1.0f);
    model           = glm::translate(model, glm::vec3(dest.x, dest.y, 0.0f));
    model           = glm::translate(model, glm::vec3(origin.x, origin.y, 0.0f));
    model           = glm::rotate(model, glm::radians(rotation), glm::vec3(0.0f, 0.0f, 1.0f));
    model           = glm::translate(model, glm::vec3(-origin.x, -origin.y, 0.0f));
    model           = glm::scale(model, glm::vec3(dest.width, dest.height, 1.0f));

    GetRenderer()->default_shader.SetValue("Color", color.GetNormalizedColor());

    float texLeft   = (float) source.x / texture.width;
    float texRight  = (float) (source.x + source.width) / texture.width;
    float texTop    = (float) source.y / texture.height;
    float texBottom = (float) (source.y + source.height) / texture.height;

    GetRenderer()->default_shader.SetValue("Model", model);

    const float flipY            = -1.0f;
    std::vector<Vertex> vertices = {
        {{0.0f, 0.0f, 0.0f}, {texLeft, flipY - texTop}},
        {{1.0f, 0.0f, 0.0f}, {texRight, flipY - texTop}},
        {{1.0f, 1.0f, 0.0f}, {texRight, flipY - texBottom}},
        {{0.0f, 1.0f, 0.0f}, {texLeft, flipY - texBottom}},
    };

    std::vector<uint32_t> indices = {0, 1, 2, 0, 2, 3};

    static Mesh mesh(vertices, indices);

    mesh.Update(vertices);

    mesh.Draw();
}

void DrawLine(glm::vec2 start, glm::vec2 end, const Color& color, float thickness) {

    static Mesh mesh;

    GetRenderer()->default_shader.SetValue("Color", color.GetNormalizedColor());

    GetRenderer()->default_shader.SetValue("Model", glm::mat4(1.0f));

    std::vector<Vertex> vertices = {{glm::vec3(start, 0.0f), glm::vec2(0.0f, 0.0f)},
                                    {glm::vec3(end, 0.0f), glm::vec2(0.0f, 0.0f)}};
    mesh.Update(vertices);

    glLineWidth(thickness);

    mesh.Draw(GL_LINES);
}


void DrawRect(const ember::Rectangle& rect, const Color& color, float thickness) {
    static Mesh mesh;

    GetRenderer()->default_shader.SetValue("Color", color.GetNormalizedColor());

    GetRenderer()->default_shader.SetValue("Model", glm::mat4(1.0f));

    std::vector<Vertex> vertices = {{{rect.x, rect.y, 0.0f}, {0.0f, 0.0f}},
                                    {{rect.x + rect.width, rect.y, 0.0f}, {0.0f, 0.0f}},
                                    {{rect.x + rect.width, rect.y + rect.height, 0.0f}, {0.0f, 0.0f}},
                                    {{rect.x, rect.y + rect.height, 0.0f}, {0.0f, 0.0f}}};

    mesh.Update(vertices);

    glLineWidth(thickness);

    mesh.Draw(GL_LINE_LOOP);
}


void DrawRectFilled(const ember::Rectangle& rect, const Color& color) {
    static Mesh mesh;

    const glm::mat4 model = glm::mat4(1.0f);


    GetRenderer()->default_shader.SetValue("Color", color.GetNormalizedColor());

    GetRenderer()->default_shader.SetValue("Model", model);

    std::vector<Vertex> vertices = {
        {glm::vec3(rect.x, rect.y, 0.0f), glm::vec2(0.0f, 0.0f)},
        {glm::vec3(rect.x + rect.width, rect.y, 0.0f), glm::vec2(0.0f, 0.0f)},
        {glm::vec3(rect.x + rect.width, rect.y + rect.height, 0.0f), glm::vec2(0.0f, 0.0f)},
        {glm::vec3(rect.x, rect.y + rect.height, 0.0f), glm::vec2(0.0f, 0.0f)},
    };

    mesh.Update(vertices);

    mesh.Draw(GL_TRIANGLE_FAN);
}

void BeginMode2D(const Camera2D& camera) {

    const glm::mat4& view = camera.GetViewMatrix();

    const glm::mat4& projection = camera.GetProjectionMatrix();

    GetRenderer()->default_shader.SetValue("View", view);

    GetRenderer()->default_shader.SetValue("Projection", projection);
}

void EndMode2D() {
    const glm::mat4 view = glm::mat4(1.0f);
    GetRenderer()->default_shader.SetValue("View", view);
}


void BeginCanvas() {

    auto calculate_scale_factor = []() -> const float {
        if (renderer->viewport[0] == 0 || renderer->viewport[1] == 0) {
            return 1.0f;
        }

        float scale_x = static_cast<float>(renderer->viewport[0]) / static_cast<float>(core.Window.width);
        float scale_y = static_cast<float>(renderer->viewport[1]) / static_cast<float>(core.Window.height);

        float scale_factor = SDL_min(scale_x, scale_y);
        return scale_factor;
    };

    const float scale_factor = calculate_scale_factor();

    const glm::mat4 projection = glm::ortho(0.0f, (float) core.Window.width * scale_factor,
                                            (float) core.Window.height * scale_factor, 0.0f, -1.0f, 1.0f);

    const glm::mat4 view = glm::mat4(1.0f);

    GetRenderer()->default_shader.SetValue("View", view);
    GetRenderer()->default_shader.SetValue("Projection", projection);
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}


void EndCanvas() {
    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
}
