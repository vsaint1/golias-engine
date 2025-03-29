#include "core/engine_structs.h"


bool Font::IsValid() const {
    return texture.id != 0 && glyphs.size() > 0;
}


glm::vec4 Color::GetNormalizedColor() const {
    glm::vec4 norm_color = {
        r / 255.0f,
        g / 255.0f,
        b / 255.0f,
        a / 255.0f,
    };

    return norm_color;
}