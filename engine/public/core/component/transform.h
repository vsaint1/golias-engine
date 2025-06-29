#pragma once
#include "imports.h"


// TODO: refactor this to other file
struct Transform2D {
    glm::vec3 Position{0, 0,0};
    glm::vec2 Scale{1, 1};
    float Rotation = 0.0f;

    glm::mat4 GetMatrix() const {
        glm::mat4 mat(1.0f);
        mat = glm::translate(mat, glm::vec3(Position));
        mat = glm::rotate(mat, Rotation, glm::vec3(0, 0, 1));
        mat = glm::scale(mat, glm::vec3(Scale, 1.0f));
        return mat;
    }

};

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

