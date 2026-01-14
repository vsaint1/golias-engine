#pragma once
#include "core/graphics/structs.h"
#include <glad.h>

GLint ToGLBuferTarget(EBufferTarget target);
GLint ToGLBufferUsage(EBufferUsageFlags usage);
GLint ToGLDataType(EDataType type);

GLenum ToGLTextureWrapMode(ETextureWrapMode mode);
GLenum ToGLTextureFilterMode(ETextureFilterMode mode);
GLenum ToGLTextureFormat(ETextureFormat format);
GLenum ToGLTextureFormatFromChannels(int num_channels);

GLenum ToGLBlendFactor(EBlendFactor factor);
GLenum ToGLBlendOp(EBlendOp op);

GLenum ToGLCullMode(ECullMode mode);
GLenum ToGLComparisonFunc(EComparisonFunc func);

GLenum ToGLStencilOp(EStencilOp op);

 inline  GLenum FramebufferTextureFormatToGL(EFramebufferTextureFormat format) {
        switch (format) {
        case EFramebufferTextureFormat::RGBA8:
            return GL_RGBA8;
        case EFramebufferTextureFormat::RGB8:
            return GL_RGB8;
        case EFramebufferTextureFormat::RGBA16F:
            return GL_RGBA16F;
        case EFramebufferTextureFormat::RGB16F:
            return GL_RGB16F;
        case EFramebufferTextureFormat::RGBA32F:
            return GL_RGBA32F;
        case EFramebufferTextureFormat::RGB32F:
            return GL_RGB32F;
        case EFramebufferTextureFormat::DEPTH24:
            return GL_DEPTH_COMPONENT24;
        case EFramebufferTextureFormat::DEPTH32F:
            return GL_DEPTH_COMPONENT32F;
        case EFramebufferTextureFormat::DEPTH24_STENCIL8:
            return GL_DEPTH24_STENCIL8;
        case EFramebufferTextureFormat::DEPTH32F_STENCIL8:
            return GL_DEPTH32F_STENCIL8;
        }
        return GL_RGBA8;
    }

    inline  bool IsDepthFormat(EFramebufferTextureFormat format) {
        switch (format) {
        case EFramebufferTextureFormat::DEPTH24:
        case EFramebufferTextureFormat::DEPTH32F:
        case EFramebufferTextureFormat::DEPTH24_STENCIL8:
        case EFramebufferTextureFormat::DEPTH32F_STENCIL8:
            return true;
        default:
            return false;
        }
    }

std::string GetShaderHeaderVersion();
