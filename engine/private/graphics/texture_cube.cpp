#include "graphics/texture_cube.h"

#include "core/engine.h"
#include "graphics/ogl_commons.h"
#include <stb_image.h>

namespace {

    glm::vec3 cube_direction(uint32_t face, float u, float v) {
        switch (face) {
        case 0:
            return glm::normalize(glm::vec3(1.0f, -v, -u));
        case 1:
            return glm::normalize(glm::vec3(-1.0f, -v, u));
        case 2:
            return glm::normalize(glm::vec3(u, 1.0f, v));
        case 3:
            return glm::normalize(glm::vec3(u, -1.0f, -v));
        case 4:
            return glm::normalize(glm::vec3(u, -v, 1.0f));
        default:
            return glm::normalize(glm::vec3(-u, -v, -1.0f));
        }
    }

    glm::vec2 equirectangular_uv(const glm::vec3& direction) {
        constexpr float kPi = 3.14159265359f;
        return glm::vec2(std::atan2(direction.z, direction.x) / (2.0f * kPi) + 0.5f,
                         std::asin(glm::clamp(direction.y, -1.0f, 1.0f)) / kPi + 0.5f);
    }

} // namespace

namespace golias {

    TextureCube::TextureCube(const TextureDesc& desc) {
        Recreate(desc);
    }

    TextureCube::~TextureCube() {
        if (mTextureID) {
            glDeleteTextures(1, &mTextureID);
        }
    }

    Ref<TextureCube> TextureCube::Load(CString path) {
        const Path fullPath = Engine::GetInstance().GetFileSystem().GetAssetsFolder() / path.data();
        if (!Engine::GetInstance().GetFileSystem().FileExists(fullPath)) {
            GOLIAS_LOG_ERROR("Cubemap file does not exist: %s", fullPath.string().c_str());
            return nullptr;
        }

        int width               = 0;
        int height              = 0;
        int channels            = 0;
        const int requestedSize = 512;

        if (stbi_is_hdr(fullPath.string().c_str())) {
            float* source = stbi_loadf(fullPath.string().c_str(), &width, &height, &channels, 3);
            if (!source) {
                GOLIAS_LOG_ERROR("Failed to load HDR equirectangular texture: %s", path.data());
                return nullptr;
            }

            const uint32_t size = static_cast<uint32_t>(std::clamp(std::min(width / 4, height / 2), 1, requestedSize));
            TextureDesc desc{.Width = size, .Height = size, .Format = TextureFormat::RGBA16F};
            Ref<TextureCube> cube = std::make_shared<TextureCube>(desc);
            std::vector<float> face(size * size * 4);
            for (uint32_t faceIndex = 0; faceIndex < 6; ++faceIndex) {
                for (uint32_t y = 0; y < size; ++y) {
                    for (uint32_t x = 0; x < size; ++x) {
                        const glm::vec3 direction = cube_direction(faceIndex,
                                                                   static_cast<float>(x) / static_cast<float>(size - 1) * 2.0f - 1.0f,
                                                                   static_cast<float>(y) / static_cast<float>(size - 1) * 2.0f - 1.0f);
                        const glm::vec2 uv        = equirectangular_uv(direction);
                        const int sx              = std::clamp(static_cast<int>(uv.x * width), 0, width - 1);
                        const int sy              = std::clamp(static_cast<int>((1.0f - uv.y) * height), 0, height - 1);
                        const size_t sourceIndex  = (static_cast<size_t>(sy) * width + sx) * 3;
                        const size_t targetIndex  = (static_cast<size_t>(y) * size + x) * 4;
                        face[targetIndex + 0]     = source[sourceIndex + 0];
                        face[targetIndex + 1]     = source[sourceIndex + 1];
                        face[targetIndex + 2]     = source[sourceIndex + 2];
                        face[targetIndex + 3]     = 1.0f;
                    }
                }
                cube->UploadFaceFloat(faceIndex, size, size, face.data());
            }

            stbi_image_free(source);
            return cube;
        }

        unsigned char* source = stbi_load(fullPath.string().c_str(), &width, &height, &channels, 4);
        if (!source) {
            GOLIAS_LOG_ERROR("Failed to load equirectangular texture: %s", path.data());
            return nullptr;
        }

        const uint32_t size = static_cast<uint32_t>(std::clamp(std::min(width / 4, height / 2), 1, requestedSize));
        TextureDesc desc{.Width = size, .Height = size, .Format = TextureFormat::RGBA8};
        Ref<TextureCube> cube = std::make_shared<TextureCube>(desc);
        std::vector<unsigned char> face(size * size * 4);
        for (uint32_t faceIndex = 0; faceIndex < 6; ++faceIndex) {
            for (uint32_t y = 0; y < size; ++y) {
                for (uint32_t x = 0; x < size; ++x) {
                    const glm::vec3 direction = cube_direction(faceIndex,
                                                               static_cast<float>(x) / static_cast<float>(size - 1) * 2.0f - 1.0f,
                                                               static_cast<float>(y) / static_cast<float>(size - 1) * 2.0f - 1.0f);
                    const glm::vec2 uv        = equirectangular_uv(direction);
                    const int sx              = std::clamp(static_cast<int>(uv.x * width), 0, width - 1);
                    const int sy              = std::clamp(static_cast<int>((1.0f - uv.y) * height), 0, height - 1);
                    const size_t sourceIndex  = (static_cast<size_t>(sy) * width + sx) * 4;
                    const size_t targetIndex  = (static_cast<size_t>(y) * size + x) * 4;
                    std::copy_n(source + sourceIndex, 4, face.data() + targetIndex);
                }
            }
            cube->UploadFace(faceIndex, size, size, 4, face.data());
        }
        stbi_image_free(source);
        return cube;
    }

