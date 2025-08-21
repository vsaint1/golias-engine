#include "core/component/transform_node.h"

glm::mat4 Transform2D::get_matrix() const {
    glm::mat4 mat(1.0f);
    mat = glm::translate(mat, glm::vec3(position, 1.f));
    mat = glm::rotate(mat, rotation, glm::vec3(0, 0, 1));
    mat = glm::scale(mat, glm::vec3(scale, 1.0f));
    return mat;
}

glm::vec3 Transform2D::transform_point(const glm::vec2& point) const {
    const auto res = get_matrix() * glm::vec4(point, 1.f, 1.0f);

    return res;
}
