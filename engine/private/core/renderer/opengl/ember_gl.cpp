#include "core/renderer/opengl/ember_gl.h"

#include <stb_image.h>

OpenglRenderer::OpenglRenderer()
    : vao(0), vbo(0), ebo(0), shader_program(0), _fbo_vao(0), _fbo_vbo(0), _frame_buffer_object(0), _fbo_texture(0) {
}

OpenglRenderer::~OpenglRenderer() = default;

std::shared_ptr<Texture> OpenglRenderer::get_texture(const std::string& path) {
    auto [it, inserted] = _textures.try_emplace(path, nullptr);

    if (inserted) {
        it->second = load_texture(path);
    }

    return it->second;
}


void OpenglRenderer::draw_rect(Rect2 rect, float rotation, const glm::vec4& color, bool filled, int z_index) {

    BatchKey key{0, z_index, DrawCommandType::RECT};

    submit(key, rect.x, rect.y, rect.width, rect.height, 0, 0, 1, 1, color, rotation, filled);
}

bool OpenglRenderer::load_font(const std::string& file_path, const std::string& font_alias, int font_size) {
    Font font = {};

    FileAccess file(file_path, ModeFlags::READ);
    const auto font_buffer = file.get_file_as_bytes();
    if (font_buffer.empty()) {
        LOG_ERROR("Failed to load font file into memory: %s", file_path.c_str());
        return false;
    }

    FT_Face face = {};
    if (FT_New_Memory_Face(_ft, reinterpret_cast<const FT_Byte*>(font_buffer.data()), static_cast<FT_Long>(font_buffer.size()), 0, &face)) {
        LOG_ERROR("Failed to load font face from memory.");
        return false;
    }

    FT_Set_Pixel_Sizes(face, 0, font_size);

    font.font_path = file_path;
    font.font_size = font_size;

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    // Load character ranges: ASCII + Latin-1 + Cyrillic
    std::vector<std::pair<uint32_t, uint32_t>> ranges = {
        {0x0020, 0x007E}, // Basic Latin (printable ASCII)
        {0x00A0, 0x00FF}, // Latin-1 Supplement
        {0x0100, 0x017F}, // Latin Extended-A
        {0x0180, 0x024F}, // Latin Extended-B
        {0x0400, 0x04FF}, // Cyrillic
        {0x1E00, 0x1EFF}, // Latin Extended Additional

    };

    for (auto [start, end] : ranges) {
        for (uint32_t codepoint = start; codepoint <= end; ++codepoint) {
            if (FT_Load_Char(face, codepoint, FT_LOAD_RENDER)) {
                LOG_WARN("Failed to load char 0x%X", codepoint);
                continue;
            }

            int w = face->glyph->bitmap.width;
            int h = face->glyph->bitmap.rows;

            // Ensure minimum size for thin glyphs
            if (w == 0) {
                w = 1;
            }
            if (h == 0) {
                h = 1;
            }

            unsigned char* buffer = face->glyph->bitmap.buffer;

            std::vector<unsigned char> rgba_buffer(w * h * 4, 0); // init to transparent

            for (int y = 0; y < face->glyph->bitmap.rows; ++y) {
                for (int x = 0; x < face->glyph->bitmap.width; ++x) {
                    int src_idx = y * face->glyph->bitmap.width + x;
                    int dst_idx = y * w + x;

                    rgba_buffer[4 * dst_idx + 0] = 255;
                    rgba_buffer[4 * dst_idx + 1] = 255;
                    rgba_buffer[4 * dst_idx + 2] = 255;
                    rgba_buffer[4 * dst_idx + 3] = buffer[src_idx];
                }
            }

            GLuint texture_id;
            glGenTextures(1, &texture_id);
            glBindTexture(GL_TEXTURE_2D, texture_id);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, rgba_buffer.data());
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

            Character character = {texture_id, glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
                                   glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
                                   static_cast<GLuint>(face->glyph->advance.x)};

            _texture_sizes[texture_id] = glm::vec2(w, h);
            font.characters[codepoint] = character;
        }
    }

    FT_Done_Face(face);
    glBindTexture(GL_TEXTURE_2D, 0);

    fonts[font_alias] = font;


    LOG_INFO("Font loaded: %s, Size %d, Alias %s, (%zu glyphs)", file_path.c_str(), font_size, font_alias.c_str(),
             fonts[font_alias].characters.size());

    return true;
}

