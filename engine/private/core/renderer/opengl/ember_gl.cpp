#include "core/renderer/opengl/ember_gl.h"

#include <stb_image.h>


void OpenglRenderer::setup_shaders(Shader* default_shader, Shader* text_shader) {

    this->_default_shader = static_cast<OpenglShader*>(default_shader);
    this->_text_shader    = static_cast<OpenglShader*>(text_shader);

    LOG_INFO("Default shader setup complete");
}

void OpenglRenderer::initialize() {

#pragma region DEFAULT_BUFFER
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * MAX_VERTICES, nullptr, GL_DYNAMIC_DRAW);

    glGenBuffers(1, &EBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);

    {
        std::vector<Uint32> indices(MAX_INDICES);
        Uint32 offset = 0;
        for (int i = 0; i < MAX_QUADS; ++i) {
            indices[i * 6 + 0] = offset + 0;
            indices[i * 6 + 1] = offset + 1;
            indices[i * 6 + 2] = offset + 2;
            indices[i * 6 + 3] = offset + 2;
            indices[i * 6 + 4] = offset + 3;
            indices[i * 6 + 5] = offset + 0;
            offset += 4;
        }
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(Uint32), indices.data(), GL_STATIC_DRAW);
    }


    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*) offsetof(Vertex, Position));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*) offsetof(Vertex, Color));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*) offsetof(Vertex, UV));
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*) offsetof(Vertex, TextureIndex));


    _default_shader->bind();

    std::array<int, MAX_TEXTURE_SLOTS> samplers;

    for (int i = 0; i < MAX_TEXTURE_SLOTS; i++) {
        samplers[i] = i;
    }

    _default_shader->set_value("uTextures", samplers.data(), MAX_TEXTURE_SLOTS);

    _textureCount = 0;
    _quadCount    = 0;
#pragma endregion


#pragma region TEXT_BUFFER

    glGenVertexArrays(1, &_textVAO);
    glBindVertexArray(_textVAO);

    glGenBuffers(1, &_textVBO);
    glBindBuffer(GL_ARRAY_BUFFER, _textVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * MAX_VERTICES, nullptr, GL_DYNAMIC_DRAW);

    glGenBuffers(1, &_textEBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _textEBO);

    // {
    //     std::vector<Uint32> indices(MAX_INDICES);
    //     Uint32 offset = 0;
    //     for (int i = 0; i < MAX_QUADS; ++i) {
    //         indices[i * 6 + 0] = offset + 0;
    //         indices[i * 6 + 1] = offset + 1;
    //         indices[i * 6 + 2] = offset + 2;
    //         indices[i * 6 + 3] = offset + 2;
    //         indices[i * 6 + 4] = offset + 3;
    //         indices[i * 6 + 5] = offset + 0;
    //         offset += 4;
    //     }
    //     glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(Uint32), indices.data(), GL_STATIC_DRAW);
    // }

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*) offsetof(Vertex, Position));

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*) offsetof(Vertex, Color));

    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*) offsetof(Vertex, UV));

#pragma endregion

    glBindVertexArray(0);
}

void OpenglRenderer::resize(int view_width, int view_height) {
    Viewport[0] = view_width;
    Viewport[1] = view_height;
    glViewport(0, 0, view_width, view_height);
}

void OpenglRenderer::set_context(const void* ctx) {
    context = static_cast<SDL_GLContext>(const_cast<void*>(ctx));
}

void* OpenglRenderer::get_context() {
    return (void*) context;
}

void OpenglRenderer::destroy() {
    _default_shader->destroy();
    _text_shader->destroy();

    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);

    glDeleteVertexArrays(1, &_textVAO);
    glDeleteBuffers(1, &_textVBO);
    glDeleteBuffers(1, &_textEBO);

    glDeleteTextures(1, &_textureArrayBuffer);

    ImGui_ImplOpenGL3_Shutdown();

    SDL_GL_DestroyContext(context);

    delete _default_shader;
    _default_shader = nullptr;

    delete _text_shader;
    _text_shader = nullptr;
}


