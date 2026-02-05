#include "scene/ui/inputfield_component.h"

#include "core/engine.h"
#include "core/input/input_manager.h"
#include "font/font.h"
#include "scene/game_object.h"
#include "scene/ui/canvas_component.h"
#include "scene/ui/rect_transform_component.h"

namespace golias {

    void InputFieldWidgetComponent::SetText(const std::string& txt) {
        if (characterLimit > 0 && static_cast<int>(txt.length()) > characterLimit) {
            text = txt.substr(0, characterLimit);
        } else {
            text = txt;
        }
        
        caretPosition = static_cast<int>(text.length());
        selectionStart = selectionEnd = caretPosition;
        
        if (OnValueChanged) {
            OnValueChanged(text);
        }
    }

    const std::string& InputFieldWidgetComponent::GetText() const {
        return text;
    }

    void InputFieldWidgetComponent::SetPlaceholder(const std::string& ph) {
        placeholder = ph;
    }

    const std::string& InputFieldWidgetComponent::GetPlaceholder() const {
        return placeholder;
    }

    void InputFieldWidgetComponent::SetCharacterLimit(int limit) {
        characterLimit = limit;
        if (characterLimit > 0 && static_cast<int>(text.length()) > characterLimit) {
            text = text.substr(0, characterLimit);
            caretPosition = std::min(caretPosition, characterLimit);
        }
    }

    int InputFieldWidgetComponent::GetCharacterLimit() const {
        return characterLimit;
    }

    void InputFieldWidgetComponent::SetContentType(EInputFieldContentType type) {
        contentType = type;
    }

    EInputFieldContentType InputFieldWidgetComponent::GetContentType() const {
        return contentType;
    }

    void InputFieldWidgetComponent::SetLineType(EInputFieldLineType type) {
        lineType = type;
    }

    EInputFieldLineType InputFieldWidgetComponent::GetLineType() const {
        return lineType;
    }

    void InputFieldWidgetComponent::SetInteractable(bool val) {
        interactable = val;
        if (!interactable && isFocused) {
            Unfocus();
        }
    }

    bool InputFieldWidgetComponent::IsInteractable() const {
        return interactable;
    }

    void InputFieldWidgetComponent::SetReadOnly(bool val) {
        readOnly = val;
    }

    bool InputFieldWidgetComponent::IsReadOnly() const {
        return readOnly;
    }

    void InputFieldWidgetComponent::SetBackgroundColor(const glm::vec4& color) {
        backgroundColor = color;
    }

    glm::vec4 InputFieldWidgetComponent::GetBackgroundColor() const {
        return backgroundColor;
    }

    void InputFieldWidgetComponent::SetTextColor(const glm::vec4& color) {
        textColor = color;
    }

    glm::vec4 InputFieldWidgetComponent::GetTextColor() const {
        return textColor;
    }

    void InputFieldWidgetComponent::SetPlaceholderColor(const glm::vec4& color) {
        placeholderColor = color;
    }

    glm::vec4 InputFieldWidgetComponent::GetPlaceholderColor() const {
        return placeholderColor;
    }

    void InputFieldWidgetComponent::SetSelectionColor(const glm::vec4& color) {
        selectionColor = color;
    }

    glm::vec4 InputFieldWidgetComponent::GetSelectionColor() const {
        return selectionColor;
    }

    void InputFieldWidgetComponent::SetCaretColor(const glm::vec4& color) {
        caretColor = color;
    }

    glm::vec4 InputFieldWidgetComponent::GetCaretColor() const {
        return caretColor;
    }

    void InputFieldWidgetComponent::SetCaretBlinkRate(float rate) {
        caretBlinkRate = rate;
    }

    float InputFieldWidgetComponent::GetCaretBlinkRate() const {
        return caretBlinkRate;
    }

    void InputFieldWidgetComponent::SetCaretWidth(float width) {
        caretWidth = width;
    }

    float InputFieldWidgetComponent::GetCaretWidth() const {
        return caretWidth;
    }

    void InputFieldWidgetComponent::SetFont(const std::shared_ptr<Font>& pFont) {
        font = pFont;
    }

    const std::shared_ptr<Font>& InputFieldWidgetComponent::GetFont() const {
        return font;
    }