void OpenglRenderer::draw_text(const std::string& text, float x, float y, float rotation, float scale, const glm::vec4& color,
                               const std::string& font_alias, int z_index, const UberShader& uber_shader, int ft_size) {
    const std::string& use_font_name = font_alias.empty() ? "Default" : font_alias;
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

    float size_ratio = ft_size > 0 ? static_cast<float>(ft_size) / static_cast<float>(font.font_size) : 1.0f;
    float font_scale = scale * size_ratio;

    float xpos = x;
    float ypos = y;

    for (const auto& token : tokens) {
        const auto& tcolor      = token.color;
        const std::string& utf8 = token.text;

        for (size_t i = 0; i < utf8.size();) {
            uint32_t codepoint = 0;
            unsigned char c    = utf8[i];


            if (utf8_to_unicode(utf8, i, codepoint, c)) {
                continue;
            }

            if (codepoint == '\n') {
                xpos = x;
                ypos += font.font_size * font_scale;
                continue;
            }

            auto it = font.characters.find(codepoint);
            if (it == font.characters.end()) {
                continue;
            }

            const Character& ch = it->second;

            float w  = ch.size.x * font_scale;
            float h  = ch.size.y * font_scale;
            float x0 = xpos + ch.bearing.x * font_scale;
            float y0 = ypos + (font.font_size - ch.bearing.y) * font_scale;

            BatchKey key{ch.texture_id, z_index, DrawCommandType::TEXT, uber_shader};
            submit(key, x0, y0, w, h, 0, 0, 1, 1, tcolor, rotation);

            xpos += (ch.advance >> 6) * font_scale;
        }
    }
}


void OpenglRenderer::draw_line(float x1, float y1, float x2, float y2, float width, float rotation, const glm::vec4& color, int z_index) {
    glm::vec2 start(x1, y1);
    glm::vec2 end(x2, y2);

    glm::vec2 dir    = glm::normalize(end - start);
    glm::vec2 normal = glm::vec2(-dir.y, dir.x);

    glm::vec2 p1 = start - normal;
    glm::vec2 p2 = start + normal;
    glm::vec2 p3 = end + normal;
    glm::vec2 p4 = end - normal;

    BatchKey key{0, z_index, DrawCommandType::LINE};
    Batch& batch    = _batches[key];
    batch.mode      = DrawCommandMode::LINES;
    batch.z_index   = z_index;
    batch.thickness = width;
    uint32_t base   = batch.vertices.size();

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

    BatchKey key{0, z_index, DrawCommandType::TRIANGLE, filled};
    Batch& batch  = _batches[key];
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

    if (segments < 3) {
        return;
    }

    BatchKey key{0, z_index, DrawCommandType::CIRCLE, filled};
    Batch& batch  = _batches[key];
    batch.z_index = z_index;
    batch.mode    = filled ? DrawCommandMode::TRIANGLES : DrawCommandMode::LINES;

    const uint32_t base = batch.vertices.size();
    std::vector<glm::vec2> points(segments);

    const float angle_step = 2.0f * M_PI / segments;


    for (int i = 0; i < segments; ++i) {
        float a     = i * angle_step;
        glm::vec2 p = rotate_point({center_x + radius * cos(a), center_y + radius * sin(a)}, {center_x, center_y}, rotation);
        points[i]   = p;

        batch.vertices.push_back({p, {0.5f + (p.x - center_x) / (2 * radius), 0.5f + (p.y - center_y) / (2 * radius)}, color});
    }

    if (filled) {
        batch.vertices.push_back({{center_x, center_y}, {0.5f, 0.5f}, color});
        uint32_t center_index = base + points.size();

        for (int i = 0; i < segments; ++i) {
            uint32_t next = (i + 1) % segments;
            batch.indices.push_back(center_index);
            batch.indices.push_back(base + i);
            batch.indices.push_back(base + next);
        }
    } else {
        for (int i = 0; i < segments; ++i) {
            uint32_t next = (i + 1) % segments;
            batch.indices.push_back(base + i);
            batch.indices.push_back(base + next);
        }
    }
}

