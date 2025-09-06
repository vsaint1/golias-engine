#pragma once
#include "core/component/node.h"

struct Style {
    glm::vec4 normal_color     = {0.3f, 0.3f, 0.3f, 1.0f};
    glm::vec4 hover_color      = {0.4f, 0.4f, 0.4f, 1.0f};
    glm::vec4 pressed_color    = {0.2f, 0.6f, 0.8f, 1.0f};
    glm::vec4 text_color       = {1.0f, 1.0f, 1.0f, 1.0f};
    float radius_tl         = 0.0f;
    float radius_tr         = 0.0f;
    float radius_br         = 0.0f;
    float radius_bl         = 0.0f;
    float padding              = 8.0f;
};

class Control : public Node2D {
public:
    // Anchors (0.0 = left/top, 1.0 = right/bottom)
    float anchor_left   = 0.0f;
    float anchor_top    = 0.0f;
    float anchor_right  = 0.0f;
    float anchor_bottom = 0.0f;

    // Margins (in pixels)
    float margin_left   = 0.0f;
    float margin_top    = 0.0f;
    float margin_right  = 0.0f;
    float margin_bottom = 0.0f;

    glm::vec2 pivot = {0.0f, 0.0f};

    void process(double delta_time) override;

    void draw(Renderer* renderer) override;

protected:
    Rect2 rect;
    float rotation = 0.0f;
    Sizei viewport = {0,0};
    glm::vec2 scale = {1.0f, 1.0f};
    Style style;
private:

    void update_layout(const glm::vec2& viewport_size);
};
