#include "core/renderer/base_struct.h"

TextureDesc GpuTexture::get_desc() const {
    return _tex_desc;
}

TextureFormat GpuTexture::get_format() const {
    return _format;
}

Uint32 GpuTexture::get_width() const {
    return _width;
}

Uint32 GpuTexture::get_height() const {
    return _height;
}

Uint32 GpuTexture::get_mip_levels() const {
    return _mip_levels;
}

Uint32 GpuTexture::get_id() const {
    return _id;
}

bool GpuTexture::load_from_file(const std::string& path, const TextureDesc& desc) {
    int w, h, channels;
    stbi_set_flip_vertically_on_load(true);

    unsigned char* data = nullptr;

    const bool is_hdr = stbi_is_hdr(path.c_str());

    if (is_hdr) {
        float* hdr_data = stbi_loadf(path.c_str(), &w, &h, &channels, 0);
        data            = reinterpret_cast<unsigned char*>(hdr_data);
    } else {
        data = stbi_load(path.c_str(), &w, &h, &channels, 0);
    }

    if (!data) {
        return false;
    }

    TextureDesc final_desc = desc;
    final_desc.width  = w;
    final_desc.height = h;

    if (final_desc.format == TextureFormat::UNKNOWN) {
        if (is_hdr) {
            final_desc.format = (channels == 4) ? TextureFormat::RGBA16F : TextureFormat::RGB16F;
        } else {
            switch (channels) {
            case 1:
                final_desc.format = TextureFormat::R8;
                break;
            case 2:
                final_desc.format = TextureFormat::RG8;
                break;
            case 3:
                final_desc.format = TextureFormat::RGB8;
                break;
            case 4:
                final_desc.format = TextureFormat::RGBA8;
                break;
            default:
                spdlog::error("Unsupported number of channels: {}", channels);
                stbi_image_free(data);
                break;
            }
        }
    }


    const bool success = create(data, final_desc);

    if (!success) {
        spdlog::error("Failed to create texture from file: {}", path);
    }

    stbi_image_free(data);
    return success;
}

bool GpuTexture::load_from_memory(const void* data, size_t size, const TextureDesc& desc) {
    return false;
}
