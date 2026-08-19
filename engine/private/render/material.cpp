#include "render/material.h"

#include "graphics/shader.h"

namespace golias {

    void Material::SetShader(const Ref<Shader>& shader) {
        mShader = shader;
    }

    Ref<Shader> Material::GetShader() const {
        return mShader;
    }

    void Material::Bind() const {
        if (mShader) {
            mShader->Bind();

            for (const auto& [name, value] : mParameters) {
                std::visit(
                    [&](auto&& arg) {
                        using T = std::decay_t<decltype(arg)>;
                        if constexpr (std::is_same_v<T, int>) {
                            mShader->SetUniform(name, arg);
                        } else if constexpr (std::is_same_v<T, float>) {
                            mShader->SetUniform(name, arg);
                        } else if constexpr (std::is_same_v<T, glm::vec2>) {
                            mShader->SetUniform(name, arg);
                        } else if constexpr (std::is_same_v<T, glm::vec3>) {
                            mShader->SetUniform(name, arg);
                        } else if constexpr (std::is_same_v<T, glm::vec4>) {
                            mShader->SetUniform(name, arg);
                        }
                    },
                    value);
            }
        }
    }

    void Material::SetParameter(const std::string& name, const ParamType& value) {
        mParameters[name] = value;
    }

    ParamType Material::GetParameter(const std::string& name) const {
        auto it = mParameters.find(name);
        if (it != mParameters.end()) {
            return it->second;
        }
        return {};
    }

} // namespace golias