void OpenglRenderer::draw_rect_rounded(const Rect2& rect, float rotation, const glm::vec4& color, float radius_tl, float radius_tr,
                                       float radius_br, float radius_bl, bool filled, int z_index, int corner_segments) {

    if (radius_tl <= 0.0f && radius_tr <= 0.0f && radius_br <= 0.0f && radius_bl <= 0.0f) {
        draw_rect(rect, rotation, color, filled, z_index);
        return;
    }

    radius_tl = SDL_min(radius_tl, SDL_min(rect.width, rect.height) * 0.5f);
    radius_tr = SDL_min(radius_tr, SDL_min(rect.width, rect.height) * 0.5f);
    radius_br = SDL_min(radius_br, SDL_min(rect.width, rect.height) * 0.5f);
    radius_bl = SDL_min(radius_bl, SDL_min(rect.width, rect.height) * 0.5f);

    std::vector<glm::vec2> vertices;

    glm::vec2 topLeft     = {rect.x + radius_tl, rect.y + radius_tl};
    glm::vec2 topRight    = {rect.x + rect.width - radius_tr, rect.y + radius_tr};
    glm::vec2 bottomRight = {rect.x + rect.width - radius_br, rect.y + rect.height - radius_br};
    glm::vec2 bottomLeft  = {rect.x + radius_bl, rect.y + rect.height - radius_bl};

    auto add_corner = [&](glm::vec2 center, float start_angle, float radius) {
        for (int i = 0; i <= corner_segments; i++) {
            float t     = (float) i / (float) corner_segments;
            float angle = start_angle + t * glm::half_pi<float>(); // 90deg step
            vertices.emplace_back(center + glm::vec2(glm::cos(angle), glm::sin(angle)) * radius);
        }
    };

    add_corner(topLeft, glm::pi<float>(), radius_tl);
    add_corner(topRight, 1.5f * glm::pi<float>(), radius_tr);
    add_corner(bottomRight, 0.0f, radius_br);
    add_corner(bottomLeft, 0.5f * glm::pi<float>(), radius_bl);

    draw_polygon(vertices, rotation, color, filled, z_index);
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
        rotated_points.emplace_back(rotate_point(p, center, rotation));
    }

    if (filled) {
        BatchKey key{0, z_index, DrawCommandType::POLYGON};
        Batch& batch  = _batches[key];
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

void OpenglRenderer::setup_shaders(Shader* default_shader, Shader* framebuffer_shader) {

    this->_default_shader = dynamic_cast<OpenglShader*>(default_shader);
    this->_fbo_shader     = dynamic_cast<OpenglShader*>(framebuffer_shader);

    LOG_INFO("OpenglRenderer::setup_shaders() - Completed");
}

void OpenglRenderer::initialize() {

    LOG_INFO("OpenglRenderer::initialize()");

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

    if (FT_Init_FreeType(&_ft)) {
        LOG_ERROR("Could not init FreeType Library");
        return;
    }

    _projection = glm::ortho(0.0f, static_cast<float>(Viewport[0]), static_cast<float>(Viewport[1]), 0.0f, -1.0f, 1.0f);
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

    if (!this->load_font("fonts/Default.ttf", "Default", 16)) {
        LOG_WARN("Failed to load default font.");
    }
}

void OpenglRenderer::resize_viewport(const int view_width, const int view_height) {
    Viewport[0] = view_width;
    Viewport[1] = view_height;

    _projection = glm::ortho(0.0f, static_cast<float>(Viewport[0]), static_cast<float>(Viewport[1]), 0.0f, -1.0f, 1.0f);

    if (_frame_buffer_object == 0) {
        glGenFramebuffers(1, &_frame_buffer_object);
    }
    if (_fbo_texture == 0) {
        glGenTextures(1, &_fbo_texture);
    }

    glBindFramebuffer(GL_FRAMEBUFFER, _frame_buffer_object);

    glBindTexture(GL_TEXTURE_2D, _fbo_texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, Viewport[0], Viewport[1], 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

    //TODO: get this from project.xml
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, _fbo_texture, 0);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        LOG_ERROR("Framebuffer is not complete!");
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void OpenglRenderer::set_context(const void* ctx) {
    context = static_cast<SDL_GLContext>(const_cast<void*>(ctx));
}

void* OpenglRenderer::get_context() {
    return (void*) context;
}

void OpenglRenderer::destroy() {
    LOG_INFO("OpenglRenderer::destroy()");

    _default_shader->destroy();
    _fbo_shader->destroy();

    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &vbo);
    glDeleteBuffers(1, &ebo);


    glDeleteVertexArrays(1, &_fbo_vao);
    glDeleteBuffers(1, &_fbo_vbo);

    glDeleteFramebuffers(1, &_frame_buffer_object);
    glDeleteTextures(1, &_fbo_texture);

    _textures.clear();
    _texture_sizes.clear();

#if defined(WITH_EDITOR)
    ImGui_ImplOpenGL3_Shutdown();
#endif

    FT_Done_FreeType(_ft);

    SDL_GL_DestroyContext(context);

    delete _default_shader;
    _default_shader = nullptr;

    delete _fbo_shader;
    _fbo_shader = nullptr;
}


std::shared_ptr<Texture> OpenglRenderer::load_texture(const std::string& file_path) {

    auto it = _textures.find(file_path);

    if (it != _textures.end()) {
        return it->second;
    }

    auto texture = std::make_shared<Texture>();

    glGenTextures(1, &texture->id);
    int nr_channels = 4;
    stbi_set_flip_vertically_on_load(false);

    FileAccess file(file_path, ModeFlags::READ);

    const auto buffer = file.get_file_as_bytes();

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

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    if (!has_error_texture) {
        LOG_INFO(R"(Loaded texture with ID: %d, path: %s
         > Width %d, Height %d
         > Num. Channels %d)",
                 texture->id, file_path.c_str(), texture->width, texture->height, nr_channels);
        stbi_image_free(data);
    } else {
        LOG_WARN("Couldn't load texture from file: %s", file_path.c_str());
        free(data);
    }


    // ! HACK_FIX: Cache texture sizes by ID ( glGetTexLevelParameteriv doesn't work on opengles )
    _texture_sizes[texture->id] = glm::vec2(texture->width, texture->height);
    auto [tex, _]               = _textures.emplace(file_path, std::move(texture));

    return tex->second;
}