void OpenglRenderer::clear_background(const Color& color) {
    const glm::vec4 norm_color = color.normalize_color();

    glClearColor(norm_color.r, norm_color.g, norm_color.b, norm_color.a);
    glClear(GL_COLOR_BUFFER_BIT);
}


void OpenglRenderer::begin_drawing(const glm::mat4& view_projection) {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL3_NewFrame();
    ImGui::NewFrame();

    const glm::mat4 projection = glm::ortho(0.0f, static_cast<float>(GEngine->get_renderer()->Viewport[0]),
                                            static_cast<float>(GEngine->get_renderer()->Viewport[1]), 0.0f, -1.0f, 1.0f);


    // TODO: multiply w/ view_projection
    Projection = projection * view_projection;

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);

#if defined(SDL_PLATFORM_ANDROID) || defined(SDL_PLATFORM_IOS) || defined(SDL_PLATFORM_EMSCRIPTEN)
    _buffer = static_cast<Vertex*>(
        glMapBufferRange(GL_ARRAY_BUFFER, 0, MAX_VERTICES * sizeof(Vertex), GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT));
#else
    _buffer = static_cast<Vertex*>(glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY));
#endif


    _indexCount   = 0;
    _textureCount = 0;
    _quadCount    = 0;
    //
    //     glBindVertexArray(_textVAO);
    //     glBindBuffer(GL_ARRAY_BUFFER, _textVBO);
    //
    // #if defined(SDL_PLATFORM_ANDROID) || defined(SDL_PLATFORM_IOS) || defined(SDL_PLATFORM_EMSCRIPTEN)
    //     _textBuffer = static_cast<Vertex*>(
    //         glMapBufferRange(GL_ARRAY_BUFFER, 0, MAX_VERTICES * sizeof(Vertex), GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT));
    // #else
    //     _textBuffer = static_cast<Vertex*>(glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY));
    // #endif
    //
    //     _textQuadCount  = 0;
    //     _textIndexCount = 0;
}


void OpenglRenderer::flush_text() {
    if (_textIndexCount == 0) {
        return;
    }

    glBindVertexArray(_textVAO);
    glBindBuffer(GL_ARRAY_BUFFER, _textVBO);
    glUnmapBuffer(GL_ARRAY_BUFFER);


    _text_shader->bind();
    _text_shader->set_value("ViewProjection", Projection);
    _text_shader->set_value("uTexture", 0);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, _currentFontTextureID);

    glDrawElements(GL_TRIANGLES, _textIndexCount, GL_UNSIGNED_INT, nullptr);
}

void OpenglRenderer::end_drawing() {

    flush();

    // flush_text();

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    SDL_GL_SwapWindow(Window);
}

