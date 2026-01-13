#include "core/graphics/material.h"

#include "core/engine.h"
#include <stdafx.h>

namespace golias {

    static std::string GenerateMaterialCacheKey(const std::string_view pPath, const Json* paramOverrides) {
        if (!paramOverrides || paramOverrides->empty()) {
            return std::string(pPath);
        }

        std::string key = std::string(pPath) + ":";

        std::string paramStr = paramOverrides->dump();
        key += std::to_string(std::hash<std::string>{}(paramStr));

        return key;
    }

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

    bool Material::UseImageBasedLighting() const {
        return useIBL;
    }


    std::shared_ptr<Material> Material::Load(const std::string_view pPath, const Json* paramOverrides) {

        std::string cacheKey = GenerateMaterialCacheKey(pPath, paramOverrides);

        if (auto existingMaterial = Engine::GetInstance().GetMaterialManager().GetMaterial(cacheKey); existingMaterial) {
            spdlog::info("Material::Load: Using cached material: {} (cache key: {})", pPath, cacheKey);
            return existingMaterial;
        }

        auto& fs = Engine::GetInstance().GetFileSystem();

        std::string content = fs.LoadAssetFileText(pPath);

        if (content.empty()) {
            spdlog::error("Material::Load: Failed to load Material file: {}", pPath);
            return nullptr;
        }

        Json json = Json::parse(content);

        auto material = std::make_shared<Material>();

        auto rd = Engine::GetInstance().GetSceneRenderer().GetRenderingDevice();

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

        } else {
            spdlog::info("Material::Load: No shader specified in Material: {}, using default shader.", pPath);
            material->SetShader(rd->GetDefaultShader3D());
        }

        material->SetParameter("TEXTURE_FLAGS", 0); // No textures by default
        material->SetParameter("u_material.modulate", glm::vec4(1.0f));
        material->SetParameter("u_material.metallicFactor", 0.0f);
        material->SetParameter("u_material.roughnessFactor", 1.0f);
        material->SetParameter("u_material.emissiveFactor", glm::vec3(0.0f));
        material->SetParameter("u_material.emissiveStrength", 1.0f);

        material->SetBlendMode(EBlendMode::BLEND_MODE_OPAQUE);
        material->SetDepthTestEnabled(true);
        material->SetDepthWriteEnabled(true);
        material->SetDepthFunc(EComparisonFunc::COMPARISON_LESS);
        material->SetCullMode(ECullMode::CULL_BACK);
        material->SetAlphaClipThreshold(0.5f);

        if (json.contains("blend_mode")) {
            std::string blendModeStr = json["blend_mode"].get<std::string>();

            if (blendModeStr == "opaque") {
                material->SetBlendMode(EBlendMode::BLEND_MODE_OPAQUE);
                material->SetDepthWriteEnabled(true);
            } else if (blendModeStr == "alpha" || blendModeStr == "alpha_blend") {
                material->SetBlendMode(EBlendMode::BLEND_MODE_ALPHA);
                material->SetDepthWriteEnabled(false);
            } else if (blendModeStr == "additive") {
                material->SetBlendMode(EBlendMode::BLEND_MODE_ADDITIVE);
                material->SetDepthWriteEnabled(false);
            } else if (blendModeStr == "multiply") {
                material->SetBlendMode(EBlendMode::BLEND_MODE_MULTIPLY);
                material->SetDepthWriteEnabled(false);
            }
        }

        if (json.contains("alpha_clip_threshold")) {
            material->SetAlphaClipThreshold(json["alpha_clip_threshold"].get<float>());
        }

        if (json.contains("depth_test")) {
            material->SetDepthTestEnabled(json["depth_test"].get<bool>());
        }

        if (json.contains("depth_write")) {
            material->SetDepthWriteEnabled(json["depth_write"].get<bool>());
        }

        if (json.contains("depth_func")) {
            std::string depthFuncStr = json["depth_func"].get<std::string>();
            if (depthFuncStr == "never") {
                material->SetDepthFunc(EComparisonFunc::COMPARISON_NEVER);
            } else if (depthFuncStr == "less") {
                material->SetDepthFunc(EComparisonFunc::COMPARISON_LESS);
            } else if (depthFuncStr == "equal") {
                material->SetDepthFunc(EComparisonFunc::COMPARISON_EQUAL);
            } else if (depthFuncStr == "less_equal") {
                material->SetDepthFunc(EComparisonFunc::COMPARISON_LESS_EQUAL);
            } else if (depthFuncStr == "greater") {
                material->SetDepthFunc(EComparisonFunc::COMPARISON_GREATER);
            } else if (depthFuncStr == "not_equal") {
                material->SetDepthFunc(EComparisonFunc::COMPARISON_NOT_EQUAL);
            } else if (depthFuncStr == "greater_equal") {
                material->SetDepthFunc(EComparisonFunc::COMPARISON_GREATER_EQUAL);
            } else if (depthFuncStr == "always") {
                material->SetDepthFunc(EComparisonFunc::COMPARISON_ALWAYS);
            }
        }

       
        if (json.contains("cull_mode")) {
            std::string cullModeStr = json["cull_mode"].get<std::string>();
            if (cullModeStr == "none") {
                material->SetCullMode(ECullMode::CULL_NONE);
            } else if (cullModeStr == "front") {
                material->SetCullMode(ECullMode::CULL_FRONT);
            } else if (cullModeStr == "back") {
                material->SetCullMode(ECullMode::CULL_BACK);
            }
        }

