#pragma once

#include "core/ember_core.h"

/*
   @brief Opengl Renderer implementation
   - Opengl 3.3
   - Opengl ES 3.0
*/
class OpenglRenderer final : public Renderer {
public:
    OpenglRenderer() = default;

    OpenglShader* GetDefaultShader() override;

    OpenglShader* GetTextShader() override;

    void Resize(int view_width, int view_height) override;

    void SetContext(const void* ctx) override;

    void* GetContext() override;

    void Destroy() override;

    Texture LoadTexture(const std::string& file_path) override;

    Font LoadFont(const std::string& file_path, int font_size) override;

    void UnloadFont(const Font& font) override;

    void UnloadTexture(const Texture& texture) override;

    void ClearBackground(const Color& color) override;

    void BeginDrawing() override;

    void EndDrawing() override;

    void DrawText(const Font& font, const std::string& text, Transform transform, Color color, float font_size, const ShaderEffect& shader_effect = {},
                  float kerning = 0.0f) override;

    void DrawTexture(const Texture& texture, ember::Rectangle rect, Color color) override;

    void DrawTextureEx(const Texture& texture, const ember::Rectangle& source, const ember::Rectangle& dest,
                       glm::vec2 origin, float rotation, const Color& color) override;

    void DrawLine(glm::vec2 start, glm::vec2 end, const Color& color, float thickness) override;

    void DrawRect(const ember::Rectangle& rect, const Color& color, float thickness) override;

    void DrawRectFilled(const ember::Rectangle& rect, const Color& color) override;

    void BeginMode2D(const Camera2D& camera) override;

    void EndMode2D() override;

    void BeginCanvas() override;

    void EndCanvas() override;

    OpenglShader* default_shader = nullptr;
    OpenglShader* text_shader = nullptr;
private:
    unsigned int VAO = 0, VBO = 0, EBO = 0;

    SDL_GLContext context = nullptr;
};

