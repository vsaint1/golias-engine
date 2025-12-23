#pragma once
#include "core/graphics/structs.h"
#include <glad.h>

GLint ToGLBuferTarget(EBufferTarget target);
GLint ToGLBufferUsage(EBufferUsageFlags usage);
GLint ToGLDataType(EDataType type);
