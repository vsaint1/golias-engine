#pragma once
#include "imports.h"

/*!
 *  @brief Transform struct 2D
 *
 *  @details transform points around the screen
 *
 *  @version 1.0.0
 */
struct Transform {
    glm::vec2 position{0, 0};
    glm::vec2 scale{1, 1};
    float rotation = 0.0f;


    /*! @brief Get model matrix
     *
     * @version 0.2.0
     */
   [[nodiscard]] glm::mat4 get_matrix() const {
        glm::mat4 mat(1.0f);
        mat = glm::translate(mat, glm::vec3(position, 1.f));
        mat = glm::rotate(mat, rotation, glm::vec3(0, 0, 1));
        mat = glm::scale(mat, glm::vec3(scale, 1.0f));
        return mat;
    }
};
