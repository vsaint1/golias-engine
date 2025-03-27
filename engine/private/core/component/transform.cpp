#include "core/component/transform.h"

glm::mat4 Transform::GetModelMatrix() const {
    return glm::mat4(1.0f);
}

glm::mat4 Transform::GetModelMatrix2D() const {

    glm::mat4 model = glm::mat4(1.0f);
    model           = glm::translate(model, position);
    model           = glm::rotate(model, glm::radians(rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));
    model           = glm::scale(model, scale);
    return model;
}
