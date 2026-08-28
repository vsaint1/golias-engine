#pragma once
#include "stdafx.h"

namespace golias {

    enum class TextureFormat { RGBA8, Depth24, Depth32F };
    enum class TextureFilter { Nearest, Linear };
    enum class TextureWrap { Repeat, ClampToEdge, ClampToBorder };

    struct TextureDesc {
        uint32_t Width        = 1;
        uint32_t Height       = 1;
        uint32_t Layers       = 1;
        TextureFormat Format  = TextureFormat::RGBA8;
        TextureFilter Filter  = TextureFilter::Linear;
        TextureWrap Wrap      = TextureWrap::ClampToEdge;
        glm::vec4 BorderColor = glm::vec4(1.0f);
    };

    class Texture {
    public:
        virtual ~Texture() = default;

        virtual GLuint GetHandle() const = 0;

        virtual GLenum GetTarget() const = 0;

        virtual const TextureDesc& GetDesc() const = 0;

        virtual bool Recreate(const TextureDesc& desc) = 0;
    };

} // namespace golias