void OpenglRenderer::draw_texture(const Texture* texture, const Rect2& dest_rect, float rotation, const glm::vec4& color,
                                  const Rect2& src_rect, int z_index, bool flip_h, bool flip_v, const UberShader& uber_shader) {
    if (!texture) {
        return;
    }

    float u0 = 0.0f, v0 = 0.0f, u1 = 1.0f, v1 = 1.0f;
    int draw_w = texture->width;
    int draw_h = texture->height;

    if (!src_rect.is_zero()) {
        u0 = src_rect.x / texture->width;
        v0 = src_rect.y / texture->height;
        u1 = (src_rect.x + src_rect.width) / texture->width;
        v1 = (src_rect.y + src_rect.height) / texture->height;

        draw_w = static_cast<int>(src_rect.width);
        draw_h = static_cast<int>(src_rect.height);
    }

    if (flip_h) {
        std::swap(u0, u1);
    }
    if (flip_v) {
        std::swap(v0, v1);
    }

    float cx = dest_rect.x;
    float cy = dest_rect.y;
    float dw = dest_rect.width != 0 ? dest_rect.width : draw_w;
    float dh = dest_rect.height != 0 ? dest_rect.height : draw_h;

    float dx = cx - dw * 0.5f;
    float dy = cy - dh * 0.5f;

    BatchKey key{texture->id, z_index, DrawCommandType::TEXTURE, uber_shader};
    submit(key, dx, dy, dw, dh, u0, v0, u1, v1, color, rotation);
}


