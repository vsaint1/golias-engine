#pragma once


#include "core/graphics/gles3/rendering_device_gles3.h"

namespace golias {

    struct DrawCommand {
        Mesh* mesh         = nullptr;
        Material* material = nullptr;
        glm::mat4 modelMatrix;
    };

    struct DrawCommand2D{
        glm::mat4 modelMatrix;
        Texture2D* texture = nullptr;
        glm::vec4 color    = glm::vec4(1.0f);
        glm::vec2 size = glm::vec2(32.0f,32.0f);
        glm::vec2 lowerLeftUV  = glm::vec2(0.0f, 0.0f);
        glm::vec2 upperRightUV = glm::vec2(1.0f, 1.0f);        
        glm::vec2 pivot = glm::vec2(0.5f, 0.5f); 
    };
    
    struct CameraData {
        glm::mat4 viewMatrix;
        glm::mat4 projectionMatrix;
        glm::mat4 orthographicMatrix;
        glm::vec3 position;
    };


    class RenderingCanvas {
    public:
        bool Initialize();
        void Submit(const DrawCommand& command);
        void Submit(const DrawCommand2D& command);
        void Draw(RenderingDevice* rd,const CameraData& camera);

    private:
        std::vector<DrawCommand> command_queue;
        std::vector<DrawCommand2D> command_queue_2d;
        std::shared_ptr<Mesh> quad = nullptr;
    };

}; // namespace golias
