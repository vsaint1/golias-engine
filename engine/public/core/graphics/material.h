#pragma once

#include "shader.h"
#include <json.hpp>

namespace golias {

    class Material {
    public:

        void Activate();

        void SetShader(const std::shared_ptr<Shader>& pShader);
        std::shared_ptr<Shader> GetShader() const;

        void SetUseIBL(bool value) { this->useIBL = value; }
        bool UseImageBasedLighting() const;

        template<typename T>
        void SetParameter(const std::string_view pName, const T& value) {
            parameters[std::string(pName)] = value;
        }

        static std::shared_ptr<Material> Load(const std::string_view pPath, const nlohmann::json* paramOverrides = nullptr);

        static void ParseParameters(std::shared_ptr<Material>& material, const nlohmann::json& json);

    private:
        std::shared_ptr<Shader> shader;
        bool useIBL = true; // Use IBL by default, can be disabled per-material

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
