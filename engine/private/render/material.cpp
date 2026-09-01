#include "render/material.h"

#include "core/engine.h"
#include "graphics/shader.h"
#include "graphics/texture_2d.h"

namespace golias {

    Ref<Material> Material::Create(const Ref<Shader>& shader) {
        Ref<Material> material = std::make_shared<Material>();
        material->SetShader(shader);
        material->SetParameterValue("_BaseColor", glm::vec4(1.0f));
        material->SetParameterValue("_MainTexture", Engine::GetInstance().GetAssetManager().AcquireWhiteTexture());
        material->SetParameterValue("_NormalMap", Engine::GetInstance().GetAssetManager().AcquireFlatNormalTexture());
        return material;
    }

    Ref<Material> Material::CreateDefault() {
        Ref<Shader> shader = Engine::GetInstance().GetAssetManager().Load<Shader>("shaders/default.gshader");

        return shader ? Create(shader) : nullptr;
    }

    Ref<Material> Material::Clone() const {
        Ref<Material> material = std::make_shared<Material>();
        material->mShader      = mShader;
        material->mParameters  = mParameters;
        return material;
    }

    Ref<Material> Material::Load(CString path) {

        String content = Engine::GetInstance().GetFileSystem().LoadAssetFileText(path);

        if (content.empty()) {
            GOLIAS_LOG_ERROR("Failed to load material from path: %s", path.data());
            return nullptr;
        }

        Json json = Json::parse(content, nullptr, false);

        Ref<Material> mat = std::make_shared<Material>();
        mat->SetParameterValue("_BaseColor", glm::vec4(1.0f));
        mat->SetParameterValue("_MainTexture", Engine::GetInstance().GetAssetManager().AcquireWhiteTexture());
        mat->SetParameterValue("_NormalMap", Engine::GetInstance().GetAssetManager().AcquireFlatNormalTexture());
        if (json.contains("shader")) {
            const String shaderPath   = json["shader"].get<String>();
            Ref<Shader> shaderProgram = Engine::GetInstance().GetAssetManager().Load<Shader>(shaderPath);

            if (!shaderProgram) {
                GOLIAS_LOG_ERROR("Failed to load shader: %s", shaderPath.data());
                return nullptr;
            }

            mat->SetShader(shaderProgram);
        }

        if (json.contains("parameters")) {
            mat->ApplyParametersFromJson(json["parameters"]);
        }

        if (json.contains("render_state") && json["render_state"].is_object()) {
            const Json& state = json["render_state"];

            if (state.contains("blend")) {
                const String blend = state["blend"].get<String>();
                if (blend == "alpha") {
                    mat->SetBlendMode(BlendMode::Alpha);
                } else if (blend == "additive") {
                    mat->SetBlendMode(BlendMode::Additive);
                } else if (blend == "multiply") {
                    mat->SetBlendMode(BlendMode::Multiply);
                } else {
                    mat->SetBlendMode(BlendMode::None);
                }
            }

            if (state.contains("depth_test")) {
                mat->SetDepthTestEnabled(state["depth_test"].get<bool>());
            }

            if (state.contains("depth_write")) {
                mat->SetDepthWriteEnabled(state["depth_write"].get<bool>());
            }

            if (state.contains("cull")) {
                const String cull = state["cull"].get<String>();
                if (cull == "front") {
                    mat->SetCullMode(CullMode::Front);
                } else if (cull == "back") {
                    mat->SetCullMode(CullMode::Back);
                } else if (cull == "front_and_back") {
                    mat->SetCullMode(CullMode::FrontAndBack);
                } else {
                    mat->SetCullMode(CullMode::None);
                }
            }
        }

        GOLIAS_LOG_INFO("Material loaded successfully from path: %s", path.data());

        return mat;
    }

