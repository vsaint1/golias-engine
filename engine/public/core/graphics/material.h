#pragma once

#include "shader.h"

namespace golias {

    class Material {
    public:

        void Activate();

        void SetShader(const std::shared_ptr<Shader>& pShader);
        std::shared_ptr<Shader> GetShader() const;

        void SetParameter(const std::string_view pName, UniformValue& value);

    private:
        std::shared_ptr<Shader> shader;

        std::unordered_map<std::string, UniformValue> parameters;
    };
    
} // namespace golias
