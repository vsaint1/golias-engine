#include "core/renderer/opengl/ember_gl.h"

#include <stb_image.h>

OpenglRenderer::OpenglRenderer()
    : vao(0), vbo(0), ebo(0), shader_program(0), _fbo_vao(0), _fbo_vbo(0), _frame_buffer_object(0), _fbo_texture(0) {
}

OpenglRenderer::~OpenglRenderer() {
}

std::shared_ptr<Texture> OpenglRenderer::get_texture(const std::string& path) {
    auto it = textures.find(path);

    if (it != textures.end()) {
        return it->second;
    }

    return load_texture(path);
}

void OpenglRenderer::_set_default_font(const std::string& font_name) {
    if (fonts.contains(font_name)) {
        current_font_name = font_name;
    } else {
        LOG_ERROR("Font not found: %s", font_name.c_str());
    }
}

void OpenglRenderer::draw_rect(Rect2 rect, float rotation, const glm::vec4& color, bool filled, int z_index) {

    BatchKey key{0, z_index, DrawCommandType::RECT};

    _add_quad_to_batch(key, rect.x, rect.y, rect.width, rect.height, 0, 0, 1, 1, color, rotation, filled);
}

void OpenglRenderer::draw_text(const std::string& text, float x, float y, float rotation, float scale, const glm::vec4& color,
                               const std::string& font_alias, int z_index, const UberShader& uber_shader, int ft_size) {
    const std::string& use_font_name = font_alias.empty() ? current_font_name : font_alias;
    if (use_font_name.empty()) {
        return;
    }

    auto font_it = fonts.find(use_font_name);
    if (font_it == fonts.end()) {
        return;
    }

    const Font& font  = font_it->second;
    const auto tokens = TextToken::parse_bbcode(text, color);
    if (tokens.empty()) {
        return;
    }

    float size_ratio    = ft_size > 0 ? static_cast<float>(ft_size) / static_cast<float>(font.font_size) : 1.0f;
    float font_scale    = scale * size_ratio;
    bool needs_rotation = (rotation != 0.0f);

    std::vector<Glyph> char_data;
    char_data.reserve(text.size());

    float xpos = x, ypos = y;
    float min_x = x, max_x = x, min_y = y, max_y = y;

    for (const auto& token : tokens) {
        const glm::vec4& tcolor = token.color;
        for (unsigned char c : token.text) {
            if (c == '\n') {
                xpos = x;
                ypos += font.font_size * font_scale;
                continue;
            }

            auto it = font.characters.find(c);
            if (it == font.characters.end()) {
                continue;
            }

            const Character& ch = it->second;
            float w             = ch.size.x * font_scale;
            float h             = ch.size.y * font_scale;
            float x0            = xpos + ch.bearing.x * font_scale;
            float y0            = ypos + (font.font_size - ch.bearing.y) * font_scale;

            min_x = std::min(min_x, x0);
            max_x = std::max(max_x, x0 + w);
            min_y = std::min(min_y, y0);
            max_y = std::max(max_y, y0 + h);

            char_data.push_back({x0, y0, w, h, &ch, &tcolor});
            xpos += (ch.advance >> 6) * font_scale;
        }
    }

    if (char_data.empty()) {
        return;
    }

    glm::vec2 text_center = needs_rotation ? glm::vec2((min_x + max_x) * 0.5f, (min_y + max_y) * 0.5f) : glm::vec2(0.0f);

    BatchKey current_key{};
    for (const auto& data : char_data) {
        glm::vec2 glyph_pos = needs_rotation ? _rotate_point({data.x0, data.y0}, text_center, rotation) : glm::vec2(data.x0, data.y0);
        current_key         = {data.ch->texture_id, z_index, DrawCommandType::TEXT, uber_shader};
        _add_quad_to_batch(current_key, glyph_pos.x, glyph_pos.y, data.w, data.h, 0, 0, 1, 1, *(data.token_color), 0.0f);
    }
}

