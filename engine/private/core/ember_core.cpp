#include "core/ember_core.h"


glm::vec2 Renderer::_rotate_point(const glm::vec2& point, const glm::vec2& center, float rotation) {
    float s           = sin(rotation);
    float c           = cos(rotation);
    glm::vec2 rel     = point - center;
    glm::vec2 rot_rel = glm::vec2(rel.x * c - rel.y * s, rel.x * s + rel.y * c);
    return center + rot_rel;
}


void Renderer::_add_quad_to_batch(const BatchKey& key, float x, float y, float w, float h, float u0, float v0, float u1, float v1,
                                  const glm::vec4& color, float rotation, bool is_filled ) {


    Batch& batch     = batches[key];
    batch.texture_id = key.texture_id;
    batch.type       = key.type;
    batch.z_index    = key.z_index;
    uint32_t base    = batch.vertices.size();
    batch.mode       = is_filled ? DrawCommandMode::TRIANGLES : DrawCommandMode::LINES;

    const glm::vec2 center = glm::vec2(x + w * 0.5f, y + h * 0.5f);

    glm::vec2 p0 = _rotate_point({x, y + h}, center, rotation); // top-left
    glm::vec2 p1 = _rotate_point({x + w, y + h}, center, rotation); // top-right
    glm::vec2 p2 = _rotate_point({x + w, y}, center, rotation); // bottom-right
    glm::vec2 p3 = _rotate_point({x, y}, center, rotation); // bottom-left

    batch.vertices.push_back({p0, {u0, v1}, color});
    batch.vertices.push_back({p1, {u1, v1}, color});
    batch.vertices.push_back({p2, {u1, v0}, color});
    batch.vertices.push_back({p3, {u0, v0}, color});

    if (is_filled) {
        // Two triangles
        batch.indices.insert(batch.indices.end(), {base, base + 1, base + 2, base + 2, base + 3, base});
    } else {
        // Outline (4 lines)
        batch.indices.insert(batch.indices.end(), {base, base + 1, base + 1, base + 2, base + 2, base + 3, base + 3, base});
    }
}
