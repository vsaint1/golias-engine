#include "core/graphics/material.h"

#include "core/engine.h"
#include <stdafx.h>

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


    std::shared_ptr<Material> Material::Load(const std::string& pPath) {

        auto& fs = Engine::GetInstance().GetFileSystem();

        std::string content = fs.LoadAssetFileText(pPath);

        if (content.empty()) {
            spdlog::error("Material::Load: Failed to load Material file: {}", pPath);
            return nullptr;
        }

        Json json = Json::parse(content);

        auto material = std::make_shared<Material>();

        auto rd = golias::Engine::GetInstance().GetRenderingDevice();

        if (json.contains("shader")) {

            Json shaderObj = json["shader"];

            std::string vertexPath   = shaderObj.value("vertex", "");
            std::string fragmentPath = shaderObj.value("fragment", "");

            if (vertexPath.empty() || fragmentPath.empty()) {
                spdlog::error("Material::Load: Shader paths missing in Material: {}", pPath);
                return nullptr;
            }

            std::string vertexSource   = fs.LoadAssetFileText(vertexPath);
            std::string fragmentSource = fs.LoadAssetFileText(fragmentPath);

            if (vertexSource.empty() || fragmentSource.empty()) {
                spdlog::error("Material::Load: Failed to load Shader sources for Material: {}", pPath);
                return nullptr;
            }

            auto shader = rd->CreateShaderFromSource(vertexSource, fragmentSource);

            if (shader) {
                material->SetShader(shader);
            } else {
                spdlog::error("Material::Load: Failed to create Shader for Material: {}", pPath);
                return nullptr;
            }

        }


        if (json.contains("parameters")) {

            Json paramsObj = json["parameters"];

            if (paramsObj.contains("textures")) {

                Json texturesArray = paramsObj["textures"];

                for (const auto& texEntry : texturesArray) {
                    std::string name  = texEntry.value("name", "");
                    std::string value = texEntry.value("path", "");

                    auto texture = rd->CreateTextureFromFile(value);

                    if (texture) {
                        material->SetParameter<std::shared_ptr<Texture2D>>(name, texture);
                    } else {
                        spdlog::error("Material::Load: Failed to load Texture2D '{}' for Material: {}", value, pPath);
                    }

                    spdlog::info("Material::Load: Loaded Texture2D '{}' for Material: {}", value, pPath);
                }
            }

            if (paramsObj.contains("floats")) {

                Json floatsArray = paramsObj["floats"];

                for (const auto& floatEntry : floatsArray) {
                    std::string name = floatEntry.value("name", "");
                    float value      = floatEntry.value("value", 0.0f);

                    material->SetParameter<float>(name, value);
                }
            }

            if (paramsObj.contains("vectors")) {
                Json vectors = paramsObj["vectors"];

                for (const auto& vecEntry : vectors) {
                    std::string name = vecEntry.value("name", "");
                    std::string type = vecEntry.value("type", "vec2"); // DEFAULT -> 2D

                    float x = vecEntry.value("x", 0.0f);
                    float y = vecEntry.value("y", 0.0f);
                    float z = vecEntry.value("z", 0.0f);
                    float w = vecEntry.value("w", 0.0f);

                    if (type == "vec2") {
                        material->SetParameter<glm::vec2>(name, glm::vec2(x, y));
                    } else if (type == "vec3") {
                        material->SetParameter<glm::vec3>(name, glm::vec3(x, y, z));
                    } else if (type == "vec4") {
                        material->SetParameter<glm::vec4>(name, glm::vec4(x, y, z, w));
                    } else {
                        spdlog::warn("Material::Load: Unknown vector type '{}' for '{}'", type, name);
                    }
                }
            }
        }

        spdlog::info("Material::Load: Successfully loaded Material: {}", pPath);

        return material;
    }

} // namespace golias
