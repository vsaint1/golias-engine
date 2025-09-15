#include "core/component/ui/control_node.h"

#include "core/ember_core.h"

void Control::update_layout(const glm::vec2& viewport_size) {
    float x0 = anchor_left * viewport_size.x + margin_left;
    float y0 = anchor_top * viewport_size.y + margin_top;
    float x1 = anchor_right * viewport_size.x + margin_right;
    float y1 = anchor_bottom * viewport_size.y + margin_bottom;

    rect.x      = x0;
    rect.y      = y0;
    rect.width  = x1 - x0;
    rect.height = y1 - y0;

    set_transform({{rect.x + pivot.x, rect.y + pivot.y}, {1.0f, 1.0f}, get_transform().rotation});
}

void Control::set_style(const Style& new_style) {
    _style = new_style;
}

void Control::process(double delta_time) {
    Node2D::process(delta_time);
    auto vp = GEngine->Config.get_viewport();

    if (viewport.width != vp.width || viewport.height != vp.height) {
        viewport  = {vp.width, vp.height};
        update_layout({vp.width, vp.height});
    }
}

void Control::draw(Renderer* renderer) {
    Node2D::draw(renderer);
}
