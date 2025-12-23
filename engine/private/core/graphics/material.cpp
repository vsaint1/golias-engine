#include "core/graphics/material.h"


namespace golias {

    void Material::SetShader(const std::shared_ptr<Shader>& pShader) {
        shader = pShader;
    }


    void Material::Activate() {
        if (shader) {
            shader->Bind();
        }

        for (auto& [name, value] : parameters) {
            shader->SetUniform(name, value);
        }
    }

    std::shared_ptr<Shader> Material::GetShader() const {
        return shader;
    }

    void Material::SetParameter(const std::string_view pName, UniformValue& value) {
        parameters[std::string(pName)] = value;
    }
} // namespace golias
