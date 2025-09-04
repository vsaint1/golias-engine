#include "core/component/label.h"

#include "core/ember_core.h"


Label::Label(const std::string& font_path, const std::string& font_alias, const std::string& text, const int ft_size, const Color& color)
    : _font_alias(font_alias), _text(text), _color(color.normalize_color()), _font_size(ft_size) {
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
                                       _color, _font_alias, _z_index, _effect, _font_size);
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
    _color = color.normalize_color();
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

#if !defined(WITH_EDITOR)

    return;
#else

    if (ImGui::CollapsingHeader("Font", ImGuiTreeNodeFlags_DefaultOpen)) {


        const std::vector<std::string>& font_names = GEngine->get_renderer()->get_loaded_fonts_name();

        static int selected_font_index = 0;

        if (!font_names.empty()) {
            if (ImGui::BeginCombo("Alias", font_names[selected_font_index].c_str())) {
                for (int i = 0; i < font_names.size(); ++i) {
                    bool is_selected = (selected_font_index == i);
                    if (ImGui::Selectable(font_names[i].c_str(), is_selected)) {
                        selected_font_index = i;
                        _path               = font_names[i];
                    }
                    if (is_selected) {
                        ImGui::SetItemDefaultFocus();
                    }
                }
                ImGui::EndCombo();
            }
        }

        ImGui::InputInt("Override", &_font_size);
    }

    char buf[1024];
    SDL_strlcpy(buf, _text.c_str(), sizeof(buf));
    if (ImGui::InputTextMultiline("Text", buf, sizeof(buf))) {
        _text = std::string(buf);
    }

    ImGui::Checkbox("Enable BBCode", &bb_code_enabled);


    if (ImGui::ColorEdit4("Color", &_color.r)) {

    }


    if (ImGui::CollapsingHeader("Effects")) {
        static const char* effects[] = {"None", "Outline", "Shadow"};
        static int current_effect    = 0;

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
#endif
}


void Label::draw_hierarchy() {
    Node2D::draw_hierarchy();
}

Label::~Label() {
}
