#pragma once

#include "stdafx.h"
#include <any>
#include <typeindex>

namespace golias {

    class Material;
    class Shader;
    class Texture2D;
    class Model;
    class Mesh;
    struct ModelPrimitive;

    using AssetType = std::variant<Ref<Material>, Ref<Shader>, Ref<Texture2D>, Ref<Model>, Ref<Mesh>>;

    class AssetManager {
    public:
        template <typename T, typename = std::enable_if_t<std::is_same_v<Shader, T> || std::is_same_v<Model, T>
                                                            || std::is_same_v<Mesh, T> || std::is_base_of_v<Material, T>
                                                            || std::is_base_of_v<Texture2D, T>>>
        Ref<T> Load(CString path) {
            const String key(path);
            auto& assets    = mAssets[key];
            const auto type = std::type_index(typeid(T));

            auto it = assets.find(type);
            if (it != assets.end()) {
                return std::get<Ref<T>>(it->second);
            }

            Ref<T> asset = T::Load(path);
            if (asset) {
                assets.emplace(type, asset);
            }

            return asset;
        }

        Ref<Material> LoadDefaultMaterial();

        Ref<Mesh> LoadMesh(CString modelPath, const ModelPrimitive& primitive);

        Ref<Texture2D> AcquireWhiteTexture();

        Ref<Texture2D> AcquireErrorTexture();

    private:
        using AssetMap = std::unordered_map<std::type_index, AssetType>;
        std::unordered_map<String, AssetMap> mAssets;

        Ref<Material> mDefaultMaterial = nullptr;
        Ref<Texture2D> mWhiteTexture   = nullptr;
        Ref<Texture2D> mErrorTexture   = nullptr;
    };

} // namespace golias
