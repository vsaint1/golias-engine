#include "core/graphics/rendering_canvas.h"

namespace golias{
    void RenderingCanvas::Submit(const DrawCommand& command) {
       command_queue.push_back(command);
    }


    void RenderingCanvas::Draw(RenderingDevice* rendering_device) {
        for (const auto& command : command_queue) {

            rendering_device->BindMaterial(command.material);
            command.material->SetParameter("MODEL_MATRIX", command.modelMatrix);
            
            rendering_device->BindMesh(command.mesh);   
            
            rendering_device->DrawMesh(command.mesh);
        }

        command_queue.clear();
    }
};