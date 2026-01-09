#include "core/graphics/scene_renderer.h"

#include <glm/ext/matrix_transform.hpp>

namespace golias {

    bool create_renderer_internal(golias::ERenderingDeviceType deviceType, golias::RenderingDevice** pOutDevice) {
        switch (deviceType) {
        case golias::ERenderingDeviceType::COMPATIBILITY:
            *pOutDevice = new golias::RenderingDeviceGLES3();
            return true;
        case golias::ERenderingDeviceType::FORWARD_PLUS:
            spdlog::error("FORWARD_PLUS rendering device not implemented yet.");
            return false;
        default:
            spdlog::error("Unknown rendering device type.");
            return false;
        }
    }

    RenderingDevice* SceneRenderer::GetRenderingDevice() const {
        return rendering_device;
    }

    SceneRenderer::~SceneRenderer() {

        if (rendering_device) {
            delete rendering_device;
            rendering_device = nullptr;
        }
    }

    void SceneRenderer::Submit(const DrawCommand& command) {
        command_queue.push_back(command);
    }

    void SceneRenderer::Submit(const DrawCommand2D& command) {
        command_queue_2d.push_back(command);
    }

    void SceneRenderer::Submit(const ScreenCanvasCommand& command) {
        canvas_commands.push_back(command);
    }

    void SceneRenderer::Submit(const WorldCanvasCommand& command) {
        world_canvas_commands.push_back(command);
    }

    void SceneRenderer::Draw(const CameraCommand& camera) {

        for (const auto& command : command_queue) {


            rendering_device->BindMaterial(command.material);

            auto shader = command.material->GetShader();
            shader->SetUniform("MODEL_MATRIX", command.modelMatrix);
            shader->SetUniform("VIEW_MATRIX", camera.viewMatrix);
            shader->SetUniform("PROJECTION_MATRIX", camera.projectionMatrix);

            // Scene properties
            shader->SetUniform("u_viewPosition", camera.position);
            shader->SetUniform("u_ambientStrength", 0.3f);
            shader->SetUniform("u_specularStrength", 0.5f);
            shader->SetUniform("u_shininess", 32.0f);

            // Directional lights
            shader->SetUniform("u_directionalLightCount", 1);
            shader->SetUniform("u_directionalLights[0].direction", glm::vec3(0.5f, -1.0f, 0.3f));
            shader->SetUniform("u_directionalLights[0].color", glm::vec3(1.0f, 1.0f, 1.0f));
            shader->SetUniform("u_directionalLights[0].intensity", 1.0f);

            if (command.mesh) {

                rendering_device->BindMesh(command.mesh);

                rendering_device->DrawMesh(command.mesh);

                rendering_device->UnbindMesh(command.mesh);
            }
        }

        command_queue.clear();

        glDepthMask(GL_FALSE);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        auto shader_canvas = rendering_device->GetDefaultShaderCanvas();
        shader_canvas->Bind();

        for (const auto& command : world_canvas_commands) {
            if (!command.mesh || command.batches.empty()) {
                continue;
            }

            rendering_device->BindMesh(command.mesh);

            Uint32 indexOffset = 0;
            for (const auto& batch : command.batches) {

                if (batch.texture) {
                    shader_canvas->SetUniform("HAS_TEXTURE", 1);
                    shader_canvas->SetUniform("TEXTURE", batch.texture);
                } else {
                    shader_canvas->SetUniform("HAS_TEXTURE", 0);
                }

                glm::mat4 scaleMatrix      = glm::scale(glm::mat4(1.0f), glm::vec3(command.scale, command.scale, 1.0f));
                glm::mat4 finalModelMatrix = command.modelMatrix * scaleMatrix;

                shader_canvas->SetUniform("MODEL_MATRIX", finalModelMatrix);
                shader_canvas->SetUniform("VIEW_MATRIX", camera.viewMatrix);
                shader_canvas->SetUniform("PROJECTION_MATRIX", camera.projectionMatrix);
                shader_canvas->SetUniform("CAMERA_POSITION", camera.position);
                shader_canvas->SetUniform("USE_BILLBOARDING", true);

                command.mesh->DrawIndexed(indexOffset, batch.indexCount);

                indexOffset += batch.indexCount;
            }

            command.mesh->Unbind();
        }

        world_canvas_commands.clear();


        glDisable(GL_DEPTH_TEST);
        const auto shader_2d = rendering_device->GetDefaultShader2D();

        shader_2d->Bind();

        quad->Bind();
        for (const auto& command : command_queue_2d) {


            shader_2d->SetUniform("MODEL_MATRIX", command.modelMatrix);
            shader_2d->SetUniform("VIEW_MATRIX", camera.viewMatrix);
            shader_2d->SetUniform("PROJECTION_MATRIX", camera.orthographicMatrix);
            shader_2d->SetUniform("TEXTURE_SIZE", command.size);
            shader_2d->SetUniform("TEXTURE_PIVOT", command.pivot);
            shader_2d->SetUniform("TEXTURE_UV_MIN", command.lowerLeftUV);
            shader_2d->SetUniform("TEXTURE_UV_MAX", command.upperRightUV);
            shader_2d->SetUniform("COLOR", command.color);
            shader_2d->SetUniform("TEXTURE", command.texture);

            quad->Draw();
        }
        quad->Unbind();


        command_queue_2d.clear();

        if (!canvas_commands.empty()) {
            shader_canvas = rendering_device->GetDefaultShaderCanvas();
            shader_canvas->Bind();

            for (const auto& command : canvas_commands) {
                if (!command.mesh || command.batches.empty()) {
                    continue;
                }

                rendering_device->BindMesh(command.mesh);

                Uint32 indexOffset = 0;
                for (const auto& batch : command.batches) {

                    if (batch.texture) {
                        shader_canvas->SetUniform("HAS_TEXTURE", 1);
                        shader_canvas->SetUniform("TEXTURE", batch.texture);

                    } else {
                        shader_canvas->SetUniform("HAS_TEXTURE", 0);
                    }

                    shader_canvas->SetUniform("USE_BILLBOARDING", false);
                    shader_canvas->SetUniform("MODEL_MATRIX", glm::mat4(1.0f));
                    shader_canvas->SetUniform("VIEW_MATRIX", glm::mat4(1.0f));
                    shader_canvas->SetUniform("PROJECTION_MATRIX", camera.orthographicMatrix);
                    shader_canvas->SetUniform("CAMERA_POSITION", camera.position);

                    command.mesh->DrawIndexed(indexOffset, batch.indexCount);

                    indexOffset += batch.indexCount;
                }

                command.mesh->Unbind();
            }
        }

        glDepthMask(GL_TRUE);
        glEnable(GL_DEPTH_TEST);
        glDisable(GL_BLEND);
        canvas_commands.clear();
    }

    bool SceneRenderer::Initialize(SDL_Window* pWindow, ERenderingDeviceType deviceType) {

        if (!create_renderer_internal(deviceType, &rendering_device)) {
            spdlog::error("Engine::Initialize Failed to create Rendering Device.");
            return false;
        }

        if (!rendering_device->Initialize(pWindow)) {
            spdlog::error("Engine::InitializeFailed to initialize Rendering Device.");
            return false;
        }

        quad = Mesh::CreateQuad(1.0f, 1.0f);

        spdlog::info("SceneRenderer::Initialize Scene Renderer initialized successfully.");
        return true;
    }

    void SceneRenderer::Clear(const glm::vec4& color) {
        rendering_device->Clear(color);
    }

    void SceneRenderer::Present() {
        rendering_device->SwapChain();
    }

}; // namespace golias
