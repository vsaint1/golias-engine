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
        inline constexpr TextureBinding ShadowMap{"_ShadowMap", 8};
    } // namespace TextureSlots

} // namespace golias
