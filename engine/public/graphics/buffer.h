#pragma once

#include "render_types.h"


namespace golias {

    class Buffer {

    public:
        Buffer(const BufferDesc& desc);
        ~Buffer();

        uint32_t GetHandle();

        const BufferDesc& GetDesc() const;

        void Bind(uint32_t slot = 0) const;

        void Update(const void* data, const uint32_t size, const uint32_t offset = 0);

        void Unbind(uint32_t slot = 0) const;

    private:
        uint32_t mBufferId = 0;

        BufferDesc mDesc = {};
    };
} // namespace golias