Texture OpenglRenderer::load_texture(const std::string& file_path) {
    Texture texture{};
    int w = 0, h = 0, channels = 4;

    stbi_set_flip_vertically_on_load(true);
    const auto buffer   = _load_file_into_memory(file_path);
    unsigned char* data = stbi_load_from_memory((unsigned char*) buffer.data(), buffer.size(), &w, &h, &channels, 4);

    bool error_texture = false;

    if (!data) {
        LOG_ERROR("Failed to load texture: %s", file_path.c_str());
        w    = 128;
        h    = 128;
        data = new unsigned char[w * h * 4];

        for (int y = 0; y < h; ++y) {
            for (int x = 0; x < w; ++x) {
                int i       = (y * w + x) * 4;
                bool pink   = ((x / 8) % 2) == ((y / 8) % 2);
                data[i + 0] = pink ? 180 : 0;
                data[i + 1] = 0;
                data[i + 2] = pink ? 180 : 0;
                data[i + 3] = 255;
            }
        }

        error_texture = true;
    }

#if defined(SDL_PLATFORM_ANDROID) || defined(SDL_PLATFORM_IOS) || defined(SDL_PLATFORM_EMSCRIPTEN)

    constexpr int ATLAS_WIDTH  = 1024;
    constexpr int ATLAS_HEIGHT = 1024;

    if (_textureArrayBuffer == 0) {
        glGenTextures(1, &_textureArrayBuffer);
        glBindTexture(GL_TEXTURE_2D_ARRAY, _textureArrayBuffer);
        glTexStorage3D(GL_TEXTURE_2D_ARRAY, 1, GL_RGBA8, w, h, MAX_TEXTURE_SLOTS);
        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        // glTexStorage3D(GL_TEXTURE_2D_ARRAY, 1, GL_RGBA8, ATLAS_WIDTH, ATLAS_HEIGHT, MAX_TEXTURE_SLOTS);
    }

    int layer = _textureCount + 1;


    if (layer >= MAX_TEXTURE_SLOTS) {
        LOG_WARN("Texture slot overflow, wrapping to 1");
        layer = 1;
    }

    glBindTexture(GL_TEXTURE_2D_ARRAY, _textureArrayBuffer);
    glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, layer, w, h, 1, GL_RGBA, GL_UNSIGNED_BYTE, data);

    texture.id = layer;
    _textureCount++;

#else
    unsigned int texId;
    glGenTextures(1, &texId);
    glBindTexture(GL_TEXTURE_2D, texId);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    texture.id = texId;
#endif


    if (error_texture) {
        LOG_WARN("Failed to load texture: %s", file_path.c_str());
        delete[] data;


    } else {
        LOG_INFO("Loaded texture with ID: %d, path: %s", texture.id, file_path.c_str());
        LOG_INFO(" > Width %d, Height %d", w, h);
        LOG_INFO(" > Num. Channels %d", channels);
        stbi_image_free(data);
    }

    texture.width  = w;
    texture.height = h;

    return texture;
}


