#include "render/material.h"

#include "core/engine.h"
#include "graphics/shader.h"
#include "graphics/texture.h"

namespace golias {

    Ref<Material> Material::Create(const Ref<Shader>& shader) {
        Ref<Material> material = std::make_shared<Material>();
        material->SetShader(shader);
        material->SetParameter("_BaseColor", glm::vec4(1.0f));
        material->SetParameter("_MainTexture", Engine::GetInstance().GetAssetManager().AcquireWhiteTexture());
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
        mat->SetParameter("_BaseColor", glm::vec4(1.0f));
        mat->SetParameter("_MainTexture", Engine::GetInstance().GetAssetManager().AcquireWhiteTexture());
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
                SetParameter(parameter.value("name", ""), parameter.value("value", 0.0f));
            }
        }

        if (parameters.contains("int")) {
            for (const auto& parameter : parameters["int"]) {
                SetParameter(parameter.value("name", ""), parameter.value("value", 0));
            }
        }

        if (parameters.contains("float2")) {
            for (const auto& parameter : parameters["float2"]) {
                const String name = parameter.value("name", "");
                const Json& value = parameter["value"];
                if (!value.is_array() || value.size() != 2) {
                    GOLIAS_LOG_ERROR("Invalid float2 parameter value for name: %s", name.data());
                    valid = false;
                    continue;
                }

                SetParameter(name, glm::vec2(value[0].get<float>(), value[1].get<float>()));
            }
        }

        if (parameters.contains("float3")) {
            for (const auto& parameter : parameters["float3"]) {
                const String name = parameter.value("name", "");
                const Json& value = parameter["value"];
                if (!value.is_array() || value.size() != 3) {
                    GOLIAS_LOG_ERROR("Invalid float3 parameter value for name: %s", name.data());
                    valid = false;
                    continue;
                }

                SetParameter(name, glm::vec3(value[0].get<float>(), value[1].get<float>(), value[2].get<float>()));
            }
        }

        if (parameters.contains("float4")) {
            for (const auto& parameter : parameters["float4"]) {
                const String name = parameter.value("name", "");
                const Json& value = parameter["value"];
                if (!value.is_array() || value.size() != 4) {
                    GOLIAS_LOG_ERROR("Invalid float4 parameter value for name: %s", name.data());
                    valid = false;
                    continue;
                }

                SetParameter(name, glm::vec4(value[0].get<float>(), value[1].get<float>(), value[2].get<float>(), value[3].get<float>()));
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
                    SetParameter(name, texture);
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
                        } else if constexpr (std::is_same_v<T, glm::mat3>) {
                            mShader->SetUniform(name, arg);
                        } else if constexpr (std::is_same_v<T, glm::mat4>) {
                            mShader->SetUniform(name, arg);
                        } else if constexpr (std::is_same_v<T, Ref<Texture2D>>) {
                            mShader->SetUniform(name, arg.get());
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
