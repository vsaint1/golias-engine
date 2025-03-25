#include "core/component/component.h"


bool Font::IsValid() const {
    return texture.id != 0 && glyphs.size() > 0;
}