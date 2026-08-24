#pragma once
#include "stdafx.h"


namespace golias {

    class Texture2D {

    public:
        Texture2D(int32_t width, int32_t height, int32_t channels, unsigned char* data);
        ~Texture2D();

        static Ref<Texture2D> Load(CString path);

        int32_t GetWidth() const;

        int32_t GetHeight() const;

        int32_t GetChannels() const;

        GLuint GetHandle() const;

    private:
        int32_t mWidth    = 0;
        int32_t mHeight   = 0;
        int32_t mChannels = 0;

        GLuint mTextureID = 0;
    };


    class TextureManager {
    public:
        Ref<Texture2D> TryGet(CString path);

        Ref<Texture2D> AcquireWhiteTexture();

        Ref<Texture2D> AcquireErrorTexture();

    private:
        std::unordered_map<String, Ref<Texture2D>> mTextures;

        Ref<Texture2D> mWhiteTexture = nullptr;
        Ref<Texture2D> mErrorTexture = nullptr;
    };
} // namespace golias