// https://stackoverflow.com/questions/71185718/how-to-use-ft-render-mode-sdf-in-freetype
Font OpenglRenderer::load_font(const std::string& file_path, const int font_size) {

    Font font = {};

    const auto font_buffer = _load_file_into_memory(file_path);
    if (font_buffer.empty()) {
        LOG_ERROR("Failed to load font file into memory %s", file_path.c_str());
        return font;
    }

    FT_Library ft;
    if (FT_Init_FreeType(&ft)) {
        LOG_ERROR("Failed to initialize FreeType.");
        return font;
    }

    FT_Face face;
    if (FT_New_Memory_Face(ft, reinterpret_cast<const FT_Byte*>(font_buffer.data()), static_cast<FT_Long>(font_buffer.size()), 0, &face)) {
        LOG_ERROR("Failed to load font face.");
        return font;
    }

    FT_Set_Pixel_Sizes(face, 0, font_size);

    int atlas_w                  = 512;
    int atlas_h                  = 512;
    unsigned char* bitmap_buffer = new unsigned char[atlas_w * atlas_h]();
    int x = 0, y = 0, max_row_height = 0;

    for (char c = 32; c < 127; ++c) {
        if (FT_Load_Char(face, c, FT_LOAD_DEFAULT)) {
            LOG_WARN("Failed to load character %c", c);
            continue;
        }

        FT_Glyph glyph;
        if (FT_Get_Glyph(face->glyph, &glyph)) {
            continue;
        }

        if (FT_Glyph_To_Bitmap(&glyph, FT_RENDER_MODE_SDF, nullptr, 1)) {
            FT_Done_Glyph(glyph);
            continue;
        }

        FT_BitmapGlyph bmp_glyph = (FT_BitmapGlyph) glyph;
        FT_Bitmap& bmp           = bmp_glyph->bitmap;
        int gw                   = bmp.width;
        int gh                   = bmp.rows;

        if (x + gw >= atlas_w) {
            x = 0;
            y += max_row_height;
            max_row_height = 0;
        }

        max_row_height = SDL_max(max_row_height, gh);

        for (int row = 0; row < gh; ++row) {
            unsigned char* src = bmp.buffer + row * bmp.pitch;
            unsigned char* dst = bitmap_buffer + (x + (y + row) * atlas_w);
            SDL_memcpy(dst, src, gw);
        }


        Glyph g    = {};
        g.x0       = (float) x / atlas_w;
        g.y0       = (float) y / atlas_h;
        g.x1       = (float) (x + gw) / atlas_w;
        g.y1       = (float) (y + gh) / atlas_h;
        g.w        = gw;
        g.h        = gh;
        g.x_offset = bmp_glyph->left;
        g.y_offset = -bmp_glyph->top;
        g.advance  = face->glyph->advance.x >> 6;

        font.glyphs[c] = g;
        x += gw;

        FT_Done_Glyph(glyph);
    }

    unsigned char* rgba_buffer = new unsigned char[atlas_w * atlas_h * 4]();

    for (int i = 0; i < atlas_w * atlas_h; i++) {
        const unsigned char value = bitmap_buffer[i];
        rgba_buffer[i * 4 + 0]    = value;
        rgba_buffer[i * 4 + 1]    = value;
        rgba_buffer[i * 4 + 2]    = value;
        rgba_buffer[i * 4 + 3]    = value;
    }

    glGenTextures(1, &font.texture.id);
    glBindTexture(GL_TEXTURE_2D, font.texture.id);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, atlas_w, atlas_h, 0, GL_RGBA, GL_UNSIGNED_BYTE, rgba_buffer);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    font.ascent         = face->size->metrics.ascender >> 6;
    font.descent        = face->size->metrics.descender >> 6;
    font.line_gap       = (face->size->metrics.height >> 6) - font.ascent + font.descent;
    font.scale          = 1.0f;
    font.font_size      = font_size;
    font.texture.width  = atlas_w;
    font.texture.height = atlas_h;
    // font.face = face;

    LOG_INFO("Loaded Font. Texture ID: %d, path: %s", font.texture.id, file_path.c_str());

    delete[] bitmap_buffer;
    delete[] rgba_buffer;
    FT_Done_Face(face);
    FT_Done_FreeType(ft);

    return font;
}

// TODO: draw text batched, for now use this.
void OpenglRenderer::draw_text(const Font& font, const std::string& text, const Transform& transform, Color color, float font_size,
                               const ShaderEffect& shader_effect, float kerning) {


    if (text.empty()) {
        return;
    }

    _text_shader->bind();
    _text_shader->set_value("ViewProjection", Projection);
    // TextShader->set_value("uTexture", 0);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, font.texture.id);

    glBindVertexArray(_textVAO);

    const glm::vec4 norm_color = color.normalize_color();
    const float scale          = font_size / font.font_size;

    glm::vec2 pos = transform.position;

    std::vector<Vertex> quad_vertices = {};
    std::vector<unsigned int> indices = {};

    unsigned int index_offset = 0;

    for (char c : text) {
        if (c == '\n') {
            pos.x = transform.position.x;
            pos.y += (font.ascent - font.descent + font.line_gap) * scale;
            continue;
        }

        const auto& glyph = font.glyphs.at(c);

        float xpos = pos.x + glyph.x_offset * scale;
        float ypos = pos.y + glyph.y_offset * scale;

        float w = glyph.w * scale;
        float h = glyph.h * scale;

        glm::vec3 p0 = glm::vec3(xpos, ypos, 0);
        glm::vec3 p1 = glm::vec3(xpos + w, ypos, 0);
        glm::vec3 p2 = glm::vec3(xpos + w, ypos + h, 0);
        glm::vec3 p3 = glm::vec3(xpos, ypos + h, 0);

        constexpr float texIndex = 0.0f;

        quad_vertices.push_back({p0, norm_color, {glyph.x0, glyph.y0}, texIndex});
        quad_vertices.push_back({p1, norm_color, {glyph.x1, glyph.y0}, texIndex});
        quad_vertices.push_back({p2, norm_color, {glyph.x1, glyph.y1}, texIndex});
        quad_vertices.push_back({p3, norm_color, {glyph.x0, glyph.y1}, texIndex});

        indices.push_back(index_offset + 0);
        indices.push_back(index_offset + 1);
        indices.push_back(index_offset + 2);

        indices.push_back(index_offset + 2);
        indices.push_back(index_offset + 3);
        indices.push_back(index_offset + 0);

        index_offset += 4;

        pos.x += (static_cast<float>(glyph.advance) + kerning) * scale;
    }

    glBindBuffer(GL_ARRAY_BUFFER, _textVBO);
    glBufferData(GL_ARRAY_BUFFER, quad_vertices.size() * sizeof(Vertex), quad_vertices.data(), GL_DYNAMIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _textEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), indices.data(), GL_DYNAMIC_DRAW);

    glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);

    glBindVertexArray(0);
}

