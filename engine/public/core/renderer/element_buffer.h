#pragma once
#include "helpers/logging.h"


class ElementBuffer {

    unsigned int rendererId;
    unsigned int count;

public:
    ElementBuffer() = default;

    ElementBuffer(unsigned int* data, unsigned int count);

    ~ElementBuffer();

    void Bind() const;

    void Unbind() const;

    unsigned int GetRendererId() const {
        return rendererId;
    }

    unsigned int GetCount() const {
        return count;
    }
};
