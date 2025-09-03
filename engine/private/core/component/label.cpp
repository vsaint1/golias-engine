#include "core/component/label.h"

#include "core/ember_core.h"


Label::Label(const std::string& font_path, const std::string& font_alias, const std::string& text, const int ft_size, const Color& color)
    : _font_alias(font_alias), _text(text), _color(color), _font_size(ft_size) {
    _path = font_path;

    if (!GEngine->get_renderer()->load_font(font_path, font_alias, ft_size)) {
        LOG_ERROR("Failed to load font %s", font_path.c_str());
    }
}

void Label::ready() {
    Node2D::ready();
}

void Label::process(double delta_time) {
    Node2D::process(delta_time);
}

void Label::draw(Renderer* renderer) {
    Node2D::draw(renderer);

    if (!is_effective_visible() || !is_alive()) {
        return;
    }

    const auto& transform = get_global_transform();
    GEngine->get_renderer()->draw_text(_text, transform.position.x, transform.position.y, transform.rotation, transform.scale.x,
                                       _color.normalize_color(), _font_alias, _z_index, _effect, _font_size);
}

std::string Label::get_text() const {
    return _text;
}

void Label::set_text(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);

    const int size = SDL_vsnprintf(nullptr, 0, fmt, args);
    va_end(args);

    if (size <= 0) {
        return;
    }

    std::string buffer(size, '\0');

    va_start(args, fmt);
    SDL_vsnprintf(&buffer[0], size + 1, fmt, args);
    va_end(args);

    _text = std::move(buffer);
}


void Label::set_font_size(float size) {
    _font_size = size;
}

void Label::set_text_color(const Color& color) {
    _color = color;
}

void Label::set_outline(bool enabled, float thickness, const Color& color) {
    _effect.use_outline   = enabled;
    _effect.outline_width = thickness;
    _effect.outline_color = color.normalize_color();
}

void Label::set_shadow(bool enabled, glm::vec2 offset, const Color& color) {
    _effect.use_shadow    = enabled;
    _effect.shadow_color  = color.normalize_color();
    _effect.shadow_offset = offset;
}

void Label::input(const InputManager* input) {
    Node2D::input(input);
}

void Label::draw_inspector() {
    Node2D::draw_inspector();

    ImGui::Separator();

    if (ImGui::CollapsingHeader("Font", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::InputInt("Size", &_font_size);

        ImGui::Text("Path: %s", _path.c_str());
        if (ImGui::Button("Select Font")) {

        }

        char font_buf[256];
        SDL_strlcpy(font_buf, _font_alias.c_str(), sizeof(font_buf));
        if (ImGui::InputText("Alias", font_buf, sizeof(font_buf))) {
            _font_alias = std::string(font_buf);
        }
    }

    char buf[1024];
    SDL_strlcpy(buf, _text.c_str(), sizeof(buf));
    if (ImGui::InputTextMultiline("Text", buf, sizeof(buf))) {
        _text = std::string(buf);
    }

    ImGui::Checkbox("Enable BBCode", &bb_code_enabled);

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


    if (ImGui::CollapsingHeader("Effects")) {
        static const char* effects[] = {"None", "Outline", "Shadow"};
        static int current_effect = 0;

        if (ImGui::Combo("Effect", &current_effect, effects, IM_ARRAYSIZE(effects))) {
            switch (current_effect) {
                case 0: // None
                    _effect = UberShader::none();
                    break;
                case 1: // Outline
                    _effect = UberShader::outline_only();
                    break;
                case 2: // Shadow
                    _effect = UberShader::shadow_only();
                    break;
                default:;
                }
        }

        if (current_effect == 1) {
            ImGui::ColorEdit4("Outline Color", &_effect.outline_color[0]);
            ImGui::DragFloat("Outline Width", &_effect.outline_width, 0.1f, 0.0f, 10.0f);
        }

        if (current_effect == 2) {
            ImGui::ColorEdit4("Shadow Color", &_effect.shadow_color[0]);
            ImGui::DragFloat2("Shadow Offset", &_effect.shadow_offset.x, 0.5f);
        }
    }
}


void Label::draw_hierarchy() {
    Node2D::draw_hierarchy();
}

Label::~Label() {
}