void OpenglRenderer::draw_texture(const Texture& texture, const Transform& transform, glm::vec2 size, const Color& color) {

    if (size.x <= 0 || size.y <= 0) {
        size = glm::vec2(texture.width, texture.height);
    }

    SDL_assert(texture.width > 0 && texture.height > 0);

    _submit(transform, size, color.normalize_color(), texture.id);
}


void OpenglRenderer::draw_texture_ex(const Texture& texture, const ember::Rectangle& source, const ember::Rectangle& dest, glm::vec2 origin,
                                     float rotation, const Color& color) {

    const float texIndex = _bind_texture(texture.id);


    glm::vec2 uv0 = {source.x / (float) texture.width, 1.0f - (source.y + source.height) / (float) texture.height};
    glm::vec2 uv1 = {(source.x + source.width) / static_cast<float>(texture.width), 1.0f - source.y / static_cast<float>(texture.height)};

    glm::vec2 pivot = {origin.x * dest.width, origin.y * dest.height};

    constexpr float angleCorrection = glm::radians(180.f);

    glm::mat4 model(1.0f);
    model = glm::translate(model, glm::vec3(dest.x, dest.y, 1.f));
    model = glm::translate(model, glm::vec3(pivot, 0.0f));
    model = glm::rotate(model, angleCorrection, glm::vec3(0, 0, 1));
    model = glm::rotate(model, glm::radians(rotation), glm::vec3(0, 0, 1));
    model = glm::translate(model, glm::vec3(-pivot, 0.0f));
    model = glm::scale(model, glm::vec3(dest.width, dest.height, 1.0f));

    glm::vec4 corners[4] = {
        model * glm::vec4(0, 0, 0, 1), // TL
        model * glm::vec4(1, 0, 0, 1), // TR
        model * glm::vec4(1, 1, 0, 1), // BR
        model * glm::vec4(0, 1, 0, 1), // BL
    };

    const glm::vec4 normalized_color = color.normalize_color();

    Vertex quad[4] = {
        {corners[0], normalized_color, {uv0.x, uv0.y}, texIndex},
        {corners[1], normalized_color, {uv1.x, uv0.y}, texIndex},
        {corners[2], normalized_color, {uv1.x, uv1.y}, texIndex},
        {corners[3], normalized_color, {uv0.x, uv1.y}, texIndex},
    };

    for (int i = 0; i < 4; ++i) {
        _buffer[_quadCount * 4 + i] = quad[i];
    }

    _quadCount++;
    _indexCount += 6;

    if (_indexCount >= MAX_INDICES) {
        flush();
    }
}

void OpenglRenderer::draw_line(glm::vec3 start, glm::vec3 end, const Color& color, float thickness) {
    const glm::vec2 dir = end - start;
    float len           = glm::length(dir);
    const float angle   = atan2(dir.y, dir.x);

    Transform t;
    t.position = start;
    t.scale    = {len, thickness};
    t.rotation = angle;

    _submit(t, {1, 1}, color.normalize_color(), UINT32_MAX);
}


