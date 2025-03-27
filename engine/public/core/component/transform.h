#pragma once
#include "imports.h"

struct Transform {

    glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f);
    glm::vec3 rotation = glm::vec3(0.0f, 0.0f, 0.0f);
    glm::vec3 scale    = glm::vec3(1.0f, 1.0f, 1.0f);

    Transform() = default;
    Transform(glm::vec3 position, glm::vec3 rotation, glm::vec3 scale)
        : position(position), rotation(rotation), scale(scale) {}

    /*!
        @brief Get the model matrix 2D

        @version 0.0.2
    */
    glm::mat4 GetModelMatrix2D() const;


    /*!
        @brief Get the model matrix 3D ( TODO )

    */
    glm::mat4 GetModelMatrix() const;
};
