#pragma once
#include "helpers/logging.h"

class VertexBuffer {

    unsigned int rendererId;

public:

    VertexBuffer() = default;

    VertexBuffer(const void* data, size_t size);

    ~VertexBuffer();

    void Bind() const;

    void Unbind() const;

    unsigned int GetRendererId() const {
        return rendererId;
    }
};
