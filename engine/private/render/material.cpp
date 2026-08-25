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
            const String shaderPath = json["shader"].get<String>();
            Ref<Shader> shaderProgram = Engine::GetInstance().GetAssetManager().Load<Shader>(shaderPath);

            if (!shaderProgram) {
                GOLIAS_LOG_ERROR("Failed to load shader: %s", shaderPath.data());
                return nullptr;
            }

            mat->SetShader(shaderProgram);
        }

        if (json.contains("parameters")) {
            const auto& param = json["parameters"];

            if (param.contains("float")) {

                for(const auto& p : param["float"]) {
                    String name  = p.value("name", "");
                    float value  = p.value("value", 0.0f);

                    mat->SetParameter(name, value);
                }
            }

            if (param.contains("int")) {
           
                for(const auto& p : param["int"]) {
                    String name  = p.value("name", "");
                    int value    = p.value("value", 0);

                    mat->SetParameter(name, value);
                }

            }

            if (param.contains("float2")) {
             
                for (const auto& p : param["float2"]) {
                    String name       = p.value("name", "");
                    const auto& value = p["value"];

                    if (value.is_array() && value.size() == 2) {
                        glm::vec2 vecValue(value[0].get<float>(), value[1].get<float>());

                        mat->SetParameter(name, vecValue);
                    } else {
                        GOLIAS_LOG_ERROR("Invalid float2 parameter value for name: %s", name.data());
                    }
                }

            }

            if (param.contains("float3")) {

                for (const auto& p : param["float3"]) {
                    String name       = p.value("name", "");
                    const auto& value = p["value"];

                    if (value.is_array() && value.size() == 3) {
                        glm::vec3 vecValue(value[0].get<float>(), value[1].get<float>(), value[2].get<float>());

                        mat->SetParameter(name, vecValue);
                    } else {
                        GOLIAS_LOG_ERROR("Invalid float3 parameter value for name: %s", name.data());
                    }
                }

            }

            if (param.contains("float4")) {

                for (const auto& p : param["float4"]) {
                    String name       = p.value("name", "");
                    const auto& value = p["value"];

                    if (value.is_array() && value.size() == 4) {
                        glm::vec4 vecValue(value[0].get<float>(), value[1].get<float>(), value[2].get<float>(), value[3].get<float>());

                        mat->SetParameter(name, vecValue);
                    } else {
                        GOLIAS_LOG_ERROR("Invalid float4 parameter value for name: %s", name.data());
                    }
                }

            }

            if (param.contains("textures")) {
                
                for (const auto& p : param["textures"]) {
                    String name = p.value("name", "");

                    String path = p.value("path", "");

                    Ref<Texture2D> texture = Engine::GetInstance().GetAssetManager().Load<Texture2D>(path);

                    if (!texture) {
                        GOLIAS_LOG_ERROR("Failed to load texture from path: %s", path.data());
                        texture = Engine::GetInstance().GetAssetManager().AcquireErrorTexture();
                    }

                    if (texture) {
                        mat->SetParameter(name, texture);
                    } else {
                        continue;
                    }
                }

            }
        }

        GOLIAS_LOG_INFO("Material loaded successfully from path: %s", path.data());

        return mat;
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
