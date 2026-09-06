#include "graphics/buffer.h"

#include "graphics/ogl_commons.h"

namespace golias {

    Buffer::Buffer(const BufferDesc& desc) : mDesc(desc) {

        GLuint buffer;
        glGenBuffers(1, &buffer);

        GLenum target         = GL_ARRAY_BUFFER;
        const char* targetStr = "Vertex";

        if (HasFlag(desc.Target, BufferTarget::Vertex)) {
            target = GL_ARRAY_BUFFER;
        } else if (HasFlag(desc.Target, BufferTarget::Index)) {
            target    = GL_ELEMENT_ARRAY_BUFFER;
            targetStr = "Index";
        } else if (HasFlag(desc.Target, BufferTarget::Uniform)) {
            target    = GL_UNIFORM_BUFFER;
            targetStr = "Uniform";
        }

        GLenum usage         = GL_STATIC_DRAW;
        const char* usageStr = "Static";
        if (desc.Usage == BufferUsage::Dynamic) {
            usage    = GL_DYNAMIC_DRAW;
            usageStr = "Dynamic";
        } else if (desc.Usage == BufferUsage::Stream) {
            usage    = GL_STREAM_DRAW;
            usageStr = "Stream";
        }

        glBindBuffer(target, buffer);
        glBufferData(target, desc.Size, nullptr, usage);
        glBindBuffer(target, 0);

        GOLIAS_LOG_TRACE("Target %s Buffer | Usage %s | Buffer Handle %u | Size %zu", targetStr, usageStr, buffer, desc.Size);

        mBufferId = buffer;
    }

    uint32_t Buffer::GetHandle() const {
        return mBufferId;
    }

    const BufferDesc& Buffer::GetDesc() const {
        return mDesc;
    }

    Buffer::~Buffer() {
        if (mBufferId != 0) {
            glDeleteBuffers(1, &mBufferId);
            mBufferId = 0;
        }
    }

    void Buffer::Update(const void* data, uint32_t size, uint32_t offset) {

        GLenum internalTarget = BufferTargetToGl(mDesc.Target);

        glBindBuffer(internalTarget, mBufferId);

        if (offset + size > mDesc.Size) {
            mDesc.Size = offset + size;
            glBufferData(internalTarget, mDesc.Size, data, GL_DYNAMIC_DRAW);
        } else {
            glBufferSubData(internalTarget, offset, size, data);
        }

        glBindBuffer(internalTarget, 0);
    }

    void Buffer::Bind(uint32_t slot) const {

        if (HasFlag(mDesc.Target, BufferTarget::Vertex)) {
            glBindBuffer(GL_ARRAY_BUFFER, mBufferId);
        } else if (HasFlag(mDesc.Target, BufferTarget::Index)) {
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mBufferId);
        } else if (HasFlag(mDesc.Target, BufferTarget::Uniform)) {
            glBindBufferBase(GL_UNIFORM_BUFFER, slot, mBufferId);
        }
    }

    void Buffer::Unbind(uint32_t slot) const {

        if (HasFlag(mDesc.Target, BufferTarget::Vertex)) {
            glBindBuffer(GL_ARRAY_BUFFER, 0);
        } else if (HasFlag(mDesc.Target, BufferTarget::Index)) {
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
        } else if (HasFlag(mDesc.Target, BufferTarget::Uniform)) {
            glBindBufferBase(GL_UNIFORM_BUFFER, slot, 0);
        }
    }
} // namespace golias
