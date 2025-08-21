#pragma once
#include "imports.h"

/*!
 *  @brief Transform struct 2D
 *
 *  @details transform points around the screen
 *
 *  @version 1.0.0
 */
struct Transform2D {
    glm::vec2 position{0, 0};
    glm::vec2 scale{1, 1};
    float rotation = 0.0f;


    /*! @brief Get model matrix
     *
     * @version 0.2.0
     */
    [[nodiscard]] glm::mat4 get_matrix() const;

    /*! @brief Transform point into screen
     *
     * @version 1.0.0
     */
    [[nodiscard]] glm::vec3 transform_point(const glm::vec2& point) const;
};
