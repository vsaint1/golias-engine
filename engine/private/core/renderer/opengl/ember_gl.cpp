#include "core/renderer/opengl/ember_gl.h"

#include <stb_image.h>

OpenglRenderer::OpenglRenderer() {
}

OpenglRenderer::~OpenglRenderer() {
}

Texture& OpenglRenderer::get_texture(const std::string& path) {
    auto it = textures.find(path);
    if (it != textures.end()) {
        return *(it->second);
    }
    return load_texture(path);
}

void OpenglRenderer::set_current_font(const std::string& font_name) {
    if (fonts.find(font_name) != fonts.end()) {
        current_font_name = font_name;
    } else {
        LOG_ERROR("Font not found: %s", font_name.c_str());
    }
}

void OpenglRenderer::draw_rect(Rect2 rect, float rotation, const glm::vec4& color, bool filled, int z_index) {

    if (filled) {
        BatchKey key{0, z_index, DrawCommandType::RECT};
        add_quad_to_batch(key, rect.x, rect.y, rect.width, rect.height, 0, 0, 1, 1, color, rotation);
    } else {
        float line_width = 1.0f;
        glm::vec2 center(rect.x + rect.width * 0.5f, rect.y + rect.height * 0.5f);
        glm::vec2 p1 = rotate_point({rect.x, rect.y}, center, rotation);
        glm::vec2 p2 = rotate_point({rect.x + rect.width, rect.y}, center, rotation);
        glm::vec2 p3 = rotate_point({rect.x + rect.width, rect.y + rect.height}, center, rotation);
        glm::vec2 p4 = rotate_point({rect.x, rect.y + rect.height}, center, rotation);
        draw_line(p1.x, p1.y, p2.x, p2.y, line_width, color, z_index);
        draw_line(p2.x, p2.y, p3.x, p3.y, line_width, color, z_index);
        draw_line(p3.x, p3.y, p4.x, p4.y, line_width, color, z_index);
        draw_line(p4.x, p4.y, p1.x, p1.y, line_width, color, z_index);
    }
}

void OpenglRenderer::draw_text(const std::string& text, float x, float y, float rotation, float scale, const glm::vec4& color,
                               const std::string& font_alias, int z_index) {

    const std::string use_font_name = font_alias.empty() ? current_font_name : font_alias;

    if (use_font_name.empty() || !fonts.contains(use_font_name)) {
        return;
    }

    const Font& font = fonts[use_font_name];

    float xpos  = x;
    float min_x = x, max_x = x, min_y = y, max_y = y;
    for (char c : text) {
        auto it = font.characters.find(c);

        if (it == font.characters.end()) {
            continue;
        }

        const Character& ch = it->second;
        float w             = ch.size.x * scale;
        float h             = ch.size.y * scale;
        float x0            = xpos + ch.bearing.x * scale;
        float y0            = y + (font.font_size - ch.bearing.y) * scale;
        min_x               = SDL_min(min_x, x0);
        max_x               = SDL_max(max_x, x0 + w);
        min_y               = SDL_min(min_y, y0);
        max_y               = SDL_max(max_y, y0 + h);
        xpos += (ch.advance >> 6) * scale;
    }

    const glm::vec2 text_center((min_x + max_x) * 0.5f, (min_y + max_y) * 0.5f);

    xpos = x;
    for (char c : text) {
        auto it = font.characters.find(c);
        if (it == font.characters.end()) {
            continue;
        }
        const Character& ch = it->second;
        float w             = ch.size.x * scale;
        float h             = ch.size.y * scale;
        float x0            = xpos + ch.bearing.x * scale;
        float y0            = y + (font.font_size - ch.bearing.y) * scale;

        glm::vec2 glyph_pos = rotate_point({x0, y0}, text_center, rotation);

        BatchKey key{ch.texture_id, z_index, DrawCommandType::TEXT};
        add_quad_to_batch(key, glyph_pos.x, glyph_pos.y, w, h, 0, 0, 1, 1, color, 0.0f);
        xpos += (ch.advance >> 6) * scale;
    }
}

