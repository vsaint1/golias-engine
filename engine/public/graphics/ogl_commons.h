#pragma once
#include "graphics/render_types.h"
#include "graphics/texture.h"
#include "stdafx.h"

namespace golias {

    struct TextureFormatGl {
        GLint Internal;
        GLenum External;
        GLenum Type;
    };

    /// @brief  Maps a TextureFormat to its OpenGL internal/external/type triple.
    TextureFormatGl TextureFormatToGl(TextureFormat format);

    /// @brief  Maps a TextureFilter to its OpenGL minification filter.
    GLint TextureMinFilterToGl(TextureFilter filter);

    /// @brief  Maps a TextureFilter to its OpenGL magnification filter.
    GLint TextureMagFilterToGl(TextureFilter filter);

    /// @brief  Maps a TextureWrap to its OpenGL wrap mode.
    GLint TextureWrapToGl(TextureWrap wrap);

    /// @brief  Maps a BufferTarget to its corresponding OpenGL target enum.
    GLenum BufferTargetToGl(BufferTarget target);

    /// @brief  Maps a QueryType to its corresponding OpenGL query target enum.
    GLenum QueryTargetToGl(QueryType type);

} // namespace golias