void OpenglRenderer::draw_rect(const Transform& transform, glm::vec2 size, const Color& color, float thickness) {
    glm::vec3 p0 = {0, 0, 0};
    glm::vec3 p1 = {size.x, 0, 0};
    glm::vec3 p2 = {size.x, size.y, 0};
    glm::vec3 p3 = {0, size.y, 0};

    const glm::mat4 model = transform.get_matrix();

    p0 = glm::vec3(model * glm::vec4(p0, 1.0f));
    p1 = glm::vec3(model * glm::vec4(p1, 1.0f));
    p2 = glm::vec3(model * glm::vec4(p2, 1.0f));
    p3 = glm::vec3(model * glm::vec4(p3, 1.0f));

    draw_line(p0, p1, color, thickness);
    draw_line(p1, p2, color, thickness);
    draw_line(p2, p3, color, thickness);
    draw_line(p3, p0, color, thickness);
}


void OpenglRenderer::draw_rect_filled(const Transform& transform, glm::vec2 size, const Color& color, float thickness) {
    _submit(transform, size, color.normalize_color(), UINT32_MAX);
}

void OpenglRenderer::draw_triangle(glm::vec3 p0, glm::vec3 p1, glm::vec3 p2, const Color& color) {

    constexpr float thickness = 1.f;
    draw_line(p0, p1, color, thickness);
    draw_line(p1, p2, color, thickness);
    draw_line(p2, p0, color, thickness);
}

void OpenglRenderer::draw_triangle_filled(glm::vec3 p0, glm::vec3 p1, glm::vec3 p2, const Color& color) {

    constexpr float texIndex = 0.0f;

    const glm::vec4 normalized_color = color.normalize_color();

    _buffer[_quadCount * 4 + 0] = {p0, normalized_color, {0, 0}, texIndex};
    _buffer[_quadCount * 4 + 1] = {p1, normalized_color, {0, 0}, texIndex};
    _buffer[_quadCount * 4 + 2] = {p2, normalized_color, {0, 0}, texIndex};
    _buffer[_quadCount * 4 + 3] = {p2, normalized_color, {0, 0}, texIndex};

    _indexCount += 6;
    _quadCount++;

    if (_indexCount >= MAX_INDICES) {
        flush();
    }
}


void OpenglRenderer::draw_circle(glm::vec3 position, float radius, const Color& color, int segments) {
    for (int i = 0; i < segments; ++i) {
        float theta0 = static_cast<float>(i) / segments * 2.0f * M_PI;
        float theta1 = static_cast<float>(i + 1) / segments * 2.0f * M_PI;

        glm::vec3 p0 = position + glm::vec3(cos(theta0), sin(theta0), 0.0f) * radius;
        glm::vec3 p1 = position + glm::vec3(cos(theta1), sin(theta1), 0.0f) * radius;

        draw_line(p0, p1, color, 1.0f);
    }
}


void OpenglRenderer::draw_circle_filled(glm::vec3 position, float radius, const Color& color, int segments) {
    glm::vec3 start = position + glm::vec3(radius, 0.0f, 0.0f);
    glm::vec3 prev  = start;

    for (int i = 1; i < segments; ++i) {
        float theta = static_cast<float>(i) / segments * 2.0f * M_PI;

        glm::vec3 point = position + glm::vec3(cos(theta) * radius, sin(theta) * radius, 0.0f);

        draw_triangle_filled(position, prev, point, color);

        prev = point;
    }

    draw_triangle_filled(position, prev, start, color);
}


