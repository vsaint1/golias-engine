#pragma once

#include "control_node.h"


class Panel : public Control {
public:
    void ready() override;

    void draw(Renderer* renderer) override;

    void set_panel_rect(const Rect2& rect);
    [[nodiscard]] const Rect2& get_panel_rect() const;

    [[nodiscard]] bool is_flat() const;

    void set_flat(bool flat);

    ~Panel() override;

protected:
    bool _is_flat    = false;
    glm::vec4 _color = {0.15f, 0.18f, 0.22f, 1.0f};
    Rect2 _panel_rect;
};
