#pragma once

#include "shader.h"

#include <json.hpp>

namespace golias {

    class Material {
    public:
        void Activate();

        void SetShader(const std::shared_ptr<Shader>& pShader);
        std::shared_ptr<Shader> GetShader() const;

        void SetUseIBL(bool value) {
            this->useIBL = value;
        }

        bool UseImageBasedLighting() const;

        bool IsTransparent() const{
            return blendMode != EBlendMode::BLEND_MODE_OPAQUE;
        }

        EBlendMode GetBlendMode() const {
            return blendMode;
        }
        
        void SetBlendMode(EBlendMode mode) {
            blendMode = mode;
        }

        float GetAlphaClipThreshold() const {
            return alphaClipThreshold;
        }
        void SetAlphaClipThreshold(float threshold) {
            alphaClipThreshold = threshold;
        }

        // Depth testing
        bool IsDepthTestEnabled() const {
            return depthTestEnabled;
        }
        void SetDepthTestEnabled(bool enabled) {
            depthTestEnabled = enabled;
        }

        bool IsDepthWriteEnabled() const {
            return depthWriteEnabled;
        }
        void SetDepthWriteEnabled(bool enabled) {
            depthWriteEnabled = enabled;
        }

        EComparisonFunc GetDepthFunc() const {
            return depthFunc;
        }
        void SetDepthFunc(EComparisonFunc func) {
            depthFunc = func;
        }

        ECullMode GetCullMode() const {
            return cullMode;
        }
        void SetCullMode(ECullMode mode) {
            cullMode = mode;
        }

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
        ECullMode cullMode        = ECullMode::CULL_BACK;
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