    void InputFieldWidgetComponent::SetFont(const std::string_view path, int size) {
        font = Engine::GetInstance().GetAssetManager().EnsureFont(path, size);
        fontSize = size;
    }

    void InputFieldWidgetComponent::SetFontSize(int size) {
        fontSize = size;
        if (font) {
            font->SetSize(size);
        }
    }

    int InputFieldWidgetComponent::GetFontSize() const {
        return fontSize;
    }

    void InputFieldWidgetComponent::SetPadding(const glm::vec4& pad) {
        padding = pad;
    }

    glm::vec4 InputFieldWidgetComponent::GetPadding() const {
        return padding;
    }

    bool InputFieldWidgetComponent::IsFocused() const {
        return isFocused;
    }

    void InputFieldWidgetComponent::Focus() {
        if (interactable && !isFocused) {
            isFocused = true;
            caretVisible = true;
            caretBlinkTimer = 0.0f;
            SDL_StartTextInput(Engine::GetInstance().GetApplication()->GetWindowNativeHandle());
        }
    }

    void InputFieldWidgetComponent::Unfocus() {
        if (isFocused) {
            isFocused = false;
            SDL_StopTextInput(Engine::GetInstance().GetApplication()->GetWindowNativeHandle());
            
            if (OnEndEdit) {
                OnEndEdit(text);
            }
        }
    }

    void InputFieldWidgetComponent::Start() {
        // Only set default font if no font has been set yet
        if (!font) {
            font = Engine::GetInstance().GetAssetManager().EnsureFont("fonts/NotoSans.ttf", fontSize);
        }
    }

    void InputFieldWidgetComponent::Update(float deltaTime) {
        if (!isFocused) {
            return;
        }

        // Handle caret blinking
        caretBlinkTimer += deltaTime;
        if (caretBlinkTimer >= caretBlinkRate) {
            caretBlinkTimer = 0.0f;
            caretVisible = !caretVisible;
        }

        ProcessKeyInput(deltaTime);
    }

