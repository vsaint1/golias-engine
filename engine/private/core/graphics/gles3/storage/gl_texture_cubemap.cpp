#include "core/graphics/gles3/storage/gl_texture_cubemap.h"

#include "core/engine.h"
#include <spdlog/spdlog.h>
#include <stb_image.h>

namespace golias {

    OpenglTextureCubemap::OpenglTextureCubemap(const std::string& filePath) {
        void* data = nullptr;
        int imageWidth, imageHeight;
        bool isHDR;
        GLenum dataType, internalFormat, formatEnum;

        if (!LoadImageData(filePath, data, imageWidth, imageHeight, channels, isHDR, dataType, internalFormat, formatEnum)) {
            handle = 0;
            return;
        }

        float aspectRatio      = static_cast<float>(imageWidth) / static_cast<float>(imageHeight);
        int faceSize           = 0;
        bool isVerticalCross   = false;
        bool isEquirectangular = false;

        if (aspectRatio >= 1.9f && aspectRatio <= 2.1f) {
            isEquirectangular = true;
            faceSize          = imageWidth / 4;
            spdlog::info("OpenglTextureCubemap: Detected equirectangular format: {}x{}, face size: {}", imageWidth, imageHeight, faceSize);
        } else if (imageWidth * 4 == imageHeight * 3) {
            faceSize        = imageWidth / 3;
            isVerticalCross = true;
            spdlog::info("OpenglTextureCubemap: Detected vertical cross layout: {}x{}, face size: {}", imageWidth, imageHeight, faceSize);
        } else if (imageHeight * 4 == imageWidth * 3) {
            faceSize        = imageHeight / 3;
            isVerticalCross = false;
            spdlog::info("OpenglTextureCubemap: Detected horizontal cross layout: {}x{}, face size: {}", imageWidth, imageHeight, faceSize);
        } else {
            spdlog::error("OpenglTextureCubemap: Unsupported format: {}x{} (aspect: {:.2f}). "
                          "Supported: equirectangular (2:1), vertical cross (3:4), horizontal cross (4:3)",
                          imageWidth,
                          imageHeight,
                          aspectRatio);
            stbi_image_free(data);
            handle = 0;
            return;
        }

        if (faceSize < 256) {
            spdlog::warn("OpenglTextureCubemap: Face size is small ({}x{}), quality may be reduced.", faceSize, faceSize);
        }

        width  = faceSize;
        height = faceSize;

        GLuint textureID;
        glGenTextures(1, &textureID);
        handle = textureID;
        glBindTexture(GL_TEXTURE_CUBE_MAP, handle);

        if (isEquirectangular) {
            ProcessEquirectangular(data, imageWidth, imageHeight, faceSize, channels, isHDR, internalFormat, formatEnum, dataType);
        } else {
            ProcessCrossLayout(
                data, imageWidth, imageHeight, faceSize, channels, isHDR, isVerticalCross, internalFormat, formatEnum, dataType);
        }

        stbi_image_free(data);
        SetupTextureParameters();

        spdlog::info("OpenglTextureCubemap: Successfully created cubemap from {}: {}x{} -> {}x{} faces",
                     isEquirectangular ? "Equirectangular" : (isHDR ? "HDR cross" : "LDR cross"),
                     imageWidth,
                     imageHeight,
                     faceSize,
                     faceSize);
    }

    OpenglTextureCubemap::OpenglTextureCubemap(const std::array<std::string, 6>& faces) {
        const auto& fileSystem = Engine::GetInstance().GetFileSystem();

        GLuint textureID;
        glGenTextures(1, &textureID);
        glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

        bool loadSuccess = true;
        int faceWidth = 0, faceHeight = 0;

        for (Uint32 i = 0; i < faces.size(); i++) {
            auto imagePath = fileSystem.GetAssetFile(faces[i]);
            int w, h, nrChannels;
            unsigned char* data = stbi_load(imagePath.c_str(), &w, &h, &nrChannels, 0);

            if (!data) {
                spdlog::error("OpenglTextureCubemap: Failed to load cubemap face {}: {}", i, faces[i]);
                loadSuccess = false;
                break;
            }

            if (i == 0) {
                faceWidth  = w;
                faceHeight = h;
                channels   = nrChannels;
            } else {
                if (w != faceWidth || h != faceHeight || nrChannels != channels) {
                    spdlog::error("OpenglTextureCubemap: Face {} dimensions {}x{} (channels: {}) don't match first "
                                  "face {}x{} (channels: {})",
                                  i,
                                  w,
                                  h,
                                  nrChannels,
                                  faceWidth,
                                  faceHeight,
                                  channels);
                    stbi_image_free(data);
                    loadSuccess = false;
                    break;
                }
            }

            GLenum format         = (nrChannels == 3) ? GL_RGB : GL_RGBA;
            GLenum internalFormat = (nrChannels == 3) ? GL_RGB : GL_RGBA;

            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, internalFormat, w, h, 0, format, GL_UNSIGNED_BYTE, data);
            stbi_image_free(data);
        }