    Ref<TextureCube> TextureCube::CreateProcedural(const glm::vec3& skyTint,
                                                   const glm::vec3& groundColor,
                                                   const glm::vec3& sunDirection,
                                                   float sunBrightness) {
        constexpr uint32_t size = 64;
        TextureDesc desc{.Width = size, .Height = size, .Format = TextureFormat::RGBA8};
        Ref<TextureCube> cube = std::make_shared<TextureCube>(desc);
        const glm::vec3 sun   = glm::normalize(sunDirection);
        std::vector<unsigned char> face(size * size * 4);
        for (uint32_t faceIndex = 0; faceIndex < 6; ++faceIndex) {
            for (uint32_t y = 0; y < size; ++y) {
                for (uint32_t x = 0; x < size; ++x) {
                    const glm::vec3 direction = cube_direction(faceIndex,
                                                               static_cast<float>(x) / static_cast<float>(size - 1) * 2.0f - 1.0f,
                                                               static_cast<float>(y) / static_cast<float>(size - 1) * 2.0f - 1.0f);
                    const float horizon       = glm::smoothstep(-0.1f, 0.3f, direction.y);
                    glm::vec3 color           = glm::mix(groundColor, skyTint, horizon);
                    color += glm::vec3(glm::pow(glm::max(glm::dot(direction, sun), 0.0f), 32.0f) * sunBrightness * 0.5f);
                    color              = glm::clamp(color, glm::vec3(0.0f), glm::vec3(1.0f));
                    const size_t index = (static_cast<size_t>(y) * size + x) * 4;
                    face[index + 0]    = static_cast<unsigned char>(color.r * 255.0f);
                    face[index + 1]    = static_cast<unsigned char>(color.g * 255.0f);
                    face[index + 2]    = static_cast<unsigned char>(color.b * 255.0f);
                    face[index + 3]    = 255;
                }
            }
            cube->UploadFace(faceIndex, size, size, 4, face.data());
        }
        return cube;
    }

    bool TextureCube::Recreate(const TextureDesc& desc) {

        if (!desc.Width || !desc.Height || desc.Width != desc.Height) {
            return false;
        }

        if (mTextureID) {
            glDeleteTextures(1, &mTextureID);
        }

        mDesc = desc;
        glGenTextures(1, &mTextureID);
        glBindTexture(GL_TEXTURE_CUBE_MAP, mTextureID);

        const TextureFormatGl glFormat = TextureFormatToGl(desc.Format);

        for (int face = 0; face < 6; ++face) {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + face,
                         0,
                         glFormat.Internal,
                         desc.Width,
                         desc.Height,
                         0,
                         glFormat.External,
                         glFormat.Type,
                         nullptr);
        }

        const GLint filter = TextureMinFilterToGl(desc.Filter);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, filter);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, TextureMagFilterToGl(desc.Filter));
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
        glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

        return true;
    }

    bool TextureCube::UploadFace(uint32_t face, uint32_t width, uint32_t height, int channels, const unsigned char* data) {
        if (face >= 6 || width != mDesc.Width || height != mDesc.Height || !data) {
            return false;
        }

        GLenum format = channels == 4 ? GL_RGBA : GL_RGB;
        glBindTexture(GL_TEXTURE_CUBE_MAP, mTextureID);
        glTexSubImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + face,
                        0,
                        0,
                        0,
                        static_cast<GLsizei>(width),
                        static_cast<GLsizei>(height),
                        format,
                        GL_UNSIGNED_BYTE,
                        data);
        glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
        return true;
    }

    bool TextureCube::UploadFaceFloat(uint32_t face, uint32_t width, uint32_t height, const float* data) {
        if (face >= 6 || width != mDesc.Width || height != mDesc.Height || !data) {
            return false;
        }

        glBindTexture(GL_TEXTURE_CUBE_MAP, mTextureID);
        glTexSubImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + face,
                        0,
                        0,
                        0,
                        static_cast<GLsizei>(width),
                        static_cast<GLsizei>(height),
                        GL_RGBA,
                        GL_FLOAT,
                        data);
        glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
        return true;
    }

    GLuint TextureCube::GetHandle() const {
        return mTextureID;
    }

    GLenum TextureCube::GetTarget() const {
        return GL_TEXTURE_CUBE_MAP;
    }

    const TextureDesc& TextureCube::GetDesc() const {
        return mDesc;
    }

} // namespace golias
