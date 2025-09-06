#include "core/ember_core.h"

glm::vec2 Renderer::calc_text_size(const std::string& text, float scale, const std::string& font_name) {
    glm::vec2 size(0.0f);

    const std::string& use_font_name = font_name.empty() ? current_font_name : font_name;
    auto it                          = fonts.find(use_font_name);
    if (it == fonts.end()) {
        return size;
    }

    const Font& font  = it->second;
    const auto tokens = TextToken::parse_bbcode(text, glm::vec4(1.0f));
    if (tokens.empty()) {
        return size;
    }

    float font_scale = scale * 1.f;

    float x_cursor     = 0.0f;
    float max_height   = 0.0f;
    float total_height = 0.0f;

    for (const auto& token : tokens) {
        const std::string& utf8 = token.text;

        for (size_t i = 0; i < utf8.size();) {
            uint32_t codepoint = 0;
            unsigned char c    = utf8[i];

            if (c < 0x80) {
                codepoint = c;
                i += 1;
            } else if ((c >> 5) == 0x6 && i + 1 < utf8.size()) {
                codepoint = ((c & 0x1F) << 6) | (utf8[i + 1] & 0x3F);
                i += 2;
            } else if ((c >> 4) == 0xE && i + 2 < utf8.size()) {
                codepoint = ((c & 0x0F) << 12) | ((utf8[i + 1] & 0x3F) << 6) | (utf8[i + 2] & 0x3F);
                i += 3;
            } else if ((c >> 3) == 0x1E && i + 3 < utf8.size()) {
                codepoint = ((c & 0x07) << 18) | ((utf8[i + 1] & 0x3F) << 12) | ((utf8[i + 2] & 0x3F) << 6) | (utf8[i + 3] & 0x3F);
                i += 4;
            } else {
                i += 1;
                continue;
            }

            if (codepoint == '\n') {
                size.x = SDL_max(size.x, x_cursor);
                total_height += max_height;
                x_cursor   = 0.0f;
                max_height = 0.0f;
                continue;
            }

            auto glyph_it = font.characters.find(codepoint);
            if (glyph_it == font.characters.end()) {
                continue;
            }

            const Character& ch = glyph_it->second;

            float w       = ch.size.x * font_scale;
            float h       = ch.size.y * font_scale;
            float x_left  = x_cursor + ch.bearing.x * font_scale;
            float x_right = x_left + w;

            x_cursor += (ch.advance >> 6) * font_scale;
            size.x     = SDL_max(size.x, x_right);
            max_height = SDL_max(max_height, h);
        }
    }

    size.y = total_height + max_height;
    return size;
}

void Renderer::set_view_matrix(const glm::mat4& view_matrix) {
    _view = view_matrix;
}

glm::mat4 Renderer::get_view_matrix() const {
    return _view;
}

glm::vec2 Renderer::rotate_point(const glm::vec2& point, const glm::vec2& center, float radians) {
    glm::vec3 p(point - center, 0.0f); // make it vec3 (z=0)
    glm::mat4 rot = glm::rotate(glm::mat4(1.0f), radians, glm::vec3(0, 0, 1));
    glm::vec3 pr  = rot * glm::vec4(p, 1.0f);
    return glm::vec2(pr) + center;
}

Recti Renderer::calc_display() {
    int window_width, window_height;
    SDL_GetWindowSize(Window, &window_width, &window_height);

    float target_aspect = static_cast<float>(Viewport[0]) / Viewport[1];
    float window_aspect = static_cast<float>(window_width) / window_height;

    int draw_x = 0, draw_y = 0;
    int draw_width = window_width, draw_height = window_height;

    if (window_aspect > target_aspect) {
        // Pillarbox
        draw_width = static_cast<int>(window_height * target_aspect);
        draw_x     = (window_width - draw_width) / 2;
    } else {
        // Letterbox
        draw_height = static_cast<int>(window_width / target_aspect);
        draw_y      = (window_height - draw_height) / 2;
    }

    return {draw_x, draw_y, draw_width, draw_height};
}

void Renderer::submit(const BatchKey& key, float x, float y, float w, float h, float u0, float v0, float u1, float v1,
                      const glm::vec4& color, float rotation, bool is_filled) {


    Batch& batch     = _batches[key];
    batch.texture_id = key.texture_id;
    batch.type       = key.type;
    batch.z_index    = key.z_index;
    uint32_t base    = batch.vertices.size();
    batch.mode       = is_filled ? DrawCommandMode::TRIANGLES : DrawCommandMode::LINES;

    const glm::mat4 transform = glm::translate(glm::mat4(1.0f), glm::vec3(x + w * 0.5f, y + h * 0.5f, 0.0f))
                              * glm::rotate(glm::mat4(1.0f), rotation, glm::vec3(0, 0, 1))
                              * glm::scale(glm::mat4(1.0f), glm::vec3(w, h, 1.0f));

    const glm::vec4 quad[4] = {
        {-0.5f, 0.5f, 0.0f, 1.0f}, // top-left
        {0.5f, 0.5f, 0.0f, 1.0f}, // top-right
        {0.5f, -0.5f, 0.0f, 1.0f}, // bottom-right
        {-0.5f, -0.5f, 0.0f, 1.0f} // bottom-left
    };

    glm::vec2 texcoords[4] = {{u0, v1}, {u1, v1}, {u1, v0}, {u0, v0}};

    for (int i = 0; i < 4; i++) {
        glm::vec4 p = transform * quad[i];
        batch.vertices.push_back({glm::vec2(p), texcoords[i], color});
    }

    // GL_TRIANGLES
    if (is_filled) {
        batch.indices.insert(batch.indices.end(), {base, base + 1, base + 2, base + 2, base + 3, base});
    } else {
        // GL_LINES
        batch.indices.insert(batch.indices.end(), {base, base + 1, base + 1, base + 2, base + 2, base + 3, base + 3, base});
    }
}
