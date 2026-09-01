#include "core/io/asset_manager.h"

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

} // namespace golias
