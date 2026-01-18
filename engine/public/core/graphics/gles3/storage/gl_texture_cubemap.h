#pragma once
#include "core/graphics/texture_cubemap.h"
#include <array>
#include <glad.h>
#include <string>

#include <glm/glm.hpp>


namespace golias {
    class OpenglTextureCubemap : public TextureCubemap {
    public:
        OpenglTextureCubemap(const std::string& filePath);
        OpenglTextureCubemap(const std::array<std::string, 6>& faces);
        OpenglTextureCubemap(int width, int height, ETextureFormat format, const void* pixelData);
        OpenglTextureCubemap();

        ~OpenglTextureCubemap() override;

    private:
        struct FacePos {
            int x, y;
            bool flipVertical;
            bool flipHorizontal;
        };

        bool LoadImageData(const std::string& path,
                           void*& data,
                           int& width,
                           int& height,
                           int& channels,
                           bool& isHDR,
                           GLenum& dataType,
                           GLenum& internalFormat,
                           GLenum& format);

        void ProcessEquirectangular(void* srcData,
                                    int srcWidth,
                                    int srcHeight,
                                    int faceSize,
                                    int channels,
                                    bool isHDR,
                                    GLenum internalFormat,
                                    GLenum format,
                                    GLenum dataType);

        void ProcessCrossLayout(void* srcData,
                                int srcWidth,
                                int srcHeight,
                                int faceSize,
                                int channels,
                                bool isHDR,
                                bool isVertical,
                                GLenum internalFormat,
                                GLenum format,
                                GLenum dataType);

        void SetupTextureParameters();
    };
} // namespace golias
