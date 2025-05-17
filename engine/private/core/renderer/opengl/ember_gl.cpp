#include "core/renderer/opengl/ember_gl.h"

#include <stb_image.h>


OpenglShader* OpenglRenderer::GetDefaultShader() {
    return default_shader;
}

OpenglShader* OpenglRenderer::GetTextShader() {
    return text_shader;
}

void OpenglRenderer::Resize(int view_width, int view_height) {
    viewport[0] = view_width;
    viewport[1] = view_height;
    glViewport(0, 0, view_width, view_height);
}

void OpenglRenderer::SetContext(const void* ctx) {
    context = static_cast<SDL_GLContext>(const_cast<void*>(ctx));
}

void* OpenglRenderer::GetContext() {
    return (void*) context;
}

void OpenglRenderer::Destroy() {
    default_shader->Destroy();

    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);

    SDL_GL_DestroyContext(context);

    SDL_DestroyWindow(window);

    delete default_shader;

    delete text_shader;

    delete this;
}


void OpenglRenderer::ClearBackground(const Color& color) {
    const glm::vec4 norm_color = color.GetNormalizedColor();

    glClearColor(norm_color.r, norm_color.g, norm_color.b, norm_color.a);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}


void OpenglRenderer::BeginDrawing() {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL3_NewFrame();
    ImGui::NewFrame();

    const glm::mat4 projection = glm::ortho(0.0f, (float) GEngine->GetRenderer()->viewport[0],
                                            (float) GEngine->GetRenderer()->viewport[1], 0.0f, -1.0f, 1.0f);

    constexpr glm::mat4 view = glm::mat4(1.0f);


    Shader* shader = GEngine->GetRenderer()->GetDefaultShader();
    shader->Bind();
    shader->SetValue("View", view);
    shader->SetValue("Projection", projection);

    Shader* text_shader = GEngine->GetRenderer()->GetTextShader();
    text_shader->Bind();
    text_shader->SetValue("View", view);
    text_shader->SetValue("Projection", projection);
}

void OpenglRenderer::EndDrawing() {
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    SDL_GL_SwapWindow(GEngine->GetRenderer()->window);
}

Texture OpenglRenderer::LoadTexture(const std::string& file_path) {
    Texture texture{};
    int w = 0, h = 0, channels = 4;

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
                    data[i + 1] = 0;
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

    texture.id     = texId;
    texture.width  = w;
    texture.height = h;
    return texture;
}