    bool Material::ApplyParametersFromJson(const Json& parameters) {
        if (!parameters.is_object()) {
            GOLIAS_LOG_ERROR("Material parameters must be an object.");
            return false;
        }

        bool valid = true;

        if (parameters.contains("float")) {
            for (const auto& parameter : parameters["float"]) {
                SetParameterValue(parameter.value("name", ""), parameter.value("value", 0.0f));
            }
        }

        if (parameters.contains("int")) {
            for (const auto& parameter : parameters["int"]) {
                SetParameterValue(parameter.value("name", ""), parameter.value("value", 0));
            }
        }

        if (parameters.contains("float2")) {
            for (const auto& parameter : parameters["float2"]) {
                const String name = parameter.value("name", "");
                const Json& value = parameter.value("value", Json::object());
                const float r     = value.contains("r") ? value.value("r", 1.0f) : value.value("x", 1.0f);
                const float g     = value.contains("g") ? value.value("g", 1.0f) : value.value("y", 1.0f);

                glm::vec2 vecValue = glm::vec2(r, g);

                SetParameterValue(name, vecValue);
            }
        }

        if (parameters.contains("float3")) {
            for (const auto& parameter : parameters["float3"]) {
                const String name = parameter.value("name", "");
                const Json& value = parameter.value("value", Json::object());
                const float r     = value.contains("r") ? value.value("r", 1.0f) : value.value("x", 1.0f);
                const float g     = value.contains("g") ? value.value("g", 1.0f) : value.value("y", 1.0f);
                const float b     = value.contains("b") ? value.value("b", 1.0f) : value.value("z", 1.0f);

                glm::vec3 vecValue = glm::vec3(r, g, b);

                SetParameterValue(name, vecValue);
            }
        }

        if (parameters.contains("float4")) {
            for (const auto& parameter : parameters["float4"]) {
                const String name = parameter.value("name", "");
                const Json& value = parameter.value("value", Json::object());

                const float r = value.contains("r") ? value.value("r", 1.0f) : value.value("x", 1.0f);
                const float g = value.contains("g") ? value.value("g", 1.0f) : value.value("y", 1.0f);
                const float b = value.contains("b") ? value.value("b", 1.0f) : value.value("z", 1.0f);
                const float a = value.contains("a") ? value.value("a", 1.0f) : value.value("w", 1.0f);

                glm::vec4 vecValue = glm::vec4(r, g, b, a);

                SetParameterValue(name, vecValue);
            }
        }

        if (parameters.contains("textures")) {
            for (const auto& parameter : parameters["textures"]) {
                const String name      = parameter.value("name", "");
                const String path      = parameter.value("path", "");
                Ref<Texture2D> texture = Engine::GetInstance().GetAssetManager().Load<Texture2D>(path);
                if (!texture) {
                    GOLIAS_LOG_ERROR("Failed to load texture from path: %s", path.data());
                    texture = Engine::GetInstance().GetAssetManager().AcquireErrorTexture();
                }

                if (texture) {
                    SetParameterValue(name, texture);
                }
            }
        }

        return valid;
    }

    void Material::SetShader(const Ref<Shader>& shader) {
        mShader = shader;
    }

    Ref<Shader> Material::GetShader() const {
        return mShader;
    }

    void Material::SetRenderState(const RenderState& state) {
        mRenderState = state;
    }

    const RenderState& Material::GetRenderState() const {
        return mRenderState;
    }

    void Material::SetBlendMode(BlendMode mode) {
        mRenderState.Blend = mode;
    }

    void Material::SetCullMode(CullMode mode) {
        mRenderState.Cull = mode;
    }

    void Material::SetDepthTestEnabled(bool enabled) {
        mRenderState.DepthTest = enabled;
    }

    void Material::SetDepthWriteEnabled(bool enabled) {
        mRenderState.DepthWrite = enabled;
    }


    void Material::Bind() const {
        if (!mShader) {
            return;
        }

        GraphicsDevice& device = Engine::GetInstance().GetGraphicsDevice();
        device.SetBlendMode(mRenderState.Blend);
        device.SetCullMode(mRenderState.Cull);
        device.SetDepthTestEnabled(mRenderState.DepthTest);
        device.SetDepthWriteEnabled(mRenderState.DepthWrite);

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
                    } else if constexpr (std::is_same_v<T, glm::mat3>) {
                        mShader->SetUniform(name, arg);
                    } else if constexpr (std::is_same_v<T, glm::mat4>) {
                        mShader->SetUniform(name, arg);
                    } else if constexpr (std::is_same_v<T, Ref<Texture>>) {
                        mShader->SetUniform(name, arg.get());
                    }
                },
                value);
        }
    }

    void Material::SetParameterValue(CString name, const ParamType& value) {
        mParameters[name.data()] = value;
    }

    ParamType Material::GetParameter(CString name) const {
        auto it = mParameters.find(name.data());

        if (it != mParameters.end()) {
            return it->second;
        }

        return {};
    }


} // namespace golias