void OpenglRenderer::BeginCanvas() {

    // TODO: refactor this fn
    auto calculate_scale_factor = []() -> float {
        if (GEngine->get_renderer()->Viewport[0] == 0 || GEngine->get_renderer()->Viewport[1] == 0) {
            return 1.0f;
        }

        float scale_x = static_cast<float>(GEngine->get_renderer()->Viewport[0]) / static_cast<float>(GEngine->Window.width);
        float scale_y = static_cast<float>(GEngine->get_renderer()->Viewport[1]) / static_cast<float>(GEngine->Window.height);

        float scale_factor = SDL_min(scale_x, scale_y);
        return scale_factor;
    };

    const float scale_factor = calculate_scale_factor();

    const glm::mat4 projection = glm::ortho(0.0f, static_cast<float>(GEngine->Window.width) * scale_factor,
                                            static_cast<float>(GEngine->Window.height) * scale_factor, 0.0f, -1.0f, 1.0f);

    constexpr glm::mat4 view = glm::mat4(1.0f);


    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}


void OpenglRenderer::EndCanvas() {
    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
}


float OpenglRenderer::_bind_texture(Uint32 slot) {


#if defined(SDL_PLATFORM_ANDROID) || defined(SDL_PLATFORM_IOS) || defined(SDL_PLATFORM_EMSCRIPTEN)
    return static_cast<float>(slot);
#else

    for (int i = 0; i < _textureCount; ++i) {
        if (_textures[i] == slot) {
            return static_cast<float>(i + 1);
        }
    }

    if (_textureCount >= MAX_TEXTURE_SLOTS) {
        flush();
    }

    _textures[_textureCount] = slot;
    _textureCount++;
    return static_cast<float>(_textureCount);
#endif
}

void OpenglRenderer::_submit(const Transform& transform, glm::vec2 size, glm::vec4 color, Uint32 slot) {

    if (_quadCount >= MAX_QUADS) {
        flush();
    }

    bool textured        = (slot != UINT32_MAX);
    const float texIndex = textured ? _bind_texture(slot) : 0.0f;

    glm::vec2 uv00 = slot ? glm::vec2(0, 0) : glm::vec2(0);
    glm::vec2 uv11 = slot ? glm::vec2(1) : glm::vec2(0);

    glm::mat4 model = transform.get_matrix();

    glm::vec4 corners[4] = {
        model * glm::vec4(0, 0, 0, 1),
        model * glm::vec4(size.x, 0, 0, 1),
        model * glm::vec4(size.x, size.y, 0, 1),
        model * glm::vec4(0, size.y, 0, 1),
    };


    Vertex quad[4] = {{corners[0], color, {uv00.x, uv11.y}, texIndex},
                      {corners[1], color, {uv11.x, uv11.y}, texIndex},
                      {corners[2], color, {uv11.x, uv00.y}, texIndex},
                      {corners[3], color, {uv00.x, uv00.y}, texIndex}};


    for (int i = 0; i < 4; ++i) {
        _buffer[_quadCount * 4 + i] = quad[i];
    }

    _quadCount++;
    _indexCount += 6;

    if (_indexCount >= MAX_INDICES) {
        flush();
    }
}

void OpenglRenderer::flush() {

    if (_indexCount == 0) {
        return;
    }

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glUnmapBuffer(GL_ARRAY_BUFFER);

    _default_shader->bind();
    _default_shader->set_value("ViewProjection", Projection);

#if defined(SDL_PLATFORM_ANDROID) || defined(SDL_PLATFORM_IOS) || defined(SDL_PLATFORM_EMSCRIPTEN)
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D_ARRAY, _textureArrayBuffer);
#else
    for (int i = 0; i < _textureCount; ++i) {
        glActiveTexture(GL_TEXTURE0 + i);
        glBindTexture(GL_TEXTURE_2D, _textures[i]);
    }
#endif

    glDrawElements(GL_TRIANGLES, _indexCount, GL_UNSIGNED_INT, nullptr);

    _quadCount  = 0;
    _indexCount = 0;
}

void OpenglRenderer::unload_font(const Font& font) {

    unload_texture(font.texture);
}

void OpenglRenderer::unload_texture(const Texture& texture) {

    if (texture.id == 0) {
        return;
    }

    LOG_INFO("Unloading texture with ID: %d", texture.id);
    glDeleteTextures(1, &texture.id);
}