// https://stackoverflow.com/questions/71185718/how-to-use-ft-render-mode-sdf-in-freetype
Font OpenglRenderer::LoadFont(const std::string& file_path, const int font_size) {

    Font font = {};

    const auto font_buffer = LoadFileIntoMemory(file_path);
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
    if (FT_New_Memory_Face(ft, reinterpret_cast<const FT_Byte*>(font_buffer.data()),
                           static_cast<FT_Long>(font_buffer.size()), 0, &face)) {
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
        const unsigned char value    = bitmap_buffer[i];
        rgba_buffer[i * 4 + 0] = value;
        rgba_buffer[i * 4 + 1] = value;
        rgba_buffer[i * 4 + 2] = value;
        rgba_buffer[i * 4 + 3] = value;
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

void OpenglRenderer::DrawText(const Font& font, const std::string& text, Transform transform, Color color,
                              float font_size, ShaderEffect shader_effect, float kerning) {


    if (text.empty() || !font.IsValid()) {
        return;
    }

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, font.texture.id);

    Shader* shader = GEngine->GetRenderer()->GetTextShader();
    shader->Bind();

    shader->SetValue("Color", color.GetNormalizedColor());

    if (shader_effect.Shadow.bEnabled) {

        glm::vec2 uv_offset = shader_effect.Shadow.pixel_offset / glm::vec2(font.texture.width, font.texture.height);
        shader->SetValue("shadow.enabled", shader_effect.Shadow.bEnabled);
        shader->SetValue("shadow.color", shader_effect.Shadow.color);
        shader->SetValue("shadow.uv_offset", uv_offset);
    }else {
        shader->SetValue("shadow.enabled", false);
    }

    if (shader_effect.Outline.bEnabled) {

        shader->SetValue("outline.enabled", shader_effect.Outline.bEnabled);
        shader->SetValue("outline.color", shader_effect.Outline.color);
        shader->SetValue("outline.thickness", shader_effect.Outline.thickness);
    } else {
        shader->SetValue("outline.enabled", false);
    }

    // TODO: add glow effect
    if (shader_effect.Glow.bEnabled) {
    }

    const glm::mat4 model = transform.GetModelMatrix2D();

    shader->SetValue("Model", model);

    float scale_factor = (font_size > 0.0f) ? (font_size / font.font_size) : 1.0f;


    float cursor_x = 0.0f;
    float cursor_y = 0.0f;
    float start_x  = cursor_x;

    std::vector<Vertex> vertices;
    std::vector<Uint32> indices;

    uint32_t index_offset = 0;

    for (char c : text) {

        if (c == '\n') {
            cursor_x = start_x;
            cursor_y += font.font_size * scale_factor;
            continue;
        }

        if (!font.glyphs.contains(c)) {
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

    glDepthMask(GL_FALSE);
    mesh.Draw(GL_TRIANGLES);
    glDepthMask(GL_TRUE);
}


void OpenglRenderer::DrawTexture(const Texture& texture, ember::Rectangle rect, Color color) {

    if (texture.id == 0) {
        return;
    }

    static Mesh mesh;

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture.id);


    glm::mat4 model = glm::mat4(1.0f);

    model = glm::translate(model, glm::vec3(rect.x, rect.y, 0.0f));
    model = glm::scale(model, glm::vec3(rect.width, rect.height, 1.0f));

    Shader* shader = GEngine->GetRenderer()->GetDefaultShader();
    shader->Bind();

    shader->SetValue("Color", color.GetNormalizedColor());

    shader->SetValue("Model", model);

    const std::vector<Vertex> vertices = {
        {glm::vec3(0.0f, 0.0f, 0.0f), glm::vec2(0.0f, 1.0f)},
        {glm::vec3(1.0f, 0.0f, 0.0f), glm::vec2(1.0f, 1.0f)},
        {glm::vec3(1.0f, 1.0f, 0.0f), glm::vec2(1.0f, 0.0f)},
        {glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2(0.0f, 0.0f)},
    };

    mesh.Update(vertices);


    mesh.Draw(GL_TRIANGLE_FAN);
}


void OpenglRenderer::DrawTextureEx(const Texture& texture, const ember::Rectangle& source, const ember::Rectangle& dest,
                                   glm::vec2 origin, float rotation, const Color& color) {

    if (texture.id == 0) {
        return;
    }


    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture.id);

    Shader* shader = GEngine->GetRenderer()->GetDefaultShader();

    glm::mat4 model = glm::mat4(1.0f);
    model           = glm::translate(model, glm::vec3(dest.x, dest.y, 0.0f));
    model           = glm::translate(model, glm::vec3(origin.x, origin.y, 0.0f));
    model           = glm::rotate(model, glm::radians(rotation), glm::vec3(0.0f, 0.0f, 1.0f));
    model           = glm::translate(model, glm::vec3(-origin.x, -origin.y, 0.0f));
    model           = glm::scale(model, glm::vec3(dest.width, dest.height, 1.0f));

    shader->SetValue("Color", color.GetNormalizedColor());

    float texLeft   = (float) source.x / texture.width;
    float texRight  = (float) (source.x + source.width) / texture.width;
    float texTop    = (float) source.y / texture.height;
    float texBottom = (float) (source.y + source.height) / texture.height;

    shader->SetValue("Model", model);

    const float flipY            = -1.0f;

    const std::vector<Vertex> vertices = {
        {{0.0f, 0.0f, 0.0f}, {texLeft, flipY - texTop}},
        {{1.0f, 0.0f, 0.0f}, {texRight, flipY - texTop}},
        {{1.0f, 1.0f, 0.0f}, {texRight, flipY - texBottom}},
        {{0.0f, 1.0f, 0.0f}, {texLeft, flipY - texBottom}},
    };

    const std::vector<uint32_t> indices = {0, 1, 2, 0, 2, 3};

    static Mesh mesh(vertices, indices);

    mesh.Update(vertices);

    mesh.Draw();
}

void OpenglRenderer::DrawLine(glm::vec2 start, glm::vec2 end, const Color& color, float thickness) {

    static Mesh mesh;

    Shader* shader = GEngine->GetRenderer()->GetDefaultShader();

    shader->SetValue("Color", color.GetNormalizedColor());

    shader->SetValue("Model", glm::mat4(1.0f));

    const std::vector<Vertex> vertices = {{glm::vec3(start, 0.0f), glm::vec2(0.0f, 0.0f)},
                                    {glm::vec3(end, 0.0f), glm::vec2(0.0f, 0.0f)}};
    mesh.Update(vertices);

    glLineWidth(thickness);

    mesh.Draw(GL_LINES);
}


void OpenglRenderer::DrawRect(const ember::Rectangle& rect, const Color& color, float thickness) {
    static Mesh mesh;

    Shader* shader = GEngine->GetRenderer()->GetDefaultShader();

    shader->SetValue("Color", color.GetNormalizedColor());

    shader->SetValue("Model", glm::mat4(1.0f));

    const std::vector<Vertex> vertices = {{{rect.x, rect.y, 0.0f}, {0.0f, 0.0f}},
                                    {{rect.x + rect.width, rect.y, 0.0f}, {0.0f, 0.0f}},
                                    {{rect.x + rect.width, rect.y + rect.height, 0.0f}, {0.0f, 0.0f}},
                                    {{rect.x, rect.y + rect.height, 0.0f}, {0.0f, 0.0f}}};

    mesh.Update(vertices);

    glLineWidth(thickness);

    mesh.Draw(GL_LINE_LOOP);
}


void OpenglRenderer::DrawRectFilled(const ember::Rectangle& rect, const Color& color) {
    static Mesh mesh;

    const glm::mat4 model = glm::mat4(1.0f);

    Shader* shader = GEngine->GetRenderer()->GetDefaultShader();

    shader->SetValue("Color", color.GetNormalizedColor());

    shader->SetValue("Model", model);

    const std::vector<Vertex> vertices = {
        {glm::vec3(rect.x, rect.y, 0.0f), glm::vec2(0.0f, 0.0f)},
        {glm::vec3(rect.x + rect.width, rect.y, 0.0f), glm::vec2(0.0f, 0.0f)},
        {glm::vec3(rect.x + rect.width, rect.y + rect.height, 0.0f), glm::vec2(0.0f, 0.0f)},
        {glm::vec3(rect.x, rect.y + rect.height, 0.0f), glm::vec2(0.0f, 0.0f)},
    };

    mesh.Update(vertices);

    mesh.Draw(GL_TRIANGLE_FAN);
}

void OpenglRenderer::BeginMode2D(const Camera2D& camera) {

    const glm::mat4& view = camera.GetViewMatrix();

    const glm::mat4& projection = camera.GetProjectionMatrix();

    Shader* shader = GEngine->GetRenderer()->GetDefaultShader();
    shader->SetValue("View", view);

    shader->SetValue("Projection", projection);

    Shader* text_shader = GEngine->GetRenderer()->GetTextShader();
    text_shader->SetValue("View", view);

    text_shader->SetValue("Projection", projection);
}

void OpenglRenderer::EndMode2D() {
    constexpr glm::mat4 view = glm::mat4(1.0f);
    Shader* shader           = GEngine->GetRenderer()->GetDefaultShader();
    shader->SetValue("View", view);

    Shader* text_shader = GEngine->GetRenderer()->GetTextShader();
    text_shader->SetValue("View", view);
}


void OpenglRenderer::BeginCanvas() {

    auto calculate_scale_factor = []() ->  float {
        if (GEngine->GetRenderer()->viewport[0] == 0 || GEngine->GetRenderer()->viewport[1] == 0) {
            return 1.0f;
        }

        float scale_x =
            static_cast<float>(GEngine->GetRenderer()->viewport[0]) / static_cast<float>(GEngine->Window.width);
        float scale_y =
            static_cast<float>(GEngine->GetRenderer()->viewport[1]) / static_cast<float>(GEngine->Window.height);

        float scale_factor = SDL_min(scale_x, scale_y);
        return scale_factor;
    };

    const float scale_factor = calculate_scale_factor();

    const glm::mat4 projection = glm::ortho(0.0f, static_cast<float>(GEngine->Window.width) * scale_factor,
                                            static_cast<float>(GEngine->Window.height) * scale_factor, 0.0f, -1.0f, 1.0f);

    constexpr glm::mat4 view = glm::mat4(1.0f);

    Shader* shader = GEngine->GetRenderer()->GetDefaultShader();
    shader->SetValue("View", view);
    shader->SetValue("Projection", projection);

    Shader* text_shader = GEngine->GetRenderer()->GetDefaultShader();
    text_shader->SetValue("View", view);
    text_shader->SetValue("Projection", projection);

    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}


void OpenglRenderer::EndCanvas() {
    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
}

void OpenglRenderer::UnloadFont(const Font& font) {

    UnloadTexture(font.texture);
}

void OpenglRenderer::UnloadTexture(const Texture& texture) {

    if (texture.id == 0) {
        return;
    }

    LOG_INFO("Unloading texture with ID: %d", texture.id);
    glDeleteTextures(1, &texture.id);
}
