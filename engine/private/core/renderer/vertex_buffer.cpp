#include "core/renderer/vertex_buffer.h"

VertexBuffer::VertexBuffer(const void* data, size_t size) {

    SDL_assert(sizeof(size_t) == sizeof(GLsizeiptr));

    glGenBuffers(1, &rendererId);
    GL_ERROR();

    glBindBuffer(GL_ARRAY_BUFFER, rendererId);
    GL_ERROR();

    glBufferData(GL_ARRAY_BUFFER, size, data, GL_STATIC_DRAW);
    GL_ERROR();
}

VertexBuffer::~VertexBuffer() {

    glDeleteBuffers(1, &rendererId);
}

void VertexBuffer::Bind() const {
    glBindBuffer(GL_ARRAY_BUFFER, rendererId);
}

void VertexBuffer::Unbind() const {
    LOG_INFO("Unbinding vertex buffer");
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}
