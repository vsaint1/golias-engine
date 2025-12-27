#pragma once

#include "shader.h"

namespace golias {

    class Material {
    public:

        void Activate();

        void SetShader(const std::shared_ptr<Shader>& pShader);
        std::shared_ptr<Shader> GetShader() const;

        template<typename T>
        void SetParameter(const std::string_view pName, const T& value) {
            parameters[std::string(pName)] = value;
        }

        static std::shared_ptr<Material> Load(const std::string& pPath);

    private:
        std::shared_ptr<Shader> shader;

        std::unordered_map<std::string, UniformValue> parameters;
    };
    
} // namespace golias
