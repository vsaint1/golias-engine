#pragma once
#include "core/component/node.h"

struct Style {
    glm::vec4 normal_color     = {0.15f, 0.18f, 0.22f, 1.0f};
    glm::vec4 hover_color      = {0.25f, 0.32f, 0.40f, 1.0f};
    glm::vec4 pressed_color    = {0.05f, 0.65f, 0.85f, 1.0f};
    glm::vec4 disabled_color   = {0.3f, 0.3f, 0.3f, 0.5f};


    glm::vec4 text_color       = {0.95f, 0.95f, 0.95f, 1.0f};
    glm::vec4 text_placeholder_color = {0.6f, 0.6f, 0.6f, 1.0f};

    glm::vec4 border_color     = {0.0f, 0.0f, 0.0f, 1.0f};
    float border_thickness = 2.0f;

    float radius_tl         = 0.0f;
    float radius_tr         = 0.0f;
    float radius_br         = 0.0f;
    float radius_bl         = 0.0f;
    float padding              = 8.0f;
};

class Control : public Node2D {
public:

    Control();

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

    void set_style(const Style& new_style);

    void process(double delta_time) override;

    void draw(Renderer* renderer) override;

protected:
    float rotation = 0.0f;
    glm::vec2 scale = {1.0f, 1.0f};

    Style _style;
    bool _is_dirty = false;

    bool _is_disabled = false;

private:
    Rect2 rect;
    Sizei viewport = {0,0};

    void update_layout(const glm::vec2& viewport_size);
};