void OpenglRenderer::draw_line(float x1, float y1, float x2, float y2, float width, const glm::vec4& color, int z_index, float rotation) {
    glm::vec2 dir    = glm::normalize(glm::vec2(x2 - x1, y2 - y1));
    glm::vec2 normal = glm::vec2(-dir.y, dir.x) * (width * 0.5f);
    glm::vec2 center = (glm::vec2(x1, y1) + glm::vec2(x2, y2)) * 0.5f;
    glm::vec2 p1     = rotate_point({x1 - normal.x, y1 - normal.y}, center, rotation);
    glm::vec2 p2     = rotate_point({x1 + normal.x, y1 + normal.y}, center, rotation);
    glm::vec2 p3     = rotate_point({x2 + normal.x, y2 + normal.y}, center, rotation);
    glm::vec2 p4     = rotate_point({x2 - normal.x, y2 - normal.y}, center, rotation);

    BatchKey key{0, z_index, DrawCommandType::LINE};
    Batch& batch  = batches[key];
    batch.z_index = z_index;
    uint32_t base = batch.vertices.size();
    batch.vertices.push_back({p1, {0.0f, 0.0f}, color});
    batch.vertices.push_back({p2, {0.0f, 0.0f}, color});
    batch.vertices.push_back({p3, {0.0f, 0.0f}, color});
    batch.vertices.push_back({p4, {0.0f, 0.0f}, color});
    batch.indices.insert(batch.indices.end(), {base, base + 1, base + 2, base + 2, base + 3, base});
}


void OpenglRenderer::draw_triangle(float x1, float y1, float x2, float y2, float x3, float y3, float rotation, const glm::vec4& color,
                                   bool filled, int z_index) {
    glm::vec2 center = (glm::vec2(x1, y1) + glm::vec2(x2, y2) + glm::vec2(x3, y3)) / 3.0f;
    glm::vec2 p1     = rotate_point({x1, y1}, center, rotation);
    glm::vec2 p2     = rotate_point({x2, y2}, center, rotation);
    glm::vec2 p3     = rotate_point({x3, y3}, center, rotation);

    if (filled) {
        BatchKey key{0, z_index, DrawCommandType::TRIANGLE};
        Batch& batch  = batches[key];
        batch.z_index = z_index;
        uint32_t base = batch.vertices.size();
        batch.vertices.push_back({p1, {0.0f, 0.0f}, color});
        batch.vertices.push_back({p2, {0.0f, 0.0f}, color});
        batch.vertices.push_back({p3, {0.0f, 0.0f}, color});
        batch.indices.insert(batch.indices.end(), {base, base + 1, base + 2});
    } else {
        float line_width = 1.0f;
        draw_line(p1.x, p1.y, p2.x, p2.y, line_width, color, z_index);
        draw_line(p2.x, p2.y, p3.x, p3.y, line_width, color, z_index);
        draw_line(p3.x, p3.y, p1.x, p1.y, line_width, color, z_index);
    }
}

void OpenglRenderer::draw_circle(float center_x, float center_y, float rotation, float radius, const glm::vec4& color, bool filled,
                                 int segments, int z_index) {

    glm::vec2 center(center_x, center_y);
    if (filled) {
        BatchKey key{0, z_index, DrawCommandType::CIRCLE};
        Batch& batch  = batches[key];
        batch.z_index = z_index;
        uint32_t base = batch.vertices.size();
        batch.vertices.push_back({center, {0.5f, 0.5f}, color});
        for (int i = 0; i <= segments; ++i) {
            float angle = 2.0f * M_PI * i / segments;
            float x     = center_x + radius * cos(angle);
            float y     = center_y + radius * sin(angle);
            glm::vec2 p = rotate_point({x, y}, center, rotation);
            batch.vertices.push_back({p, {0.5f + 0.5f * cos(angle), 0.5f + 0.5f * sin(angle)}, color});
            if (i > 0) {
                batch.indices.push_back(base);
                batch.indices.push_back(base + i);
                batch.indices.push_back(base + i + 1);
            }
        }
    } else {
        float line_width = 1.0f;
        for (int i = 0; i < segments; ++i) {
            float angle1 = 2.0f * M_PI * i / segments;
            float angle2 = 2.0f * M_PI * (i + 1) / segments;
            glm::vec2 p1 = rotate_point({center_x + radius * cos(angle1), center_y + radius * sin(angle1)}, center, rotation);
            glm::vec2 p2 = rotate_point({center_x + radius * cos(angle2), center_y + radius * sin(angle2)}, center, rotation);
            draw_line(p1.x, p1.y, p2.x, p2.y, line_width, color, z_index);
        }
    }
}