        if (!loadSuccess) {
            glDeleteTextures(1, &textureID);
            handle = 0;
            width  = 0;
            height = 0;
            return;
        }

        width  = faceWidth;
        height = faceHeight;
        handle = textureID;

        SetupTextureParameters();

        spdlog::info("OpenglTextureCubemap: Successfully created cubemap from 6 faces: {}x{}", width, height);
    }

    OpenglTextureCubemap::OpenglTextureCubemap() {
        spdlog::info("OpenglTextureCubemap: Creating procedural skybox cubemap.");

        GLuint textureID;
        glGenTextures(1, &textureID);
        glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

        const int size = 512;
        std::vector<unsigned char> data(size * size * 3);

        auto generate_face_gradient = [&](int face) {
            for (int y = 0; y < size; y++) {
                for (int x = 0; x < size; x++) {
                    int idx = (y * size + x) * 3;

                    float u = (2.0f * x / size) - 1.0f;
                    float v = (2.0f * y / size) - 1.0f;

                    float dx = 0, dy = 0, dz = 0;
                    switch (face) {
                    case 0:
                        dx = 1.0f;
                        dy = -v;
                        dz = -u;
                        break; // +X
                    case 1:
                        dx = -1.0f;
                        dy = -v;
                        dz = u;
                        break; // -X
                    case 2:
                        dx = u;
                        dy = 1.0f;
                        dz = v;
                        break; // +Y
                    case 3:
                        dx = u;
                        dy = -1.0f;
                        dz = -v;
                        break; // -Y
                    case 4:
                        dx = u;
                        dy = -v;
                        dz = 1.0f;
                        break; // +Z
                    case 5:
                        dx = -u;
                        dy = -v;
                        dz = -1.0f;
                        break; // -Z
                    }

                    float len = std::sqrt(dx * dx + dy * dy + dz * dz);
                    dy /= len;

                    float height = (dy + 1.0f) * 0.5f;
                    float t      = glm::smoothstep(0.0f, 1.0f, height);

                    // Sky gradient colors
                    glm::vec3 horizon(255.0f, 200.0f, 150.0f);
                    glm::vec3 zenith(50.0f, 120.0f, 200.0f);

                    float scatter   = std::pow(1.0f - t, 2.0f) * 0.3f;
                    glm::vec3 color = horizon * (1.0f - t) + zenith * t + glm::vec3(scatter * 50.0f, scatter * 50.0f, scatter * 30.0f);

                    float angle     = std::atan2(dz, dx);
                    float variation = std::sin(angle * 2.0f) * 10.0f * (1.0f - t);

                    data[idx + 0] = (unsigned char) std::clamp(color.r + variation, 0.0f, 255.0f);
                    data[idx + 1] = (unsigned char) std::clamp(color.g + variation * 0.5f, 0.0f, 255.0f);
                    data[idx + 2] = (unsigned char) std::clamp(color.b - variation * 0.3f, 0.0f, 255.0f);
                }
            }
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, 0, GL_RGB, size, size, 0, GL_RGB, GL_UNSIGNED_BYTE, data.data());
        };

        for (int face = 0; face < 6; face++) {
            generate_face_gradient(face);
        }

        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

        handle = textureID;
        width  = size;
        height = size;
    }

    OpenglTextureCubemap::~OpenglTextureCubemap() {
        if (handle != 0) {
            glDeleteTextures(1, &handle);
            handle = 0;
        }
    }

    bool OpenglTextureCubemap::LoadImageData(const std::string& path,
                                             void*& data,
                                             int& width,
                                             int& height,
                                             int& channels,
                                             bool& isHDR,
                                             GLenum& dataType,
                                             GLenum& internalFormat,
                                             GLenum& format) {
        auto extension = Engine::GetInstance().GetFileSystem().GetFileExtension(path);
        isHDR          = (extension == "hdr");

        if (isHDR) {
            float* hdrData = stbi_loadf(path.c_str(), &width, &height, &channels, 0);
            if (!hdrData) {
                spdlog::error("OpenglTextureCubemap: Failed to load HDR cubemap: {}", path);
                return false;
            }
            data           = hdrData;
            dataType       = GL_FLOAT;
            internalFormat = (channels == 3) ? GL_RGB16F : GL_RGBA16F;
            format         = (channels == 3) ? GL_RGB : GL_RGBA;
        } else {
            unsigned char* ldrData = stbi_load(path.c_str(), &width, &height, &channels, 0);
            if (!ldrData) {
                spdlog::error("OpenglTextureCubemap: Failed to load LDR cubemap: {}", path);
                return false;
            }
            data           = ldrData;
            dataType       = GL_UNSIGNED_BYTE;
            internalFormat = (channels == 3) ? GL_RGB8 : GL_RGBA8;
            format         = (channels == 3) ? GL_RGB : GL_RGBA;
        }

        return true;
    }

    void OpenglTextureCubemap::ProcessEquirectangular(void* srcData,
                                                      int srcWidth,
                                                      int srcHeight,
                                                      int faceSize,
                                                      int channels,
                                                      bool isHDR,
                                                      GLenum internalFormat,
                                                      GLenum format,
                                                      GLenum dataType) {
        size_t faceDataSize = faceSize * faceSize * channels * (isHDR ? sizeof(float) : 1);
        void* faceData      = malloc(faceDataSize);

        for (int face = 0; face < 6; face++) {
            if (isHDR) {
                SampleAndCopyPixels(
                    static_cast<float*>(srcData), static_cast<float*>(faceData), srcWidth, srcHeight, faceSize, channels, face);
            } else {
                SampleAndCopyPixels(static_cast<unsigned char*>(srcData),
                                    static_cast<unsigned char*>(faceData),
                                    srcWidth,
                                    srcHeight,
                                    faceSize,
                                    channels,
                                    face);
            }

            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, 0, internalFormat, faceSize, faceSize, 0, format, dataType, faceData);
        }

        SDL_free(faceData);
    }

    void OpenglTextureCubemap::ProcessCrossLayout(void* srcData,
                                                  int srcWidth,
                                                  int srcHeight,
                                                  int faceSize,
                                                  int channels,
                                                  bool isHDR,
                                                  bool isVertical,
                                                  GLenum internalFormat,
                                                  GLenum format,
                                                  GLenum dataType) {
        FacePos facePositions[6];

        if (isVertical) {
            facePositions[0] = {2, 1, false, false}; // +X
            facePositions[1] = {0, 1, false, false}; // -X
            facePositions[2] = {1, 0, false, false}; // +Y
            facePositions[3] = {1, 2, false, false}; // -Y
            facePositions[4] = {1, 1, false, false}; // +Z
            facePositions[5] = {1, 3, true, true}; // -Z
        } else {
            facePositions[0] = {2, 1, false, false}; // +X
            facePositions[1] = {0, 1, false, false}; // -X
            facePositions[2] = {1, 0, false, false}; // +Y
            facePositions[3] = {1, 2, false, false}; // -Y
            facePositions[4] = {1, 1, false, false}; // +Z
            facePositions[5] = {3, 1, false, false}; // -Z
        }

        size_t faceDataSize = faceSize * faceSize * channels * (isHDR ? sizeof(float) : 1);
        void* faceData      = malloc(faceDataSize);

        for (int face = 0; face < 6; face++) {
            if (isHDR) {
                CopyFaceData(static_cast<float*>(srcData),
                             static_cast<float*>(faceData),
                             srcWidth,
                             faceSize,
                             channels,
                             facePositions[face].x,
                             facePositions[face].y,
                             facePositions[face].flipVertical,
                             facePositions[face].flipHorizontal);
            } else {
                CopyFaceData(static_cast<unsigned char*>(srcData),
                             static_cast<unsigned char*>(faceData),
                             srcWidth,
                             faceSize,
                             channels,
                             facePositions[face].x,
                             facePositions[face].y,
                             facePositions[face].flipVertical,
                             facePositions[face].flipHorizontal);
            }

            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, 0, internalFormat, faceSize, faceSize, 0, format, dataType, faceData);
        }

        SDL_free(faceData);
    }

    void OpenglTextureCubemap::SetupTextureParameters() {
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

#if defined(SDL_PLATFORM_EMSCRIPTEN)
        glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
#endif
        if (GLAD_GL_EXT_texture_filter_anisotropic) {
            GLfloat maxAnisotropy = 1.0f;
            glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &maxAnisotropy);
            glTexParameterf(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAX_ANISOTROPY_EXT, std::min(maxAnisotropy, 4.0f));
        }
    }

} // namespace golias
