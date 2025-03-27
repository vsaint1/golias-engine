#pragma once
#include "transform.h"

/*! @brief Camera2D
    - Projection `ortho`
*/
struct Camera2D {
    Transform transform;
    float zoom = 1.0f; // MAX_ZOOM = 10.0f

    Camera2D(int view_width, int view_height, float zoom = 1.0f) : width(view_width), height(view_height), zoom(zoom) {}

    glm::mat4 GetViewMatrix() const;

    /*!
        @brief Get the viewport

        @version 0.0.4
    */
    glm::vec4 GetViewport() {

        SDL_clamp(zoom, 1.0f, 10.f); 
        
        float halfWidth  = width / (2.0f * zoom);
        float halfHeight = height / (2.0f * zoom);

        return glm::vec4(transform.position.x - halfWidth, transform.position.y - halfHeight, width / zoom,
                         height / zoom);
    }

    /*!
        @brief Check if the position is visible by the camera

        @param position Object position Transform (position)
        @version 0.0.4
    */
    bool IsVisible(const glm::vec3& position) {
        glm::vec4 viewport = GetViewport();
        return position.x > viewport.x && position.x < viewport.x + viewport.z && position.y > viewport.y
            && position.y < viewport.y + viewport.w;
    }

    /*!
       @brief Resize the camera view

       @param view_width Viewport width
       @param view_height Viewport height

       @version 0.0.4
   */
    void Resize(int view_width, int view_height) {
        width  = view_width;
        height = view_height;
    }

private:
    int width = 0, height = 0;
};