    void InputFieldWidgetComponent::Draw(CanvasComponent* pCanvas) {
        if (!pCanvas) {
            return;
        }

        auto rt = GetOwner()->GetComponent<RectTransformComponent>();
        if (!rt) {
            return;
        }

        auto pos = rt->GetScreenPosition();
        float uiScale = Engine::GetInstance().GetSceneRenderer().GetUIScale();
        glm::vec2 scaledSize = rt->GetSize() * uiScale;
        glm::vec4 scaledPadding = padding * uiScale;

        // Draw background
        glm::vec4 bgColor = backgroundColor;
        if (!interactable) {
            bgColor *= 0.5f;
            bgColor.a = backgroundColor.a;
        }

        pCanvas->DrawTexture2D(
            glm::vec3(pos, 0.0f),
            glm::vec3(pos.x + scaledSize.x, pos.y + scaledSize.y, 0.0f),
            glm::vec2(0.0f),
            glm::vec2(1.0f),
            nullptr,
            bgColor);

        // Draw border when focused
        if (isFocused) {
            glm::vec4 borderColor(0.4f, 0.6f, 1.0f, 1.0f);
            float borderWidth = 2.0f * uiScale;
            
            // Top border
            pCanvas->DrawTexture2D(
                glm::vec3(pos.x, pos.y + scaledSize.y - borderWidth, 0.01f),
                glm::vec3(pos.x + scaledSize.x, pos.y + scaledSize.y, 0.01f),
                glm::vec2(0.0f), glm::vec2(1.0f), nullptr, borderColor);
            // Bottom border
            pCanvas->DrawTexture2D(
                glm::vec3(pos.x, pos.y, 0.01f),
                glm::vec3(pos.x + scaledSize.x, pos.y + borderWidth, 0.01f),
                glm::vec2(0.0f), glm::vec2(1.0f), nullptr, borderColor);
            // Left border
            pCanvas->DrawTexture2D(
                glm::vec3(pos.x, pos.y, 0.01f),
                glm::vec3(pos.x + borderWidth, pos.y + scaledSize.y, 0.01f),
                glm::vec2(0.0f), glm::vec2(1.0f), nullptr, borderColor);
            // Right border
            pCanvas->DrawTexture2D(
                glm::vec3(pos.x + scaledSize.x - borderWidth, pos.y, 0.01f),
                glm::vec3(pos.x + scaledSize.x, pos.y + scaledSize.y, 0.01f),
                glm::vec2(0.0f), glm::vec2(1.0f), nullptr, borderColor);
        }

        // Calculate text area bounds (for clipping)
        float textAreaLeft = pos.x + scaledPadding.x;
        float textAreaRight = pos.x + scaledSize.x - scaledPadding.z;
        float textAreaWidth = textAreaRight - textAreaLeft;

        // Draw text or placeholder
        if (font) {
            std::string displayText = GetDisplayText();
            bool showPlaceholder = displayText.empty() && !placeholder.empty();
            
            const std::string& textToDraw = showPlaceholder ? placeholder : displayText;
            const glm::vec4& colorToUse = showPlaceholder ? placeholderColor : textColor;
            
            if (!textToDraw.empty()) {
                float textX = textAreaLeft - (showPlaceholder ? 0.0f : scrollOffset);
                float textHeight = static_cast<float>(fontSize) * uiScale;
                float textY = pos.y + (scaledSize.y - textHeight) * 0.5f + textHeight * 0.8f; // baseline adjustment
                
                auto& assetManager = Engine::GetInstance().GetAssetManager();
                
                for (char c : textToDraw) {
                    uint32_t codepoint = static_cast<uint32_t>(static_cast<unsigned char>(c));
                    
                    std::shared_ptr<Font> glyphFont;
                    const Glyph* glyph = assetManager.GetGlyphWithFallback(font, codepoint, glyphFont);
                    
                    if (!glyph || !glyphFont) {
                        continue;
                    }
                    
                    int glyphW = glyphFont->GetTexture()->GetWidth();
                    int glyphH = glyphFont->GetTexture()->GetHeight();
                    
                    float x1 = textX;
                    float y1 = textY - glyph->height * uiScale + glyph->yOffset * uiScale;
                    float x2 = x1 + static_cast<float>(glyph->width) * uiScale;
                    float y2 = y1 + static_cast<float>(glyph->height) * uiScale;
                    
                    // Only draw if glyph is at least partially visible
                    if (x2 > textAreaLeft && x1 < textAreaRight) {
                        float u1 = static_cast<float>(glyph->x0) / static_cast<float>(glyphW);
                        float v1 = static_cast<float>(glyph->y0) / static_cast<float>(glyphH);
                        float u2 = static_cast<float>(glyph->x1) / static_cast<float>(glyphW);
                        float v2 = static_cast<float>(glyph->y1) / static_cast<float>(glyphH);
                        
                        // Clip glyph horizontally
                        float clippedX1 = std::max(x1, textAreaLeft);
                        float clippedX2 = std::min(x2, textAreaRight);
                        
                        // Adjust UVs for clipping
                        float glyphWidth = x2 - x1;
                        if (glyphWidth > 0) {
                            float leftClipRatio = (clippedX1 - x1) / glyphWidth;
                            float rightClipRatio = (x2 - clippedX2) / glyphWidth;
                            float uvWidth = u2 - u1;
                            float clippedU1 = u1 + uvWidth * leftClipRatio;
                            float clippedU2 = u2 - uvWidth * rightClipRatio;
                            
                            pCanvas->DrawTexture2D(
                                glm::vec3(clippedX1, y1, 0.03f),
                                glm::vec3(clippedX2, y2, 0.03f),
                                glm::vec2(clippedU1, v2),
                                glm::vec2(clippedU2, v1),
                                glyphFont->GetTexture().get(),
                                colorToUse);
                        }
                    }
                    
                    textX += static_cast<float>(glyph->advance) * uiScale;
                }
            }
        }

        // Draw caret if focused
        if (isFocused && caretVisible && !readOnly && font) {
            std::string displayText = GetDisplayText();
            std::string textBeforeCaret = displayText.substr(0, std::min(caretPosition, static_cast<int>(displayText.length())));
            
            float textWidth = 0.0f;
            for (char c : textBeforeCaret) {
                if (auto glyph = font->GetGlyph(static_cast<uint32_t>(c))) {
                    textWidth += glyph->advance * uiScale;
                }
            }
            
            float caretX = textAreaLeft + textWidth - scrollOffset;
            
            // Only draw caret if visible within text area
            if (caretX >= textAreaLeft && caretX <= textAreaRight) {
                float caretHeight = fontSize * uiScale;
                float caretY = pos.y + (scaledSize.y - caretHeight) * 0.5f;

                pCanvas->DrawTexture2D(
                    glm::vec3(caretX, caretY, 0.02f),
                    glm::vec3(caretX + caretWidth * uiScale, caretY + caretHeight, 0.02f),
                    glm::vec2(0.0f),
                    glm::vec2(1.0f),
                    nullptr,
                    caretColor);
            }
        }
    }

