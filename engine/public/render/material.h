#pragma once

#include "stdafx.h"

namespace golias {

    class Shader;

    // TODO: Matrices, Textures, etc.
    using ParamType = std::variant<int, float, glm::vec2, glm::vec3, glm::vec4>;

    class Material {
    public:

        void SetShader(const Ref<Shader>& shader);

        Ref<Shader> GetShader() const;

        void Bind() const;

        void SetParameter(const std::string& name, const ParamType& value);

        ParamType GetParameter(const std::string& name) const;

    private:
        Ref<Shader> mShader = nullptr;

        std::unordered_map<std::string, ParamType> mParameters = {};
    };
} // namespace golias
