#pragma once

#include "texture_2d.h"
#include <glm/geometric.hpp>

namespace golias {

    class TextureCubemap {
    public:
        virtual ~TextureCubemap() = default;

        Uint32 GetWidth() const;
        Uint32 GetHeight() const;

        Uint32 GetNativeHandle() const;
        Uint32 GetNumChannels() const;

        ETextureFormat GetFormat() const;

        bool IsValid() const;

        static std::shared_ptr<TextureCubemap> Load(const std::string_view pFilePath);
        static std::shared_ptr<TextureCubemap> Load(const std::array<std::string, 6>& faces);
        static std::shared_ptr<TextureCubemap> LoadProcedural();

    protected:
        TextureCubemap() = default;

        glm::vec3 GetCubemapDirection(int face, float u, float v);
        glm::vec2 DirectionToEquirectangularUV(const glm::vec3& dir);

        template <typename T>
        void SampleAndCopyPixels(T* src, T* dst, int srcWidth, int srcHeight, int faceSize, int channels, int face);

        template <typename T>
        void CopyFaceData(
            T* src, T* dst, int srcWidth, int faceSize, int channels, int faceX, int faceY, bool flipVertical, bool flipHorizontal);

        Uint32 width          = 0;
        Uint32 height         = 0;
        Uint32 handle         = 0;
        int channels          = 0;
        ETextureFormat format = ETextureFormat::RGBA8;
        bool sRGB             = false;
    };


    template <typename T>
    void TextureCubemap::SampleAndCopyPixels(T* src, T* dst, int srcWidth, int srcHeight, int faceSize, int channels, int face) {
        for (int y = 0; y < faceSize; y++) {
            for (int x = 0; x < faceSize; x++) {
                float u = (2.0f * x / (faceSize - 1)) - 1.0f;
                float v = (2.0f * y / (faceSize - 1)) - 1.0f;

                glm::vec3 dir = glm::normalize(GetCubemapDirection(face, u, v));
                glm::vec2 uv  = DirectionToEquirectangularUV(dir);

                int srcX = static_cast<int>(uv.x * (srcWidth - 1));
                int srcY = static_cast<int>(uv.y * (srcHeight - 1));

                int srcIdx = (srcY * srcWidth + srcX) * channels;
                int dstIdx = (y * faceSize + x) * channels;

                for (int c = 0; c < channels; c++) {
                    dst[dstIdx + c] = src[srcIdx + c];
                }
            }
        }
    }

    template <typename T>
    void TextureCubemap::CopyFaceData(
        T* src, T* dst, int srcWidth, int faceSize, int channels, int faceX, int faceY, bool flipVertical, bool flipHorizontal) {
        int startX = faceX * faceSize;
        int startY = faceY * faceSize;

        for (int y = 0; y < faceSize; y++) {
            for (int x = 0; x < faceSize; x++) {
                int srcY   = startY + (flipVertical ? (faceSize - 1 - y) : y);
                int srcX   = startX + (flipHorizontal ? (faceSize - 1 - x) : x);
                int srcIdx = (srcY * srcWidth + srcX) * channels;
                int dstIdx = (y * faceSize + x) * channels;

                for (int c = 0; c < channels; c++) {
                    dst[dstIdx + c] = src[srcIdx + c];
                }
            }
        }
    }

} // namespace golias
