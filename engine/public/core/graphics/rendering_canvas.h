#pragma once



#include "core/graphics/gles3/rendering_device_gles3.h"

namespace golias {

    struct DrawCommand {
        Mesh* mesh         = nullptr;
        Material* material = nullptr;
    };

    class RenderingCanvas {
    public:

        void Submit(const DrawCommand& command);
        void Draw( RenderingDevice* rendering_device);

    private:
        std::vector<DrawCommand> command_queue;
    };

}; // namespace golias
