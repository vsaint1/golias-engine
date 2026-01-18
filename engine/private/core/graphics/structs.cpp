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

int golias::ChannelsFromTextureFormat(ETextureFormat format) {

    int channels = 0;
    switch (format) {
    case ETextureFormat::R8:
        channels = 1;
        break;
    case ETextureFormat::RG8:
        channels = 2;
        break;
    case ETextureFormat::RGB8:
        channels = 3;
        break;
    case ETextureFormat::RGBA8:
        channels = 4;
        break;
    default:
        channels = 4;
        break;
    }
    return channels;
}