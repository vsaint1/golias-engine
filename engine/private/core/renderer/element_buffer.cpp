#include "core/renderer/element_buffer.h"

ElementBuffer::ElementBuffer(unsigned int* data, unsigned int count) : count(count)  {

    SDL_assert(sizeof(unsigned int) == sizeof(GLuint));

    glGenBuffers(1, &rendererId);
    GL_ERROR();

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, rendererId);
    GL_ERROR();

    glBufferData(GL_ELEMENT_ARRAY_BUFFER, count * sizeof(unsigned int), data, GL_STATIC_DRAW);
    GL_ERROR();
}

ElementBuffer::~ElementBuffer() {
    LOG_INFO("Deleting element buffer");
    glDeleteBuffers(1, &rendererId);
}

void ElementBuffer::Bind() const {

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, rendererId);
    GL_ERROR();
}

void ElementBuffer::Unbind() const {
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    GL_ERROR();
}
