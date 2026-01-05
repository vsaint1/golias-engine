#include "scene/2d/sprite_component_2d.h"
#include "core/engine.h"

namespace golias {

    void SpriteComponent2D::SetTexture(const std::shared_ptr<Texture2D>& tex) {
        texture = tex;
    }

    std::shared_ptr<Texture2D> SpriteComponent2D::GetTexture() const {
        return texture;
    }

    void SpriteComponent2D::SetColor(const glm::vec4& value) {
        color = value;
    }

    glm::vec4 SpriteComponent2D::GetColor() const {
        return color;
    }

    void SpriteComponent2D::SetSize(const glm::vec2& value) {
        size = value;
    }

    glm::vec2 SpriteComponent2D::GetSize() const {
        return size;
    }

    void SpriteComponent2D::SetLowerLeftUV(const glm::vec2& uv) {
        lowerLeftUV = uv;
    }

    glm::vec2 SpriteComponent2D::GetLowerLeftUV() const {
        return lowerLeftUV;
    }

    void SpriteComponent2D::SetUpperRightUV(const glm::vec2& uv) {
        upperRightUV = uv;
    }

    glm::vec2 SpriteComponent2D::GetUpperRightUV() const {
        return upperRightUV;
    }

    void SpriteComponent2D::SetPivot(const glm::vec2& value) {
        pivot = value;
    }

    glm::vec2 SpriteComponent2D::GetPivot() const {
        return pivot;
    }

    void SpriteComponent2D::SetVisible(bool value) {
        visible = value;
    }

    bool SpriteComponent2D::IsVisible() const {
        return visible;
    }

    void SpriteComponent2D::LoadProperties(const nlohmann::json& json) {
        const std::string path = json.value("texture", "");
       
        if(auto tex = Engine::GetInstance().GetTextureManager().EnsureTexture2D(path)) {
            SetTexture(tex);
        }

        if(json.contains("color")) {
            const auto& colorObj = json["color"];
            glm::vec4 col = glm::vec4(1.0f);
            col.r = colorObj.value("r", 1.0f);
            col.g = colorObj.value("g", 1.0f);
            col.b = colorObj.value("b", 1.0f);
            col.a = colorObj.value("a", 1.0f);
            SetColor(col);
        }

        if(json.contains("size")) {
            const auto& sizeObj = json["size"];
            glm::vec2 sz = glm::vec2(32.0f, 32.0f);
            sz.x = sizeObj.value("x", 32.0f);
            sz.y = sizeObj.value("y", 32.0f);
            SetSize(sz);
        }


        if (json.contains("uv")) {
            const auto& uvObj = json["uv"];
            glm::vec2 lluv = glm::vec2(0.0f, 0.0f);
            glm::vec2 uruv = glm::vec2(1.0f, 1.0f);
            lluv.x = uvObj.value("lower_left_u", 0.0f);
            lluv.y = uvObj.value("lower_left_v", 0.0f);
            uruv.x = uvObj.value("upper_right_u", 1.0f);
            uruv.y = uvObj.value("upper_right_v", 1.0f);
            SetLowerLeftUV(lluv);
            SetUpperRightUV(uruv);
        }

        if (json.contains("pivot")) {
            const auto& pivotObj = json["pivot"];
            glm::vec2 pv = glm::vec2(0.5f, 0.5f);
            pv.x = pivotObj.value("x", 0.5f);
            pv.y = pivotObj.value("y", 0.5f);
            SetPivot(pv);
        }
    }

    void SpriteComponent2D::Start() {
    }

    void SpriteComponent2D::Update(float deltaTime) {

        if(!texture || !visible) {
            return;
        }

        auto& rendering_canvas = golias::Engine::GetInstance().GetRenderingCanvas();


        DrawCommand2D command;
        command.modelMatrix = GetOwner()->GetWorldTransform2D();
        command.texture     = texture.get();
        command.color       = color;
        command.size        = size;
        command.lowerLeftUV  = lowerLeftUV;
        command.upperRightUV = upperRightUV;
        command.pivot       = pivot;
        rendering_canvas.Submit(command);   
    }

} // namespace golias
