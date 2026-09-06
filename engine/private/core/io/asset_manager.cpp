#include "core/io/asset_manager.h"

#include "core/engine.h"
#include "core/stl/string_utils.h"
#include "font/font.h"
#include "graphics/texture_2d.h"
#include "render/material.h"
#include "render/mesh.h"
#include "render/model.h"

namespace golias {

    Ref<Material> AssetManager::LoadDefaultMaterial() {
        if (!mDefaultMaterial) {
            mDefaultMaterial = Material::CreateDefault();
        }

        return mDefaultMaterial ? mDefaultMaterial->Clone() : nullptr;
    }

    Ref<Mesh> AssetManager::LoadMesh(CString modelPath, const ModelPrimitive& primitive) {

        String key(modelPath.size() + 128, '\0');
        const int keyLength = std::sprintf(key.data(),
                                           "%.*s#primitive:%zu:%zu:%zu:%zu",
                                           static_cast<int>(modelPath.size()),
                                           modelPath.data(),
                                           primitive.vertexOffset,
                                           primitive.vertexCount,
                                           primitive.indexOffset,
                                           primitive.indexCount);

        key.resize(static_cast<size_t>(keyLength));
        auto& assets        = mAssets[key];
        const auto type     = std::type_index(typeid(Mesh));
        const auto existing = assets.find(type);
        if (existing != assets.end()) {
            return std::get<Ref<Mesh>>(existing->second);
        }

        Ref<Model> model = Load<Model>(modelPath);
        Ref<Mesh> mesh   = model ? Mesh::Create(*model, primitive) : nullptr;

        if (mesh) {
            assets.emplace(type, mesh);
        }

        return mesh;
    }

    Ref<Mesh> AssetManager::LoadGroupMesh(CString modelPath, const std::vector<const ModelPrimitive*>& primitives) {
        if (primitives.empty()) {
            return nullptr;
        }

        std::vector<const ModelPrimitive*> sorted = primitives;
        std::sort(sorted.begin(), sorted.end(), [](const ModelPrimitive* a, const ModelPrimitive* b) {
            if (a->vertexOffset != b->vertexOffset) {
                return a->vertexOffset < b->vertexOffset;
            }

            return a->indexOffset < b->indexOffset;
        });

        // clang-format off
        String key;
        key.reserve(modelPath.size() + 128 * sorted.size());

        for (const ModelPrimitive* primitive : sorted) {
            key += String_Format("#group:%zu:%zu:%zu:%zu;", primitive->vertexOffset, primitive->vertexCount, primitive->indexOffset, primitive->indexCount);
        }
        // clang-format on

        auto& assets        = mAssets[key];
        const auto type     = std::type_index(typeid(Mesh));
        const auto existing = assets.find(type);
        if (existing != assets.end()) {
            return std::get<Ref<Mesh>>(existing->second);
        }

        Ref<Model> model = Load<Model>(modelPath);
        Ref<Mesh> mesh   = model ? Mesh::Create(*model, sorted) : nullptr;

        if (mesh) {
            assets.emplace(type, mesh);
        }

        return mesh;
    }

    Ref<Material> AssetManager::LoadModelMaterial(CString modelPath, int materialIndex) {

        const String key = String_Format("%.*s#material:%d", static_cast<int>(modelPath.size()), modelPath.data(), materialIndex);

        auto& assets        = mAssets[key];
        const auto type     = std::type_index(typeid(Material));
        const auto existing = assets.find(type);
        if (existing != assets.end()) {
            return std::get<Ref<Material>>(existing->second);
        }

        Ref<Model> model = Load<Model>(modelPath);
        if (!model || materialIndex < 0 || materialIndex >= static_cast<int>(model->GetMaterials().size())) {
            return LoadDefaultMaterial();
        }

        const ModelMaterial& definition = model->GetMaterials()[materialIndex];
        Ref<Material> material          = LoadDefaultMaterial();
        material->SetParameterValue("_BaseColor", definition.baseColor);

        const auto bind_texture = [&](const String& path, const ModelTextureData& data, CString parameter) {
            if (!path.empty()) {

                Ref<Texture2D> texture = Load<Texture2D>(path);
                if (texture) {
                    material->SetParameterValue(parameter, texture);
                    return;
                }

                GOLIAS_LOG_WARN("Failed to load texture: %s", path.c_str());

                if (parameter == "_MainTexture") {
                    material->SetParameterValue(parameter, AcquireErrorTexture());
                }

                return;
            }

            if (data.IsValid()) {

                const String textureKeyStr = String_Format("%s#texture:%d:%s", modelPath.data(), materialIndex, parameter);

                Ref<Texture2D> texture = AcquireEmbeddedTexture(textureKeyStr, data.Pixels, data.Width, data.Height, data.Components);

                if (texture) {
                    material->SetParameterValue(parameter, texture);
                }
            }
        };

        bind_texture(definition.baseColorTexture, definition.baseColorData, "_MainTexture");
        bind_texture(definition.normalTexture, definition.normalData, "_NormalMap");

        assets.emplace(type, material);
        return material;
    }

    Ref<Font> AssetManager::LoadFont(CString path, int size) {
        if (path.empty() || size <= 0) {
            GOLIAS_LOG_ERROR("Invalid font request. Path: '%s', size: %d", std::string(path).c_str(), size);
            return nullptr;
        }

        FontFamily& family = mFonts[String(path)];
        if (const auto it = family.find(size); it != family.end()) {
            return it->second;
        }

        Ref<Font> font = Font::Load(path, size);
        if (font) {
            family.emplace(size, font);
        }

        return font;
    }

    Ref<Texture2D> AssetManager::AcquireWhiteTexture() {
        if (!mWhiteTexture) {
            unsigned char white[] = {255, 255, 255, 255};
            mWhiteTexture         = std::make_shared<Texture2D>(1, 1, 4, white);
        }

        return mWhiteTexture;
    }

    Ref<Texture2D> AssetManager::AcquireErrorTexture() {
        if (!mErrorTexture) {
            unsigned char magenta[] = {255, 0, 255, 255};
            mErrorTexture           = std::make_shared<Texture2D>(1, 1, 4, magenta);
        }

        return mErrorTexture;
    }

    Ref<Texture2D> AssetManager::AcquireFlatNormalTexture() {
        if (!mFlatNormalTexture) {
            unsigned char flatNormal[] = {128, 128, 255};
            mFlatNormalTexture         = std::make_shared<Texture2D>(1, 1, 3, flatNormal);
        }

        return mFlatNormalTexture;
    }

    Ref<Texture2D> AssetManager::AcquireEmbeddedTexture(CString key, const std::vector<unsigned char>& pixels, int32_t width, int32_t height, int32_t components) {

        if (pixels.empty() || width <= 0 || height <= 0) {
            return nullptr;
        }

        String cacheKey = String_Format("#embedded:%s", key.data());

        auto& assets        = mAssets[cacheKey];
        const auto type     = std::type_index(typeid(Texture2D));
        const auto existing = assets.find(type);
        if (existing != assets.end()) {
            return std::get<Ref<Texture2D>>(existing->second);
        }

        Ref<Texture2D> texture = std::make_shared<Texture2D>(width, height, components, pixels.data());
        if (texture) {
            assets.emplace(type, texture);
        }

        return texture;
    }

} // namespace golias
