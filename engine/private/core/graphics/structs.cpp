#include "core/graphics/structs.h"

namespace golias {
    ETextureFormat TextureFormatFromChannels(int channels) {
        switch (channels) {
        case 1:
            return ETextureFormat::R8;
        case 2:
            return ETextureFormat::RG8;
        case 3:
            return ETextureFormat::RGB8;
        case 4:
            return ETextureFormat::RGBA8;
        default:
            return ETextureFormat::RGBA8;
        }
    }
} // namespace golias
