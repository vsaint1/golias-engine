#pragma once

#include "graphics/render_types.h"
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

        void SetParameterValue(CString name, const ParamType& value);

        bool ApplyParametersFromJson(const Json& parameters);

        ParamType GetParameter(CString name) const;

        const RenderState& GetRenderState() const;
        void SetRenderState(const RenderState& state);

        void SetBlendMode(BlendMode mode);

        void SetCullMode(CullMode mode);

        void SetDepthTestEnabled(bool enabled);

        void SetDepthWriteEnabled(bool enabled);

    private:
        Ref<Shader> mShader = nullptr;

        RenderState mRenderState = {};

        std::unordered_map<std::string, ParamType> mParameters = {};
    };


} // namespace golias
