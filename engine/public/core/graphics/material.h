#pragma once

#include "shader.h"
#include "core/graphics/structs.h"
#include <json.hpp>

namespace golias {

    class Material {
    public:
        void Activate();

        void SetShader(const std::shared_ptr<Shader>& pShader);
        std::shared_ptr<Shader> GetShader() const;

        void SetImageBasedLighting(bool value);
        bool UseImageBasedLighting() const;

        bool IsTransparent() const;
        EBlendMode GetBlendMode() const;

        void SetBlendMode(EBlendMode mode);

        float GetAlphaClipThreshold() const;
        void SetAlphaClipThreshold(float threshold);


        bool IsDepthTestEnabled() const;
        void SetDepthTestEnabled(bool enabled);

        bool IsDepthWriteEnabled() const;

        void SetDepthWriteEnabled(bool enabled);
        EComparisonFunc GetDepthFunc() const;

        void SetDepthFunc(EComparisonFunc func);

        ECullMode GetCullMode() const;

        void SetCullMode(ECullMode mode);

        template <typename T>
        void SetParameter(const std::string_view pName, const T& value) {
            parameters[std::string(pName)] = value;
        }

        static std::shared_ptr<Material> Load(const std::string_view pPath, const nlohmann::json* paramOverrides = nullptr);

        static void ParseParameters(std::shared_ptr<Material>& material, const nlohmann::json& json);

    private:
        std::shared_ptr<Shader> shader;

        EBlendMode blendMode      = EBlendMode::BLEND_MODE_OPAQUE;
        float alphaClipThreshold  = 0.5f;
        bool depthTestEnabled     = true;
        bool depthWriteEnabled    = true;
        EComparisonFunc depthFunc = EComparisonFunc::COMPARISON_LESS;
        ECullMode cullMode        = ECullMode::CULL_MODE_BACK;
        bool useIBL               = false;

        std::unordered_map<std::string, UniformValue> parameters;
    };

    class MaterialManager {
    public:
        static MaterialManager& GetInstance();

        std::shared_ptr<Material> GetMaterial(const std::string_view pPath);

        void RegisterMaterial(const std::string_view pPath, const std::shared_ptr<Material>& pMaterial);

    private:
        std::unordered_map<std::string, std::shared_ptr<Material>> materials;
    };
} // namespace golias
