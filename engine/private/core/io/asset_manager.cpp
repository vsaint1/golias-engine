#include "core/io/asset_manager.h"

#include "graphics/texture.h"
#include "render/material.h"

namespace golias {

    Ref<Material> AssetManager::LoadDefaultMaterial() {
        if (!mDefaultMaterial) {
            mDefaultMaterial = Material::CreateDefault();
        }

        return mDefaultMaterial ? mDefaultMaterial->Clone() : nullptr;
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

} // namespace golias
