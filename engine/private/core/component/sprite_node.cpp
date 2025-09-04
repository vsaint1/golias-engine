#include "core/component/sprite_node.h"

#include "core/ember_core.h"


Rect2 Sprite2D::get_region() const {
    return _source;
}

void Sprite2D::set_region(const Rect2& region, glm::vec2 size) {
    _source     = region;
    _size       = size;
    _use_region = true;
}


void Sprite2D::set_color(const Color& col) {
    _color = col;
}

void Sprite2D::ready() {

    if (_is_ready) {
        return;
    }

    _is_ready = true;

    const auto tex = _texture.lock();

    if (!tex) {
        LOG_ERROR("Failed to access texture in Sprite2D");
    }

    if (_size.x <= 0 || _size.y <= 0) {
        _size = {static_cast<float>(tex->width), static_cast<float>(tex->height)};
    }

    const auto position = get_global_transform().position;

    _dest = Rect2{position.x, position.y, _size.x, _size.y};

    _origin = {0.5f, 0.5f};

    Node2D::ready();
}

void Sprite2D::process(double delta_time) {
    Node2D::process(delta_time);
}

void Sprite2D::draw(Renderer* renderer) {
    if (!is_effective_visible() || !is_alive()) {
        return;
    }

    const auto transform = get_global_transform();

    _dest.x = transform.position.x;
    _dest.y = transform.position.y;


    if (const auto tex = _texture.lock()) {
        SDL_assert(tex->id > 0);

        if (_use_region) {
            _dest.width  = _size.x * transform.scale.x;
            _dest.height = _size.y * transform.scale.y;

            renderer->draw_texture(tex.get(), _dest, transform.rotation, _color.normalize_color(), _source, _z_index, _flip_h, _flip_v);
        } else {
            _dest.width  = tex->width * transform.scale.x;
            _dest.height = tex->height * transform.scale.y;

            renderer->draw_texture(tex.get(), _dest, transform.rotation, _color.normalize_color(), {}, _z_index, _flip_h, _flip_v);
        }
    } else {
        LOG_ERROR("Failed to access texture in Sprite2D, it might have been unloaded or not set.");
    }

    Node2D::draw(renderer);
}

void Sprite2D::input(const InputManager* input) {
    Node2D::input(input);
}

void Sprite2D::draw_hierarchy() {
    Node2D::draw_hierarchy();
}

void Sprite2D::draw_inspector() {
    Node2D::draw_inspector();

#if !defined(WITH_EDITOR)
    return;

#else
    if (ImGui::CollapsingHeader("Texture")) {
        ImGui::Text("Path: %s", (_texture.lock() ? _texture.lock()->path : "None").c_str());
        if (ImGui::Button("Select Texture")) {


        }
    }

    if (ImGui::CollapsingHeader("Sprite")) {
        ImGui::DragFloat2("Size", &_size.x, 1.0f, 0.0f, FLT_MAX, "%.2f");
        ImGui::DragFloat2("Origin", &_origin.x, 1.0f, 0.0f, FLT_MAX, "%.2f");

        float col[4] = {
            _color.r / 255.f,
            _color.g / 255.f,
            _color.b / 255.f,
            _color.a / 255.f
        };

        if (ImGui::ColorEdit4("Color", col)) {
            _color.r = static_cast<Uint8>(col[0] * 255.f);
            _color.g = static_cast<Uint8>(col[1] * 255.f);
            _color.b = static_cast<Uint8>(col[2] * 255.f);
            _color.a = static_cast<Uint8>(col[3] * 255.f);
        }
    }

    if (ImGui::CollapsingHeader("Region")) {
        ImGui::Checkbox("Use Region", &_use_region);
        if (_use_region) {
            ImGui::Text("Source Rect:");
            ImGui::InputFloat("X##source", &_source.x, 1.0f, 0.0f, "%.2f");
            ImGui::InputFloat("Y##source", &_source.y, 1.0f, 0.0f,  "%.2f");
            ImGui::InputFloat("Width##source", &_source.width, 1.0f, 0.0f,  "%.2f");
            ImGui::InputFloat("Height##source", &_source.height, 1.0f, 0.0f,  "%.2f");

            ImGui::Text("Destination Rect:");
            ImGui::InputFloat("X##dest", &_dest.x, 1.0f, 0.0f,  "%.2f");
            ImGui::InputFloat("Y##dest", &_dest.y, 1.0f, 0.0f,  "%.2f");
            ImGui::InputFloat("Width##dest", &_dest.width, 1.0f, 0.0f,  "%.2f");
            ImGui::InputFloat("Height##dest", &_dest.height, 1.0f, 0.0f,  "%.2f");
        }
    }

    if (ImGui::CollapsingHeader("Flip")) {
        ImGui::Checkbox("Horizontal", &_flip_h);
        ImGui::Checkbox("Vertical", &_flip_v);
    }

#endif

}
