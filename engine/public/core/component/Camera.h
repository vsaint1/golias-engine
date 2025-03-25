#pragma once
#include "Transform.h"

struct Camera2D{
    Transform transform;
    float zoom = 1.0f;


    glm::mat4 GetViewMatrix() const;
    
};