    void InputFieldWidgetComponent::LoadProperties(const nlohmann::json& json) {
        text = json.value("text", "");
        placeholder = json.value("placeholder", "Enter text...");
        characterLimit = json.value("characterLimit", 0);
        interactable = json.value("interactable", true);
        readOnly = json.value("readOnly", false);
        fontSize = json.value("fontSize", 14);
        caretBlinkRate = json.value("caretBlinkRate", 0.53f);
        caretWidth = json.value("caretWidth", 2.0f);

        if (json.contains("backgroundColor")) {
            const auto& col = json["backgroundColor"];
            backgroundColor = {col.value("r", 0.2f), col.value("g", 0.2f), col.value("b", 0.2f), col.value("a", 1.0f)};
        }

        if (json.contains("textColor")) {
            const auto& col = json["textColor"];
            textColor = {col.value("r", 1.0f), col.value("g", 1.0f), col.value("b", 1.0f), col.value("a", 1.0f)};
        }

        if (json.contains("placeholderColor")) {
            const auto& col = json["placeholderColor"];
            placeholderColor = {col.value("r", 0.5f), col.value("g", 0.5f), col.value("b", 0.5f), col.value("a", 1.0f)};
        }

        if (json.contains("padding")) {
            const auto& pad = json["padding"];
            padding = {pad.value("left", 8.0f), pad.value("bottom", 4.0f), pad.value("right", 8.0f), pad.value("top", 4.0f)};
        }
    }

    bool InputFieldWidgetComponent::HitTest(const glm::vec2& point) const {
        if (!interactable) {
            return false;
        }

        auto rt = GetOwner()->GetComponent<RectTransformComponent>();
        if (!rt) {
            return false;
        }

        float uiScale = Engine::GetInstance().GetSceneRenderer().GetUIScale();
        auto p1 = rt->GetScreenPosition();
        glm::vec2 scaledSize = rt->GetSize() * uiScale;
        auto p2 = p1 + scaledSize;

        return (point.x >= p1.x) && (point.x <= p2.x) && (point.y >= p1.y) && (point.y <= p2.y);
    }

    void InputFieldWidgetComponent::OnPointerDown() {
        Focus();
    }

    void InputFieldWidgetComponent::OnClick() {
        // Already handled by OnPointerDown
    }

    void InputFieldWidgetComponent::ProcessTextInput(const std::string& inputText) {
        if (readOnly || !isFocused) {
            return;
        }

        // Delete selection if there is one
        if (selectionStart != selectionEnd) {
            int start = std::min(selectionStart, selectionEnd);
            int end = std::max(selectionStart, selectionEnd);
            text.erase(start, end - start);
            caretPosition = start;
            selectionStart = selectionEnd = caretPosition;
        }

        for (char c : inputText) {
            if (IsCharacterValid(c)) {
                if (characterLimit <= 0 || static_cast<int>(text.length()) < characterLimit) {
                    text.insert(caretPosition, 1, c);
                    caretPosition++;
                    caretVisible = true;
                    caretBlinkTimer = 0.0f;
                }
            }
        }

        selectionStart = selectionEnd = caretPosition;
        UpdateScrollOffset();

        if (OnValueChanged) {
            OnValueChanged(text);
        }
    }

