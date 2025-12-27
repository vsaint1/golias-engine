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