void OpenglRenderer::draw_polygon(const std::vector<glm::vec2>& points, float rotation, const glm::vec4& color, bool filled, int z_index) {
    if (points.size() < 3) {
        return;
    }
    glm::vec2 center(0.0f);
    for (const auto& p : points) {
        center += p;
    }
    center /= static_cast<float>(points.size());

    std::vector<glm::vec2> rotated_points;
    for (const auto& p : points) {
        rotated_points.push_back(rotate_point(p, center, rotation));
    }

    if (filled) {
        BatchKey key{0, z_index, DrawCommandType::POLYGON};
        Batch& batch  = batches[key];
        batch.z_index = z_index;
        uint32_t base = batch.vertices.size();
        for (const auto& point : rotated_points) {
            batch.vertices.push_back({point, {0.0f, 0.0f}, color});
        }
        for (size_t i = 1; i < rotated_points.size() - 1; ++i) {
            batch.indices.push_back(base);
            batch.indices.push_back(base + i);
            batch.indices.push_back(base + i + 1);
        }
    } else {
        float line_width = 1.0f;
        for (size_t i = 0; i < rotated_points.size(); ++i) {
            size_t next = (i + 1) % rotated_points.size();
            draw_line(rotated_points[i].x, rotated_points[i].y, rotated_points[next].x, rotated_points[next].y, line_width, color, z_index);
        }
    }
}

void OpenglRenderer::render_command(const DrawCommand& cmd) {
}

void OpenglRenderer::setup_camera(const Camera2D& camera) {
}

void OpenglRenderer::setup_canvas(const int width, const int height) {
}

void OpenglRenderer::setup_shaders(Shader* default_shader, Shader* text_shader) {

    this->_default_shader = static_cast<OpenglShader*>(default_shader);
    this->_text_shader    = static_cast<OpenglShader*>(text_shader);

    LOG_INFO("Default shader setup complete");
}

void OpenglRenderer::initialize() {

    _default_shader->bind();

    shader_program = _default_shader->get_id();

    projection_uniform = glGetUniformLocation(shader_program, "projection");

    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glGenBuffers(1, &ebo);
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*) offsetof(Vertex, position));
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*) offsetof(Vertex, tex_coord));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*) offsetof(Vertex, color));
    glEnableVertexAttribArray(2);
    glBindVertexArray(0);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    if (FT_Init_FreeType(&ft)) {
        LOG_ERROR("Could not init FreeType Library");
        return;
    }

    projection = glm::ortho(0.0f, (float) Viewport[0], (float) Viewport[1], 0.0f, -1.0f, 1.0f);
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

    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &vbo);
    glDeleteBuffers(1, &ebo);


    ImGui_ImplOpenGL3_Shutdown();

    SDL_GL_DestroyContext(context);

    delete _default_shader;
    _default_shader = nullptr;

    delete _text_shader;
    _text_shader = nullptr;
}

Texture& OpenglRenderer::load_texture(const std::string& file_path) {

    auto it = textures.find(file_path);

    if (it != textures.end()) {
        return *(it->second);
    }

    auto texture = std::make_unique<Texture>();
    texture->id = 0;

    glGenTextures(1, &texture->id);
    int nr_channels = 4;
    stbi_set_flip_vertically_on_load(false);

    const auto buffer = _load_file_into_memory(file_path);
    unsigned char* data =
        stbi_load_from_memory((unsigned char*) buffer.data(), buffer.size(), &texture->width, &texture->height, &nr_channels, 4);

    if (data) {

        glBindTexture(GL_TEXTURE_2D, texture->id);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texture->width, texture->height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        stbi_image_free(data);
        textures[file_path] = std::move(texture);
        return *(textures[file_path]);
    }

    LOG_ERROR("Failed to load texture: %s", file_path.c_str());
    LOG_ERROR("STB %s", stbi_failure_reason());
    glDeleteTextures(1, &texture->id);
    static Texture dummy;
    return dummy;
}

bool OpenglRenderer::load_font(const std::string& file_path, const std::string& font_alias, int font_size) {
    Font font = {};

    const auto font_buffer = _load_file_into_memory(file_path);
    if (font_buffer.empty()) {
        LOG_ERROR("Failed to load font file into memory %s", file_path.c_str());
        return false;
    }

    FT_Face face = {};

    if (FT_New_Memory_Face(ft, reinterpret_cast<const FT_Byte*>(font_buffer.data()), static_cast<FT_Long>(font_buffer.size()), 0, &face)) {
        LOG_ERROR("Failed to load font face from memory.");
        return false;
    }


    FT_Set_Pixel_Sizes(face, 0, font_size);
    font.font_path = file_path;
    font.font_size = font_size;
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    for (unsigned char c = 0; c < 128; c++) {

        if (FT_Load_Char(face, c, FT_LOAD_RENDER)) {
            continue;
        }

        int w                 = face->glyph->bitmap.width;
        int h                 = face->glyph->bitmap.rows;
        unsigned char* buffer = face->glyph->bitmap.buffer;
        std::vector<unsigned char> rgba_buffer(w * h * 4);
        for (int i = 0; i < w * h; ++i) {
            rgba_buffer[4 * i + 0] = 255;
            rgba_buffer[4 * i + 1] = 255;
            rgba_buffer[4 * i + 2] = 255;
            rgba_buffer[4 * i + 3] = buffer[i];
        }
        GLuint texture;
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, rgba_buffer.data());
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        Character character = {texture, glm::ivec2(w, h), glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
                               static_cast<GLuint>(face->glyph->advance.x)};
        font.characters.insert(std::pair<char, Character>(c, character));
    }

    FT_Done_Face(face);

    glBindTexture(GL_TEXTURE_2D, 0);
    fonts[font_alias] = font;

    if (current_font_name.empty()) {
        current_font_name = font_alias;
    }

    return true;
}


