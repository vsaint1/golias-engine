#pragma once

#include "font/font.h"
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
        template <
            typename T,
            typename... Args,
            typename = std::enable_if_t<std::is_same_v<Shader, T> || std::is_same_v<Model, T> || std::is_same_v<Mesh, T>
                                        || std::is_same_v<Font, T> || std::is_base_of_v<Material, T> || std::is_base_of_v<Texture2D, T>>>
        Ref<T> Load(CString path, Args&&... args) {
            if constexpr (std::is_same_v<Font, T>) {
                return LoadFont(path, std::forward<Args>(args)...);
            } else {
                const String key(path);
                auto& assets    = mAssets[key];
                const auto type = std::type_index(typeid(T));

                auto it = assets.find(type);
                if (it != assets.end()) {
                    return std::get<Ref<T>>(it->second);
                }

                Ref<T> asset = T::Load(path, std::forward<Args>(args)...);
                if (asset) {
                    assets.emplace(type, asset);
                }

                return asset;
            }
        }

        Ref<Material> LoadDefaultMaterial();

        Ref<Mesh> LoadMesh(CString modelPath, const ModelPrimitive& primitive);

        Ref<Texture2D> AcquireWhiteTexture();

        Ref<Texture2D> AcquireErrorTexture();

        Ref<Texture2D> AcquireFlatNormalTexture();

    private:
        using AssetMap = std::unordered_map<std::type_index, AssetType>;
        std::unordered_map<String, AssetMap> mAssets;

        std::unordered_map<String, FontFamily> mFonts;

        Ref<Font> LoadFont(CString path, int size);

        Ref<Material> mDefaultMaterial = nullptr;
        Ref<Texture2D> mWhiteTexture   = nullptr;
        Ref<Texture2D> mErrorTexture   = nullptr;
        Ref<Texture2D> mFlatNormalTexture = nullptr;
    };

} // namespace golias