        if (json.contains("use_ibl")) {
            material->SetUseIBL(json["use_ibl"].get<bool>());
        }

        ParseParameters(material, json);

        if (paramOverrides && !paramOverrides->empty()) {
            spdlog::debug("Material::Load: Applying parameter overrides: {}", paramOverrides->dump());

            Json j;
            j["parameters"] = *paramOverrides;
            ParseParameters(material, j);
        }

        spdlog::info("Material::Load: Successfully loaded Material: {} (cache key: {})", pPath, cacheKey);

        Engine::GetInstance().GetMaterialManager().RegisterMaterial(cacheKey, material);

        return material;
    }

    void Material::ParseParameters(std::shared_ptr<Material>& material, const nlohmann::json& json) {


        if (json.contains("parameters")) {

            Json paramsObj = json["parameters"];

            if (paramsObj.contains("textures")) {

                Json texturesArray         = paramsObj["textures"];
                ETextureFlags textureFlags = ETextureFlags::NONE;

                for (const auto& texEntry : texturesArray) {
                    std::string name  = texEntry.value("name", "");
                    std::string value = texEntry.value("path", "");

                    auto rd      = Engine::GetInstance().GetSceneRenderer().GetRenderingDevice();
                    auto texture = rd->CreateTextureFromFile(value);

                    if (texture) {
                        material->SetParameter<std::shared_ptr<Texture2D>>(name, texture);

                        if (name == "ALBEDO_TEXTURE") {
                            textureFlags |= ETextureFlags::HAS_ALBEDO;
                        } else if (name == "METALLIC_TEXTURE") {
                            textureFlags |= ETextureFlags::HAS_METALLIC;
                        } else if (name == "ROUGHNESS_TEXTURE") {
                            textureFlags |= ETextureFlags::HAS_ROUGHNESS;
                        } else if (name == "NORMAL_TEXTURE") {
                            textureFlags |= ETextureFlags::HAS_NORMAL;
                        } else if (name == "AO_TEXTURE") {
                            textureFlags |= ETextureFlags::HAS_AO;
                        } else if (name == "EMISSIVE_TEXTURE") {
                            textureFlags |= ETextureFlags::HAS_EMISSIVE;
                        }
                    } else {
                        spdlog::error("Material::Load: Failed to load Texture2D '{}'", value);
                    }
                }

                material->SetParameter("TEXTURE_FLAGS", static_cast<int>(textureFlags));
            }

            if (paramsObj.contains("bools")) {
                Json boolsArray = paramsObj["bools"];

                for (const auto& boolEntry : boolsArray) {
                    std::string name = boolEntry.value("name", "");
                    bool value       = boolEntry.value("value", false);

                    material->SetParameter(name, value);
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

                    const auto& vecObj = vecEntry["value"];
                    float x            = vecObj.value("x", 0.0f);
                    float y            = vecObj.value("y", 0.0f);
                    float z            = vecObj.value("z", 0.0f);
                    float w            = vecObj.value("w", 0.0f);

                    if (type == "vec2") {
                        material->SetParameter<glm::vec2>(name, glm::vec2(x, y));
                    } else if (type == "vec3") {
                        material->SetParameter<glm::vec3>(name, glm::vec3(x, y, z));
                    } else if (type == "vec4") {
                        material->SetParameter<glm::vec4>(name, glm::vec4(x, y, z, w));

                        if (name == "MODULATE") {
                            material->SetParameter("u_material.modulate", glm::vec4(x, y, z, w));
                        }

                    } else {
                        spdlog::warn("Material::Load: Unknown vector type '{}' for '{}'", type, name);
                    }
                }
            }
        }
    }

    MaterialManager& MaterialManager::GetInstance() {
        static MaterialManager instance;
        return instance;
    }

    std::shared_ptr<Material> MaterialManager::GetMaterial(const std::string_view pPath) {
        auto it = materials.find(pPath.data());
        if (it != materials.end()) {
            return it->second;
        }

        return nullptr;
    }

    void MaterialManager::RegisterMaterial(const std::string_view pPath, const std::shared_ptr<Material>& pMaterial) {
        materials[pPath.data()] = pMaterial;
    }


} // namespace golias