void OpenglRenderer::flush() {
    glBindFramebuffer(GL_FRAMEBUFFER, _frame_buffer_object);

    glViewport(0, 0, Viewport[0], Viewport[1]);
    _projection = glm::ortho(0.f, float(Viewport[0]), float(Viewport[1]), 0.f, -1.f, 1.f);

    int draw_call_count = 0;

    // Sort batches by z_index
    std::vector<std::pair<BatchKey, Batch*>> sorted_batches;
    sorted_batches.reserve(_batches.size());
    for (auto& [key, batch] : _batches) {
        sorted_batches.emplace_back(key, &batch);
    }

    std::ranges::sort(sorted_batches, [](auto& a, auto& b) { return a.second->z_index < b.second->z_index; });

    _default_shader->bind();
    _default_shader->set_value("PROJECTION", _projection);
    _default_shader->set_value("VIEW", _view);

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

        if (!key.uber_shader.is_none()) {
            auto tex_size = get_texture_size(batch->texture_id);
            set_effect_uniforms(key.uber_shader, tex_size);
        } else {
            set_effect_uniforms(UberShader::none(), glm::vec2(1, 1));
        }

        if (batch->mode == DrawCommandMode::TRIANGLES) {
            glDrawElements(GL_TRIANGLES, batch->indices.size(), GL_UNSIGNED_INT, 0);
        } else {
            glLineWidth(batch->thickness);
            glDrawElements(GL_LINES, batch->indices.size(), GL_UNSIGNED_INT, 0);
        }

        if (use_texture) {
            glBindTexture(GL_TEXTURE_2D, 0);
        }

        draw_call_count++;
    }

    glBindVertexArray(0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    _batches.clear();
}


void OpenglRenderer::present() {
#if defined(WITH_EDITOR)

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

#else
    render_fbo();
#endif

    SDL_GL_SwapWindow(Window);
}

void OpenglRenderer::render_fbo() {

    const auto [x, y, width, height] = calc_display();

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(x, y, width, height);

    _fbo_shader->bind();

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, _fbo_texture);
    _fbo_shader->set_value("TEXTURE", 0);

    glBindVertexArray(_fbo_vao);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
}

void OpenglRenderer::clear(glm::vec4 color) {
    // commands.clear();
    // batches.clear();

#if defined(WITH_EDITOR)

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL3_NewFrame();
    ImGui::NewFrame();

#endif

    glBindFramebuffer(GL_FRAMEBUFFER, _frame_buffer_object);

    if (color == glm::vec4(0.0f)) {
        color = GEngine->Config.get_environment().clear_color;
    }

    glClearColor(color.r, color.g, color.b, color.a);
    glClear(GL_COLOR_BUFFER_BIT);
}

glm::vec2 OpenglRenderer::get_texture_size(const Uint32 texture_id) const {

    auto it = _texture_sizes.find(texture_id);

    if (it != _texture_sizes.end()) {
        return it->second;
    }

    return glm::vec2(1.0f); // fallback or assert
}

// TODO: this must be optimized, for now it is just a simple implementation
void OpenglRenderer::set_effect_uniforms(const UberShader& uber_shader, const glm::vec2& texture_size) {
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
