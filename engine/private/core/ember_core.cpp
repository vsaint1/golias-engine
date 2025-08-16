#include "core/ember_core.h"


glm::vec2 Renderer::_rotate_point(const glm::vec2& point, const glm::vec2& center, float radians) {
    glm::vec3 p(point - center, 0.0f); // make it vec3 (z=0)
    glm::mat4 rot = glm::rotate(glm::mat4(1.0f), radians, glm::vec3(0, 0, 1));
    glm::vec3 pr = rot * glm::vec4(p, 1.0f);
    return glm::vec2(pr) + center;
}

Recti Renderer::_calc_display() {
    int window_width, window_height;
    SDL_GetWindowSize(Window, &window_width, &window_height);

    float target_aspect = static_cast<float>(Viewport[0]) / Viewport[1];
    float window_aspect = static_cast<float>(window_width) / window_height;

    int draw_x = 0, draw_y = 0;
    int draw_width = window_width, draw_height = window_height;

    if (window_aspect > target_aspect) {
        // Pillarbox
        draw_width = static_cast<int>(window_height * target_aspect);
        draw_x = (window_width - draw_width) / 2;
    } else {
        // Letterbox
        draw_height = static_cast<int>(window_width / target_aspect);
        draw_y = (window_height - draw_height) / 2;
    }

    return {draw_x,draw_y, draw_width, draw_height};
}

void Renderer::_add_quad_to_batch(const BatchKey& key, float x, float y, float w, float h, float u0, float v0, float u1, float v1,
                                  const glm::vec4& color, float rotation, bool is_filled ) {


    Batch& batch     = batches[key];
    batch.texture_id = key.texture_id;
    batch.type       = key.type;
    batch.z_index    = key.z_index;
    uint32_t base    = batch.vertices.size();
    batch.mode       = is_filled ? DrawCommandMode::TRIANGLES : DrawCommandMode::LINES;

    const glm::mat4 transform = glm::translate(glm::mat4(1.0f), glm::vec3(x + w * 0.5f, y + h * 0.5f, 0.0f))
                        * glm::rotate(glm::mat4(1.0f), rotation, glm::vec3(0, 0, 1))
                        * glm::scale(glm::mat4(1.0f), glm::vec3(w, h, 1.0f));

    const glm::vec4 quad[4] = {
        {-0.5f,  0.5f, 0.0f, 1.0f}, // top-left
        { 0.5f,  0.5f, 0.0f, 1.0f}, // top-right
        { 0.5f, -0.5f, 0.0f, 1.0f}, // bottom-right
        {-0.5f, -0.5f, 0.0f, 1.0f}  // bottom-left
    };

    glm::vec2 texcoords[4] = {
        {u0, v1},
        {u1, v1},
        {u1, v0},
        {u0, v0}
    };

    for (int i = 0; i < 4; i++) {
        glm::vec4 p = transform * quad[i];
        batch.vertices.push_back({glm::vec2(p), texcoords[i], color});
    }

    // GL_TRIANGLES
    if (is_filled) {
        batch.indices.insert(batch.indices.end(), {
            base, base + 1, base + 2,
            base + 2, base + 3, base
        });
    } else {
        // GL_LINES
        batch.indices.insert(batch.indices.end(), {
            base, base + 1,
            base + 1, base + 2,
            base + 2, base + 3,
            base + 3, base
        });
    }
}
