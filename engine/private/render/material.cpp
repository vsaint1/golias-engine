#include "render/material.h"

#include "core/engine.h"
#include "graphics/shader.h"
#include "graphics/texture.h"

namespace golias {

    Ref<Material> Material::Load(CString path) {

        String content = Engine::GetInstance().GetFileSystem().LoadAssetFileText(path);

        if (content.empty()) {
            GOLIAS_LOG_ERROR("Failed to load material from path: %s", path.data());
            return nullptr;
        }

        Json json = Json::parse(content, nullptr, false);

        Ref<Material> mat = std::make_shared<Material>();
        if (json.contains("shader")) {
            auto shader = json["shader"];

            String vertexShaderPath   = shader["vertex"].get<String>();
            String fragmentShaderPath = shader["fragment"].get<String>();

            FileSystem& fileSystem = Engine::GetInstance().GetFileSystem();

            String vertexShaderSource   = fileSystem.LoadAssetFileText(vertexShaderPath);
            String fragmentShaderSource = fileSystem.LoadAssetFileText(fragmentShaderPath);

            GraphicsDevice& graphicsDevice = Engine::GetInstance().GetGraphicsDevice();

            Ref<Shader> shaderProgram = graphicsDevice.CreateShader(vertexShaderSource, fragmentShaderSource);

            if (!shaderProgram) {
                GOLIAS_LOG_ERROR("Failed to create shader program from paths: %s, %s", vertexShaderPath.data(), fragmentShaderPath.data());
                return nullptr;
            }

            mat->SetShader(shaderProgram);
        }

        if (json.contains("parameters")) {
            auto param = json["parameters"];

            if (param.contains("float")) {
                for (auto& [name, value] : param["float"].items()) {
                    mat->SetParameter(name, value.get<float>());
                }
            }

            if (param.contains("int")) {
                for (auto& [name, value] : param["int"].items()) {
                    mat->SetParameter(name, value.get<int>());
                }
            }

            if (param.contains("float2")) {
                for (auto& [name, value] : param["float2"].items()) {
                    glm::vec2 vecValue = glm::vec2(value[0].get<float>(), value[1].get<float>());
                    mat->SetParameter(name, vecValue);
                }
            }

            if (param.contains("float3")) {
                for (auto& [name, value] : param["float3"].items()) {
                    glm::vec3 vecValue = glm::vec3(value[0].get<float>(), value[1].get<float>(), value[2].get<float>());
                    mat->SetParameter(name, vecValue);
                }
            }

            if (param.contains("float4")) {
                for (auto& [name, value] : param["float4"].items()) {
                    glm::vec4 vecValue =
                        glm::vec4(value[0].get<float>(), value[1].get<float>(), value[2].get<float>(), value[3].get<float>());
                    mat->SetParameter(name, vecValue);
                }
            }

            if (param.contains("textures")) {
                for (const auto& p : param["textures"]) {
                    String name = p.value("name", "");

                    String path = p.value("path", "");

                    Ref<Texture2D> texture = Texture2D::Load(path);

                    if (!texture) {
                        GOLIAS_LOG_ERROR("Failed to load texture from path: %s", path.data());
                        continue;
                    }

                    mat->SetParameter(name, texture);
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
