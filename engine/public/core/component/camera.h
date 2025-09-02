#pragma once
#include "node.h"

/*!
   @brief Camera2D component
   - Projection type `orthographic`

   @param view_width Viewport width
   @param view_height Viewport height
   @param zoom Camera zoom between (0.0f - 10.0f)

   @version 0.0.5
   */
class Camera2D final : public Node2D {
public:
    Camera2D();
    explicit Camera2D(const std::string& name) : Node2D(name) {}


    void set_offset(const glm::vec2& offset);

    [[nodiscard]] glm::vec2 get_offset() const;

    void set_zoom(const glm::vec2& zoom);

    [[nodiscard]] glm::vec2 get_zoom() const;

    void set_viewport(int width, int height);

    [[nodiscard]] glm::vec2 get_viewport() const;

    void follow(Node2D* target);

    void process(double dt) override;

    [[nodiscard]] glm::mat4 get_view_matrix() const;

private:
    glm::vec2 _offset = {0.f, 0.f};
    glm::vec2 _zoom = {1.f, 1.f};
    glm::vec2 _viewport_size = {0.f, 0.f};
    glm::mat4 _view_matrix = glm::mat4(1.f);
    Node2D* _follow_target = nullptr;
};