void OpenglRenderer::draw_line(float x1, float y1, float x2, float y2, float width, float rotation, const glm::vec4& color, int z_index) {
    glm::vec2 dir    = glm::normalize(glm::vec2(x2 - x1, y2 - y1));
    glm::vec2 normal = glm::vec2(-dir.y, dir.x) * (width * 0.5f);
    glm::vec2 center = (glm::vec2(x1, y1) + glm::vec2(x2, y2)) * 0.5f;
    glm::vec2 p1     = _rotate_point({x1 - normal.x, y1 - normal.y}, center, rotation);
    glm::vec2 p2     = _rotate_point({x1 + normal.x, y1 + normal.y}, center, rotation);
    glm::vec2 p3     = _rotate_point({x2 + normal.x, y2 + normal.y}, center, rotation);
    glm::vec2 p4     = _rotate_point({x2 - normal.x, y2 - normal.y}, center, rotation);

    BatchKey key{0, z_index, DrawCommandType::LINE};
    Batch& batch  = batches[key];
    batch.mode    = DrawCommandMode::LINES;
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
    glm::vec2 p1     = _rotate_point({x1, y1}, center, rotation);
    glm::vec2 p2     = _rotate_point({x2, y2}, center, rotation);
    glm::vec2 p3     = _rotate_point({x3, y3}, center, rotation);

    BatchKey key{0, z_index, DrawCommandType::TRIANGLE, filled};
    Batch& batch  = batches[key];
    batch.z_index = z_index;
    batch.mode    = filled ? DrawCommandMode::TRIANGLES : DrawCommandMode::LINES;

    uint32_t base = batch.vertices.size();
    batch.vertices.push_back({p1, {0.0f, 0.0f}, color});
    batch.vertices.push_back({p2, {0.0f, 0.0f}, color});
    batch.vertices.push_back({p3, {0.0f, 0.0f}, color});

    if (filled) {
        batch.indices.insert(batch.indices.end(), {base, base + 1, base + 2});
    } else {
        batch.indices.insert(batch.indices.end(), {base, base + 1, base + 1, base + 2, base + 2, base});
    }
}

