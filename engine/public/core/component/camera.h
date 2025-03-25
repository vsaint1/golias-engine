#pragma once
#include "transform.h"

/*! @brief Camera2D
    - Projection `ortho`
*/
struct Camera2D{
    Transform transform;
    float zoom = 1.0f;


    glm::mat4 GetViewMatrix() const;

};