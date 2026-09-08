#include "graphics/vertex_array.h"

#include "graphics/ogl_commons.h"

namespace golias {

    namespace {

        bool vertex_format_is_integer(VertexFormat format) {
            return format == VertexFormat::UShort || format == VertexFormat::UShort4 || format == VertexFormat::Int
                || format == VertexFormat::Int4 || format == VertexFormat::UByte4;
        }

    } // namespace

    VertexArray::VertexArray() {
        glGenVertexArrays(1, &mHandle);
    }

    VertexArray::~VertexArray() {
        if (mHandle != 0) {
            glDeleteVertexArrays(1, &mHandle);
        }
    }

    void VertexArray::SetVertexBuffer(const Ref<Buffer>& vertexBuffer, const VertexLayout& layout) {
        glBindVertexArray(mHandle);
        vertexBuffer->Bind();

        for (const auto& element : layout.Elements) {
            const void* offset = reinterpret_cast<void*>(static_cast<uintptr_t>(element.Offset));

            if (vertex_format_is_integer(element.Format)) {
                glVertexAttribIPointer(element.Index,
                                       VertexFormatComponentCount(element.Format),
                                       VertexFormatToGl(element.Format),
                                       static_cast<GLsizei>(layout.Stride),
                                       offset);
            } else {
                glVertexAttribPointer(element.Index,
                                      VertexFormatComponentCount(element.Format),
                                      VertexFormatToGl(element.Format),
                                      GL_FALSE,
                                      static_cast<GLsizei>(layout.Stride),
                                      offset);
            }

            glEnableVertexAttribArray(element.Index);
        }

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }

    void VertexArray::SetIndexBuffer(const Ref<Buffer>& indexBuffer) {
        glBindVertexArray(mHandle);
        indexBuffer->Bind();
        glBindVertexArray(0);
    }

    void VertexArray::SetInstanceBuffer(const Ref<Buffer>& instanceBuffer,
                                        uint32_t stride,
                                        const std::vector<InstanceAttribute>& attributes) {
       
                                            glBindVertexArray(mHandle);
        instanceBuffer->Bind();

        for (const InstanceAttribute& attribute : attributes) {
            glEnableVertexAttribArray(attribute.Location);
            glVertexAttribPointer(attribute.Location,
                                  static_cast<GLint>(attribute.ComponentCount),
                                  GL_FLOAT,
                                  GL_FALSE,
                                  static_cast<GLsizei>(stride),
                                  reinterpret_cast<void*>(static_cast<uintptr_t>(attribute.Offset)));
            glVertexAttribDivisor(attribute.Location, 1);
        }

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }

    void VertexArray::Bind() const {
        glBindVertexArray(mHandle);
    }

    void VertexArray::Unbind() const {
        glBindVertexArray(0);
    }

    void VertexArray::Draw(uint32_t vertexCount, uint32_t indexCount) const {
        if (indexCount > 0) {
            glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(indexCount), GL_UNSIGNED_INT, nullptr);
        } else {
            glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(vertexCount));
        }
    }

    void VertexArray::DrawIndexed(uint32_t start, uint32_t count) const {
        if (count == 0) {
            return;
        }

        glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(count), GL_UNSIGNED_INT, reinterpret_cast<void*>(start * sizeof(uint32_t)));
    }

    void VertexArray::DrawInstanced(uint32_t instanceCount, uint32_t vertexCount, uint32_t indexCount) const {
        if (indexCount > 0) {
            glDrawElementsInstanced(GL_TRIANGLES, static_cast<GLsizei>(indexCount), GL_UNSIGNED_INT, nullptr, instanceCount);
        } else {
            glDrawArraysInstanced(GL_TRIANGLES, 0, static_cast<GLsizei>(vertexCount), instanceCount);
        }
    }

} // namespace golias
