#pragma once
#include "stdafx.h"

namespace golias {

    struct TextureBinding {
        const char* Sampler;
        uint32_t Unit;
    };

    // TODO: add more text. slots
    namespace TextureSlots {
        inline constexpr TextureBinding MainTexture{"_MainTexture", 0};
        inline constexpr TextureBinding NormalMap{"_NormalMap", 1};
        inline constexpr TextureBinding ShadowMap{"_ShadowMap", 8};
        inline constexpr TextureBinding Skybox{"_Skybox", 9}; // Used by OpaquePass

        inline constexpr TextureBinding Skybox2{"_Skybox", 1}; // Used by SkyboxPass
    } // namespace TextureSlots

} // namespace golias
