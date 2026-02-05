#pragma once
#include "scene/ui/ui_base.h"
#include <functional>
#include <glm/vec4.hpp>

namespace golias {

    class Font;

    /// @brief Input field content type
    enum class EInputFieldContentType {
        STANDARD,
        AUTOCORRECTED,
        INTEGER_NUMBER,
        DECIMAL_NUMBER,
        ALPHANUMERIC,
        NAME,
        EMAIL_ADDRESS,
        PASSWORD,
        PIN,
        CUSTOM
    };

    /// @brief Input field line type
    enum class EInputFieldLineType {
        SINGLE_LINE,
        MULTI_LINE_SUBMIT,
        MULTI_LINE_NEWLINE
    };

    using InputFieldCallback = std::function<void(const std::string&)>;
    using InputFieldSubmitCallback = std::function<void(const std::string&)>;

    /// @brief InputField component for text input
    class InputFieldWidgetComponent : public WidgetComponent {
        COMPONENT_DERIVED(InputFieldWidgetComponent, WidgetComponent)
    public:
        void SetText(const std::string& text);
        const std::string& GetText() const;

        void SetPlaceholder(const std::string& placeholder);
        const std::string& GetPlaceholder() const;

        void SetCharacterLimit(int limit);
        int GetCharacterLimit() const;

        void SetContentType(EInputFieldContentType type);
        EInputFieldContentType GetContentType() const;

        void SetLineType(EInputFieldLineType type);
        EInputFieldLineType GetLineType() const;

        void SetInteractable(bool interactable);
        bool IsInteractable() const;

        void SetReadOnly(bool readOnly);
        bool IsReadOnly() const;

        void SetBackgroundColor(const glm::vec4& color);
        glm::vec4 GetBackgroundColor() const;

        void SetTextColor(const glm::vec4& color);
        glm::vec4 GetTextColor() const;

        void SetPlaceholderColor(const glm::vec4& color);
        glm::vec4 GetPlaceholderColor() const;

        void SetSelectionColor(const glm::vec4& color);
        glm::vec4 GetSelectionColor() const;

        void SetCaretColor(const glm::vec4& color);
        glm::vec4 GetCaretColor() const;

        void SetCaretBlinkRate(float rate);
        float GetCaretBlinkRate() const;

        void SetCaretWidth(float width);
        float GetCaretWidth() const;

        void SetFont(const std::shared_ptr<Font>& pFont);
        const std::shared_ptr<Font>& GetFont() const;
        void SetFont(const std::string_view path, int size);
        void SetFontSize(int size);
        int GetFontSize() const;

        void SetPadding(const glm::vec4& padding);
        glm::vec4 GetPadding() const;

        bool IsFocused() const;
        void Focus();
        void Unfocus();

        void Start() override;
        void Update(float deltaTime) override;
        void Draw(CanvasComponent* pCanvas) override;
        void LoadProperties(const nlohmann::json& json) override;

        bool HitTest(const glm::vec2& point) const override;
        void OnPointerDown() override;
        void OnClick() override;

        /// @brief Called when text changes
        InputFieldCallback OnValueChanged;
        
        /// @brief Called when enter is pressed or focus is lost
        InputFieldSubmitCallback OnEndEdit;
        
        /// @brief Called when enter is pressed
        InputFieldSubmitCallback OnSubmit;

    private:
        void ProcessTextInput(const std::string& inputText);
        void ProcessKeyInput(float deltaTime);
        bool IsCharacterValid(char c) const;
        std::string GetDisplayText() const;
        void HandleBackspace();
        void HandleDelete();
        void MoveCaret(int direction);
        void UpdateScrollOffset();

        std::string text;
        std::string placeholder = "Enter text...";
        
        int characterLimit = 0; // 0 = unlimited
        int caretPosition = 0;
        int selectionStart = 0;
        int selectionEnd = 0;
        
        EInputFieldContentType contentType = EInputFieldContentType::STANDARD;
        EInputFieldLineType lineType = EInputFieldLineType::SINGLE_LINE;
        
        bool interactable = true;
        bool readOnly = false;
        bool isFocused = false;
        
        glm::vec4 backgroundColor = glm::vec4(0.2f, 0.2f, 0.2f, 1.0f);
        glm::vec4 textColor = glm::vec4(1.0f);
        glm::vec4 placeholderColor = glm::vec4(0.5f, 0.5f, 0.5f, 1.0f);
        glm::vec4 selectionColor = glm::vec4(0.3f, 0.5f, 0.8f, 0.5f);
        glm::vec4 caretColor = glm::vec4(1.0f);
        
        float caretBlinkRate = 0.53f;
        float caretBlinkTimer = 0.0f;
        bool caretVisible = true;
        float caretWidth = 2.0f;
        
        std::shared_ptr<Font> font = nullptr;
        int fontSize = 14;
        
        glm::vec4 padding = glm::vec4(8.0f, 4.0f, 8.0f, 4.0f); // left, bottom, right, top
        
        // Key repeat handling
        float keyRepeatDelay = 0.4f;  // Initial delay before repeat starts
        float keyRepeatRate = 0.05f;  // Rate at which keys repeat
        float keyRepeatTimer = 0.0f;
        int keyRepeatKey = 0;
        
        // Text scrolling for overflow
        float scrollOffset = 0.0f;
    };

} // namespace golias