void OpenglRenderer::draw_texture(const Texture& texture, const Rect2& dest_rect, float rotation, const glm::vec4& color,
                                  const Rect2& src_rect, int z_index) {

    if (!texture.id) {
        LOG_WARN("TEXTURE NOT FOUND - ID: %d", texture.id);
        return;
    }

    float u0 = 0.0f, v0 = 0.0f, u1 = 1.0f, v1 = 1.0f;

    if (!src_rect.is_zero()) {
        u0 = src_rect.x / texture.width;
        v0 = src_rect.y / texture.height;
        u1 = (src_rect.x + src_rect.width) / texture.width;
        v1 = (src_rect.y + src_rect.height) / texture.height;
    }

    BatchKey key{texture.id, z_index, DrawCommandType::TEXTURE};
    add_quad_to_batch(key, dest_rect.x, dest_rect.y, dest_rect.width, dest_rect.height, u0, v0, u1, v1, color, rotation);
}

void OpenglRenderer::flush() {
    int draw_call_count = 0;
    // Sort batches by z_index
    std::vector<Batch*> sorted_batches;

    for (auto& [key, batch] : batches) {
        sorted_batches.push_back(&batch);
    }

    std::sort(sorted_batches.begin(), sorted_batches.end(), [](Batch* a, Batch* b) { return a->z_index < b->z_index; });

    glUseProgram(shader_program);
    glUniformMatrix4fv(projection_uniform, 1, GL_FALSE, glm::value_ptr(projection));
    glBindVertexArray(vao);

    // Batched draw calls
    for (auto* batch : sorted_batches) {
        if (batch->vertices.empty() || batch->indices.empty()) {
            continue;
        }
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, batch->vertices.size() * sizeof(Vertex), batch->vertices.data(), GL_DYNAMIC_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, batch->indices.size() * sizeof(uint32_t), batch->indices.data(), GL_DYNAMIC_DRAW);
        bool use_texture = (batch->type == DrawCommandType::TEXTURE || batch->type == DrawCommandType::TEXT) && batch->texture_id != 0;
        glUniform1i(glGetUniformLocation(shader_program, "use_texture"), use_texture);
        if (use_texture) {
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, batch->texture_id);
            glUniform1i(glGetUniformLocation(shader_program, "TEXTURE"), 0);
        }
        glDrawElements(GL_TRIANGLES, batch->indices.size(), GL_UNSIGNED_INT, 0);
        if (use_texture) {
            glBindTexture(GL_TEXTURE_2D, 0);
        }
        draw_call_count++;
    }

    for (const auto& cmd : commands) {
        render_command(cmd);
        draw_call_count++;
    }

    glBindVertexArray(0);
    commands.clear();
    batches.clear();
    SDL_GL_SwapWindow(Window);

    // LOG_INFO("Draw Calls %d",draw_call_count);
}

void OpenglRenderer::clear(const glm::vec4& color) {
    glClearColor(color.r, color.g, color.b, color.a);
    glClear(GL_COLOR_BUFFER_BIT);
    commands.clear();
    batches.clear();
}

void OpenglRenderer::unload_font(const Font& font) {

    for (auto& character : font.characters) {
        unload_texture(character.second.texture_id);
    }

    for (auto it = fonts.begin(); it != fonts.end(); ++it) {
        if (&(it->second) == &font) {
            fonts.erase(it);
            break;
        }
    }
}

void OpenglRenderer::unload_texture(const Uint32 texture) {

    if (texture == 0) {
        return;
    }

    LOG_INFO("Unloading texture with ID: %d", texture);
    glDeleteTextures(1, &texture);
}