void OpenglRenderer::draw_circle(float center_x, float center_y, float rotation, float radius, const glm::vec4& color, bool filled,
                                 int segments, int z_index) {

    glm::vec2 center(center_x, center_y);

    BatchKey key{0, z_index, DrawCommandType::CIRCLE, filled};
    Batch& batch  = batches[key];
    batch.z_index = z_index;
    batch.mode    = filled ? DrawCommandMode::TRIANGLES : DrawCommandMode::LINES;

    uint32_t base = batch.vertices.size();

    if (filled) {
        batch.vertices.push_back({center, {0.5f, 0.5f}, color});

        for (int i = 0; i <= segments; ++i) {
            float angle = 2.0f * M_PI * i / segments;
            float x     = center_x + radius * cos(angle);
            float y     = center_y + radius * sin(angle);
            glm::vec2 p = _rotate_point({x, y}, center, rotation);
            batch.vertices.push_back({p, {0.5f + 0.5f * cos(angle), 0.5f + 0.5f * sin(angle)}, color});

            if (i > 0) {
                batch.indices.push_back(base);
                batch.indices.push_back(base + i);
                batch.indices.push_back(base + i + 1);
            }
        }
    } else {
        for (int i = 0; i < segments; ++i) {
            if (i == 0) {
                for (int j = 0; j < segments; ++j) {
                    float a     = 2.0f * M_PI * j / segments;
                    glm::vec2 p = _rotate_point({center_x + radius * cos(a), center_y + radius * sin(a)}, center, rotation);
                    batch.vertices.push_back({p, {0.0f, 0.0f}, color});
                }
            }

            batch.indices.push_back(base + i);
            batch.indices.push_back(base + ((i + 1) % segments));
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
        rotated_points.push_back(_rotate_point(p, center, rotation));
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
        for (size_t i = 0; i < rotated_points.size(); ++i) {
            constexpr float line_width = 1.0f;
            size_t next                = (i + 1) % rotated_points.size();
            draw_line(rotated_points[i].x, rotated_points[i].y, rotated_points[next].x, rotated_points[next].y, line_width, rotation, color,
                      z_index);
        }
    }
}

void OpenglRenderer::_render_command(const DrawCommand& cmd) {
}

void OpenglRenderer::setup_camera(const Camera2D& camera) {
}

void OpenglRenderer::setup_canvas(const int width, const int height) {
}

void OpenglRenderer::setup_shaders(Shader* default_shader, Shader* framebuffer_shader) {

    this->_default_shader = static_cast<OpenglShader*>(default_shader);
    this->_fbo_shader     = static_cast<OpenglShader*>(framebuffer_shader);

    LOG_INFO("Default shader setup complete");
}

void OpenglRenderer::initialize() {

    Type = Backend::OPENGL;

#pragma region DEFAULT_SHADER_SETUP
    _default_shader->bind();

    shader_program = _default_shader->get_id();


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

    projection = glm::ortho(0.0f, static_cast<float>(Viewport[0]), static_cast<float>(Viewport[1]), 0.0f, -1.0f, 1.0f);
#pragma endregion


#pragma region FRAMEBUFFER_SETUP

    constexpr float fbo_quad_vertices[] = {
        // pos        // tex_coords
        -1.0f, 1.0f,  0.0f, 1.0f, // TL
        -1.0f, -1.0f, 0.0f, 0.0f, // BL
        1.0f,  -1.0f, 1.0f, 0.0f, // BR

        -1.0f, 1.0f,  0.0f, 1.0f, // TL
        1.0f,  -1.0f, 1.0f, 0.0f, // BR
        1.0f,  1.0f,  1.0f, 1.0f // TR
    };


    glGenVertexArrays(1, &_fbo_vao);
    glGenBuffers(1, &_fbo_vbo);

    glBindVertexArray(_fbo_vao);
    glBindBuffer(GL_ARRAY_BUFFER, _fbo_vbo);

    glBufferData(GL_ARRAY_BUFFER, sizeof(fbo_quad_vertices), fbo_quad_vertices, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*) 0);

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*) (2 * sizeof(float)));

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    glGenFramebuffers(1, &_frame_buffer_object);
    glBindFramebuffer(GL_FRAMEBUFFER, _frame_buffer_object);

    glGenTextures(1, &_fbo_texture);
    glBindTexture(GL_TEXTURE_2D, _fbo_texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, Viewport[0], Viewport[1], 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, _fbo_texture, 0);


    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        LOG_ERROR("Framebuffer is not complete!");
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

#pragma endregion
}

void OpenglRenderer::resize_viewport(const int view_width, const int view_height) {
    Viewport[0] = view_width;
    Viewport[1] = view_height;
    // glViewport(0, 0, view_width, view_height);
}

void OpenglRenderer::set_context(const void* ctx) {
    context = static_cast<SDL_GLContext>(const_cast<void*>(ctx));
}

void* OpenglRenderer::get_context() {
    return (void*) context;
}

void OpenglRenderer::destroy() {
    _default_shader->destroy();
    _fbo_shader->destroy();

    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &vbo);
    glDeleteBuffers(1, &ebo);


    glDeleteVertexArrays(1, &_fbo_vao);
    glDeleteBuffers(1, &_fbo_vbo);

    glDeleteFramebuffers(1, &_frame_buffer_object);
    glDeleteTextures(1, &_fbo_texture);

    textures.clear();
    _texture_sizes.clear();

    ImGui_ImplOpenGL3_Shutdown();

    SDL_GL_DestroyContext(context);

    delete _default_shader;
    _default_shader = nullptr;

    delete _fbo_shader;
    _fbo_shader = nullptr;
}

