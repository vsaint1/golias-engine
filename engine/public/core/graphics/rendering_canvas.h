#pragma once


#include "core/graphics/gles3/rendering_device_gles3.h"

namespace golias {

    struct DrawCommand {
        Mesh* mesh         = nullptr;
        Material* material = nullptr;
        glm::mat4 modelMatrix;
    };

    struct CameraData {
        glm::mat4 viewMatrix;
        glm::mat4 projectionMatrix;
    };

    class RenderingCanvas {
    public:
        void Submit(const DrawCommand& command);
        void Draw(RenderingDevice* rd,const CameraData& camera);

    private:
        std::vector<DrawCommand> command_queue;
    };

}; // namespace golias