    void InputFieldWidgetComponent::ProcessKeyInput(float deltaTime) {
        auto& input = Engine::GetInstance().GetInputManager();

        // Handle backspace with proper repeat delay
        if (input.IsKeyJustPressed(SDL_SCANCODE_BACKSPACE)) {
            HandleBackspace();
            keyRepeatTimer = 0.0f;
            keyRepeatKey = SDL_SCANCODE_BACKSPACE;
        } else if (input.IsKeyPressed(SDL_SCANCODE_BACKSPACE) && keyRepeatKey == SDL_SCANCODE_BACKSPACE) {
            keyRepeatTimer += deltaTime;
            if (keyRepeatTimer >= keyRepeatDelay) {
                HandleBackspace();
                keyRepeatTimer = keyRepeatDelay - keyRepeatRate;
            }
        }

        // Handle delete with proper repeat delay
        if (input.IsKeyJustPressed(SDL_SCANCODE_DELETE)) {
            HandleDelete();
            keyRepeatTimer = 0.0f;
            keyRepeatKey = SDL_SCANCODE_DELETE;
        } else if (input.IsKeyPressed(SDL_SCANCODE_DELETE) && keyRepeatKey == SDL_SCANCODE_DELETE) {
            keyRepeatTimer += deltaTime;
            if (keyRepeatTimer >= keyRepeatDelay) {
                HandleDelete();
                keyRepeatTimer = keyRepeatDelay - keyRepeatRate;
            }
        }

        // Handle left arrow with proper repeat delay
        if (input.IsKeyJustPressed(SDL_SCANCODE_LEFT)) {
            MoveCaret(-1);
            keyRepeatTimer = 0.0f;
            keyRepeatKey = SDL_SCANCODE_LEFT;
        } else if (input.IsKeyPressed(SDL_SCANCODE_LEFT) && keyRepeatKey == SDL_SCANCODE_LEFT) {
            keyRepeatTimer += deltaTime;
            if (keyRepeatTimer >= keyRepeatDelay) {
                MoveCaret(-1);
                keyRepeatTimer = keyRepeatDelay - keyRepeatRate;
            }
        }

        // Handle right arrow with proper repeat delay
        if (input.IsKeyJustPressed(SDL_SCANCODE_RIGHT)) {
            MoveCaret(1);
            keyRepeatTimer = 0.0f;
            keyRepeatKey = SDL_SCANCODE_RIGHT;
        } else if (input.IsKeyPressed(SDL_SCANCODE_RIGHT) && keyRepeatKey == SDL_SCANCODE_RIGHT) {
            keyRepeatTimer += deltaTime;
            if (keyRepeatTimer >= keyRepeatDelay) {
                MoveCaret(1);
                keyRepeatTimer = keyRepeatDelay - keyRepeatRate;
            }
        }

        // Reset key repeat if key was released
        if (!input.IsKeyPressed(keyRepeatKey)) {
            keyRepeatKey = 0;
            keyRepeatTimer = 0.0f;
        }

        // Handle home
        if (input.IsKeyJustPressed(SDL_SCANCODE_HOME)) {
            caretPosition = 0;
            UpdateScrollOffset();
            caretVisible = true;
            caretBlinkTimer = 0.0f;
        }

        // Handle end
        if (input.IsKeyJustPressed(SDL_SCANCODE_END)) {
            caretPosition = static_cast<int>(text.length());
            UpdateScrollOffset();
            caretVisible = true;
            caretBlinkTimer = 0.0f;
        }

        // Handle Ctrl+A to select all
        if ((input.IsKeyPressed(SDL_SCANCODE_LCTRL) || input.IsKeyPressed(SDL_SCANCODE_RCTRL)) && 
            input.IsKeyJustPressed(SDL_SCANCODE_A)) {
            selectionStart = 0;
            selectionEnd = static_cast<int>(text.length());
            caretPosition = selectionEnd;
            UpdateScrollOffset();
        }

        // Handle enter
        if (input.IsKeyJustPressed(SDL_SCANCODE_RETURN) || input.IsKeyJustPressed(SDL_SCANCODE_KP_ENTER)) {
            if (lineType == EInputFieldLineType::SINGLE_LINE) {
                if (OnSubmit) {
                    OnSubmit(text);
                }
                Unfocus();
            } else if (lineType == EInputFieldLineType::MULTI_LINE_SUBMIT) {
                if (OnSubmit) {
                    OnSubmit(text);
                }
            } else {
                // Multi-line newline
                if (!readOnly) {
                    ProcessTextInput("\n");
                }
            }
        }

        // Handle escape to unfocus
        if (input.IsKeyJustPressed(SDL_SCANCODE_ESCAPE)) {
            Unfocus();
        }

        // Handle text input events from SDL
        const std::string& textInput = input.GetTextInput();
        if (!textInput.empty()) {
            ProcessTextInput(textInput);
        }
    }

