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

    Camera2D(int view_width, int view_height, float zoom = 1.0f) : _width(view_width), _height(view_height), zoom(zoom) {}

    glm::mat4 get_view_matrix() const;

    /*!
        @brief Get the viewport


        @version 0.0.4

        @return glm::vec4 Viewport

    */
    glm::vec4 get_viewport() const;

    /*!
        @brief Check if the position is visible by the camera

        @param position Object position Transform (position)

        @version 0.0.4

        @return bool

    */
    bool is_visible(const glm::vec3& position);

    /*!
       @brief Resize the camera view

       @param view_width Viewport width
       @param view_height Viewport height

       @version 0.0.4

       @return void

   */
    void resize(int view_width, int view_height);

    /*!
      @brief Get the projection matrix (orthographic)


      @version 0.0.4

      @return glm::mat4 Matrix

  */
    glm::mat4 get_projection_matrix() const;

    int get_height() const {
        return _height;
    }

    int get_width() const {
        return _width;
    }

    void set_zoom(float zoom) {
        this->zoom = SDL_clamp(zoom, 0.10f, 10.0f); // ENSURE CAMERA ZOOM NEVER HITS 0
    }

    float get_zoom() const {
        return this->zoom;
    }

    float zoom = 1.0f; // MAX_ZOOM = 10.0f
private:
    int _width = 0, _height = 0;
};
