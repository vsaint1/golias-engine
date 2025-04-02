#pragma once
#include "core/component/transform.h"
#include "core/engine_structs.h"
#include "core/renderer/ember_gl.h"

class Text {
public:
    Text() = default;

    Text(Font& font, const std::string& text, Transform transform, float font_size);

    void Draw();

    void SetFont(Font& font);

    void SetText(const std::string& text);

    void SetColor(Color color);

    std::string GetText();

    Font& GetFont();

    Transform& GetTransform();

    void SetTransform(Transform transform);

    void SetFontSize(float font_size){
        this->font_size = font_size;
    }

    int GetTextWidth();
    

private:
    Font font;
    std::string text;
    Transform transform;
    Color color;
    float font_size;
};