    void InputFieldWidgetComponent::HandleBackspace() {
        if (readOnly) return;
        
        // Delete selection if there is one
        if (selectionStart != selectionEnd) {
            int start = std::min(selectionStart, selectionEnd);
            int end = std::max(selectionStart, selectionEnd);
            text.erase(start, end - start);
            caretPosition = start;
            selectionStart = selectionEnd = caretPosition;
        } else if (caretPosition > 0) {
            text.erase(caretPosition - 1, 1);
            caretPosition--;
        }
        
        UpdateScrollOffset();
        caretVisible = true;
        caretBlinkTimer = 0.0f;
        
        if (OnValueChanged) {
            OnValueChanged(text);
        }
    }

    void InputFieldWidgetComponent::HandleDelete() {
        if (readOnly) return;
        
        // Delete selection if there is one
        if (selectionStart != selectionEnd) {
            int start = std::min(selectionStart, selectionEnd);
            int end = std::max(selectionStart, selectionEnd);
            text.erase(start, end - start);
            caretPosition = start;
            selectionStart = selectionEnd = caretPosition;
        } else if (caretPosition < static_cast<int>(text.length())) {
            text.erase(caretPosition, 1);
        }
        
        UpdateScrollOffset();
        caretVisible = true;
        caretBlinkTimer = 0.0f;
        
        if (OnValueChanged) {
            OnValueChanged(text);
        }
    }

    void InputFieldWidgetComponent::MoveCaret(int direction) {
        caretPosition = std::clamp(caretPosition + direction, 0, static_cast<int>(text.length()));
        selectionStart = selectionEnd = caretPosition;
        UpdateScrollOffset();
        caretVisible = true;
        caretBlinkTimer = 0.0f;
    }

    void InputFieldWidgetComponent::UpdateScrollOffset() {
        if (!font) return;
        
        auto rt = GetOwner()->GetComponent<RectTransformComponent>();
        if (!rt) return;
        
        float uiScale = Engine::GetInstance().GetSceneRenderer().GetUIScale();
        glm::vec2 scaledSize = rt->GetSize() * uiScale;
        glm::vec4 scaledPadding = padding * uiScale;
        
        float availableWidth = scaledSize.x - scaledPadding.x - scaledPadding.z;
        
        // Calculate text width up to caret
        std::string displayText = GetDisplayText();
        std::string textBeforeCaret = displayText.substr(0, std::min(caretPosition, static_cast<int>(displayText.length())));
        
        float textWidth = 0.0f;
        for (char c : textBeforeCaret) {
            if (auto glyph = font->GetGlyph(static_cast<uint32_t>(c))) {
                textWidth += glyph->advance * uiScale;
            }
        }
        
        // Adjust scroll offset so caret is always visible
        if (textWidth - scrollOffset > availableWidth - caretWidth * uiScale) {
            scrollOffset = textWidth - availableWidth + caretWidth * uiScale + 5.0f;
        } else if (textWidth - scrollOffset < 0) {
            scrollOffset = textWidth;
        }
        
        // Clamp scroll offset to valid range
        float totalTextWidth = 0.0f;
        for (char c : displayText) {
            if (auto glyph = font->GetGlyph(static_cast<uint32_t>(c))) {
                totalTextWidth += glyph->advance * uiScale;
            }
        }
        
        float maxScroll = std::max(0.0f, totalTextWidth - availableWidth);
        scrollOffset = std::clamp(scrollOffset, 0.0f, maxScroll);
    }

    bool InputFieldWidgetComponent::IsCharacterValid(char c) const {
        switch (contentType) {
            case EInputFieldContentType::INTEGER_NUMBER:
                return std::isdigit(c) || (c == '-' && caretPosition == 0 && text.find('-') == std::string::npos);
            
            case EInputFieldContentType::DECIMAL_NUMBER:
                return std::isdigit(c) || 
                       (c == '-' && caretPosition == 0 && text.find('-') == std::string::npos) ||
                       (c == '.' && text.find('.') == std::string::npos);
            
            case EInputFieldContentType::ALPHANUMERIC:
                return std::isalnum(c);
            
            case EInputFieldContentType::PIN:
                return std::isdigit(c);
            
            default:
                return c >= 32; // Printable characters
        }
    }

    std::string InputFieldWidgetComponent::GetDisplayText() const {
        if (contentType == EInputFieldContentType::PASSWORD) {
            return std::string(text.length(), '*');
        }
        return text;
    }

} // namespace golias