std::shared_ptr<Texture> OpenglRenderer::load_texture(const std::string& file_path) {

    auto it = textures.find(file_path);

    if (it != textures.end()) {
        return it->second;
    }

    auto texture = std::make_shared<Texture>();

    glGenTextures(1, &texture->id);
    int nr_channels = 4;
    stbi_set_flip_vertically_on_load(false);

    const auto buffer = _load_file_into_memory(file_path);

    unsigned char* data =
        stbi_load_from_memory((unsigned char*) buffer.data(), buffer.size(), &texture->width, &texture->height, &nr_channels, 4);

    bool has_error_texture = false;

    if (!data) {
        has_error_texture = true;
        texture->width    = 128;
        texture->height   = 128;
        LOG_ERROR("Failed to load texture from file: %s", file_path.c_str());

        data = new unsigned char[texture->width * texture->height * 4];

        for (int y = 0; y < texture->height; ++y) {
            for (int x = 0; x < texture->width; ++x) {
                int i       = (y * texture->width + x) * 4;
                bool pink   = ((x / 8) % 2) == ((y / 8) % 2);
                data[i + 0] = pink ? 180 : 0;
                data[i + 1] = 0;
                data[i + 2] = pink ? 180 : 0;
                data[i + 3] = 255;
            }
        }
    }

    glBindTexture(GL_TEXTURE_2D, texture->id);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texture->width, texture->height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);


    if (!has_error_texture) {
        LOG_INFO("Loaded texture with ID: %d, path: %s", texture->id, file_path.c_str());
        LOG_INFO(" > Width %d, Height %d", texture->width, texture->height);
        LOG_INFO(" > Num. Channels %d", nr_channels);
        stbi_image_free(data);
    } else {
        LOG_WARN("Couldn't load texture from file: %s", file_path.c_str());
        free(data);
    }


    // ! HACK_FIX: Cache texture sizes by ID ( glGetTexLevelParameteriv doesn't work on opengles )
    _texture_sizes[texture->id] = glm::vec2(texture->width, texture->height);
    auto [tex, _]               = textures.emplace(file_path, std::move(texture));

    return tex->second;
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
    // font.face = face;

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

        GLuint texture_id;
        glGenTextures(1, &texture_id);
        glBindTexture(GL_TEXTURE_2D, texture_id);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, rgba_buffer.data());
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        Character character = {texture_id, glm::ivec2(w, h), glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
                               static_cast<GLuint>(face->glyph->advance.x)};

        _texture_sizes[texture_id] = glm::vec2(w, h);

        font.characters.insert(std::pair<char, Character>(c, character));
    }


    glBindTexture(GL_TEXTURE_2D, 0);
    fonts[font_alias] = font;

    LOG_INFO("Generated font atlas. Texture ID: %d, ft_size %d, alias %s, path: %s", font.characters.begin()->second.texture_id, font_size,
             font_alias.c_str(), file_path.c_str());

    if (current_font_name.empty()) {
        current_font_name = font_alias;
    }

    return true;
}


void OpenglRenderer::draw_texture(const Texture* texture, const Rect2& dest_rect, float rotation, const glm::vec4& color,
                                  const Rect2& src_rect, int z_index, const UberShader& uber_shader) {


    if (!texture) {
        return;
    }

    float u0 = 0.0f, v0 = 0.0f, u1 = 1.0f, v1 = 1.0f;

    if (!src_rect.is_zero()) {
        u0 = src_rect.x / texture->width;
        v0 = src_rect.y / texture->height;
        u1 = (src_rect.x + src_rect.width) / texture->width;
        v1 = (src_rect.y + src_rect.height) / texture->height;
    }

    BatchKey key{texture->id, z_index, DrawCommandType::TEXTURE, uber_shader};
    _add_quad_to_batch(key, dest_rect.x, dest_rect.y, dest_rect.width, dest_rect.height, u0, v0, u1, v1, color, rotation);
}

