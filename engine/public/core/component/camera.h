#pragma once
#include "transform.h"

/*!
       @brief Camera2D component
       - Projection type `orthographic`

       @param view_width Viewport width
       @param view_height Viewport height
       @param zoom Camera zoom between (0.0f - 10.0f)

       @version 0.0.5
   */
struct Camera2D {
    Transform transform;

    Camera2D(int view_width, int view_height, float zoom = 1.0f) : width(view_width), height(view_height), zoom(zoom) {}

    glm::mat4 GetViewMatrix() const;

    /*!
        @brief Get the viewport


        @version 0.0.4

        @return glm::vec4 Viewport

    */
    glm::vec4 GetViewport() const;

    /*!
        @brief Check if the position is visible by the camera

        @param position Object position Transform (position)

        @version 0.0.4

        @return bool

    */
    bool IsVisible(const glm::vec3& position);

    /*!
       @brief Resize the camera view

       @param view_width Viewport width
       @param view_height Viewport height

       @version 0.0.4

       @return void

   */
    void Resize(int view_width, int view_height);

    /*!
      @brief Get the projection matrix (orthographic)


      @version 0.0.4

      @return glm::mat4 Matrix

  */
    glm::mat4 GetProjectionMatrix() const;

    int GetHeight() const {
        return height;
    }

    int GetWidth() const {
        return width;
    }

    void SetZoom(float zoom) {
        this->zoom = SDL_clamp(zoom, 0.10f, 10.0f); // ENSURE CAMERA ZOOM NEVER HITS 0
    }

    float GetZoom() const {
        return this->zoom;
    }

    float zoom = 1.0f; // MAX_ZOOM = 10.0f
private:
    int width = 0, height = 0;
};
