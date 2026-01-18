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

bool IsDepthFormat(ETextureFormat format);

GLbitfield ToGLClearFlags(EClearFlags flags);

std::string GetShaderHeaderVersion();

struct GLTextureFormatDesc {
    GLenum internalFormat;
    GLenum format;
    GLenum type;
    bool   isDepth;
};

GLTextureFormatDesc GetGLTextureFormatDesc(ETextureFormat format);