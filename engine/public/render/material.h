#pragma once

#include "stdafx.h"

namespace golias {

    class Shader;

    class Texture;

    using ParamType = std::variant<int, float, glm::vec2, glm::vec3, glm::vec4, glm::mat3, glm::mat4, Ref<Texture>>;

    class Material {
    public:
        Material() = default;

        static Ref<Material> Load(CString path);

        static Ref<Material> Create(const Ref<Shader>& shader);

        static Ref<Material> CreateDefault();

        Ref<Material> Clone() const;

        void SetShader(const Ref<Shader>& shader);

        Ref<Shader> GetShader() const;

        void Bind() const;

        void SetParameter(const std::string& name, const ParamType& value);

        bool ApplyParametersFromJson(const Json& parameters);

        ParamType GetParameter(const std::string& name) const;

    private:
        Ref<Shader> mShader = nullptr;

        std::unordered_map<std::string, ParamType> mParameters = {};
    };


} // namespace golias
