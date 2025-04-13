#pragma once
#include "core/component/transform.h"
#include "core/engine_structs.h"
#include "core/renderer/opengl/ember_gl.h"

class Text {
public:
    Text() = default;

    Text(const Font& font, const std::string& text, const Transform& transform, float font_size, const Color& color_ = {255,255,255,255});

    void Draw() const;

    void SetFont(const Font& font);

    void SetText(const std::string& text);

    void SetColor(const Color& color_);

    const std::string GetText() ;

    const Font& GetFont();

    const Transform& GetTransform();

    const float GetFontSize();

    void SetTransform(const Transform& transform);

    void SetFontSize(float size);

private:
    Font font;
    std::string text;
    Transform transform;
    Color color;
    float font_size;
};