void OpenglRenderer::flush() {
    glBindFramebuffer(GL_FRAMEBUFFER, _frame_buffer_object);

    int draw_call_count = 0;

    // Sort batches by z_index
    std::vector<std::pair<BatchKey, Batch*>> sorted_batches;

    sorted_batches.clear();
    sorted_batches.reserve(batches.size());

    for (auto& [key, batch] : batches) {
        sorted_batches.emplace_back(key, &batch);
    }

    std::sort(sorted_batches.begin(), sorted_batches.end(), [](const std::pair<BatchKey, Batch*>& a, const std::pair<BatchKey, Batch*>& b) {
        return a.second->z_index < b.second->z_index;
    });

    _default_shader->bind();

    _default_shader->set_value("PROJECTION", projection);

    glBindVertexArray(vao);

    for (auto& [key, batch] : sorted_batches) {
        if (batch->vertices.empty() || batch->indices.empty()) {
            continue;
        }

        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, batch->vertices.size() * sizeof(Vertex), nullptr, GL_DYNAMIC_DRAW);
        glBufferSubData(GL_ARRAY_BUFFER, 0, batch->vertices.size() * sizeof(Vertex), batch->vertices.data());

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, batch->indices.size() * sizeof(uint32_t), nullptr, GL_DYNAMIC_DRAW);
        glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, batch->indices.size() * sizeof(uint32_t), batch->indices.data());


        const bool use_texture =
            (batch->type == DrawCommandType::TEXTURE || batch->type == DrawCommandType::TEXT) && batch->texture_id != 0;

        _default_shader->set_value("use_texture", use_texture);

        if (use_texture) {
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, batch->texture_id);
            _default_shader->set_value("TEXTURE", 0);
        }

        if (batch->type == DrawCommandType::TEXT || batch->type == DrawCommandType::TEXTURE) {
            auto tex_size = _get_texture_size(batch->texture_id);
            _set_effect_uniforms(key.uber_shader, tex_size);
        }

        if (batch->mode == DrawCommandMode::TRIANGLES) {
            glDrawElements(GL_TRIANGLES, batch->indices.size(), GL_UNSIGNED_INT, 0);
        } else {
            glDrawElements(GL_LINES, batch->indices.size(), GL_UNSIGNED_INT, 0);
        }

        if (use_texture) {
            glBindTexture(GL_TEXTURE_2D, 0);
        }

        draw_call_count++;
    }

    for (const auto& cmd : commands) {
        _render_command(cmd);
        draw_call_count++;
    }

    glBindVertexArray(0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // commands.clear();
    // batches.clear();

    // LOG_INFO("Draw Calls %d",draw_call_count);
}

void OpenglRenderer::present() {
    _render_fbo();

    SDL_GL_SwapWindow(Window);
}

void OpenglRenderer::_render_fbo() {

    const auto [x, y, width, height] = _calc_display();

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(x, y, width, height);

    _fbo_shader->bind();

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, _fbo_texture);

    glBindVertexArray(_fbo_vao);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);

    glBindTexture(GL_TEXTURE_2D, 0);
}

void OpenglRenderer::clear(glm::vec4 color) {
    commands.clear();
    batches.clear();

    glBindFramebuffer(GL_FRAMEBUFFER, _frame_buffer_object);

    if (color == glm::vec4(0.0f)) {
        color = GEngine->Config.get_environment().clear_color;
    }

    glClearColor(color.r, color.g, color.b, color.a);
    glClear(GL_COLOR_BUFFER_BIT);
}

glm::vec2 OpenglRenderer::_get_texture_size(const Uint32 texture_id) const {

    auto it = _texture_sizes.find(texture_id);

    if (it != _texture_sizes.end()) {
        return it->second;
    }

    return glm::vec2(1.0f); // fallback or assert
}

// TODO: this must be optimized, for now it is just a simple implementation
void OpenglRenderer::_set_effect_uniforms(const UberShader& uber_shader, const glm::vec2& texture_size) {
    _default_shader->set_value("use_outline", uber_shader.use_outline);
    _default_shader->set_value("outline_color", uber_shader.outline_color);
    _default_shader->set_value("outline_width", uber_shader.outline_width);

    _default_shader->set_value("use_shadow", uber_shader.use_shadow);
    _default_shader->set_value("shadow_color", uber_shader.shadow_color);
    _default_shader->set_value("shadow_offset", uber_shader.shadow_offset);

    _default_shader->set_value("texture_size", texture_size);
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
