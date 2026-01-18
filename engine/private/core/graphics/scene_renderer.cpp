#include "core/graphics/scene_renderer.h"

#include "scene/3d/skeleton_animation_component.h"
#include "scene/3d/world_environment_component.h"
#include <algorithm>

#include <glm/ext/matrix_clip_space.hpp>
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


    void SceneRenderer::Draw(const CameraCommand& camera) {
        renderContext.camera = camera;

        CalculateLightSpaceMatrices();

        std::vector<DrawCommand> opaqueCommands;
        std::vector<DrawCommand> transparentCommands;

        Frustum frustum = CalculateFrustum(camera.projectionMatrix * camera.viewMatrix);

        for (const auto& command : command_queue) {
            if (!command.mesh || !command.material) {
                continue;
            }

            AABB worldAABB = WorldTransformAABB(command.mesh->GetLocalAABB(), command.modelMatrix);

            if (!frustum.IntersectsAABB(worldAABB) || !frustum.IntersectsSphereAABB(worldAABB)) {
                continue;
            }

            if (command.material->IsTransparent()) {
                transparentCommands.push_back(command);
            } else {
                opaqueCommands.push_back(command);
            }
        }

        if (!transparentCommands.empty()) {
            std::ranges::sort(transparentCommands, [this](const DrawCommand& a, const DrawCommand& b) {
                glm::vec3 posA = glm::vec3(a.modelMatrix[3]);
                glm::vec3 posB = glm::vec3(b.modelMatrix[3]);
                float distA    = glm::distance(renderContext.camera.position, posA);
                float distB    = glm::distance(renderContext.camera.position, posB);
                return distA > distB;
            });
        }

        ShadowPass();
        GeometryOpaquePass(opaqueCommands);
        SkyboxPass();
        GeometryTransparentPass(transparentCommands);
        WorldCanvasPass();
        Sprite2DPass();
        ScreenCanvasPass();
    }

    void SceneRenderer::SetupMaterialUniforms(const DrawCommand& command) {
        auto shader = command.material->GetShader();

        shader->SetUniform("MODEL_MATRIX", command.modelMatrix);
        shader->SetUniform("VIEW_MATRIX", renderContext.camera.viewMatrix);
        shader->SetUniform("PROJECTION_MATRIX", renderContext.camera.projectionMatrix);
        shader->SetUniform("CAMERA_POSITION", renderContext.camera.position);

        shader->SetUniform("u_specularStrength", 0.5f);
        shader->SetUniform("u_shininess", 32.0f);

        if (world_environment_command.environmentComponent) {
            shader->SetUniform("u_tonemap", static_cast<int>(world_environment_command.environmentComponent->GetToneMappingMode()));
            shader->SetUniform("u_exposure", world_environment_command.environmentComponent->GetExposure());
        }

        if (command.skeletonAnimation && command.skeletonAnimation->GetSkeleton()) {
            shader->SetUniform("USE_SKINNING", true);
            const auto& jointMatrices = command.skeletonAnimation->GetJointMatrices();
            shader->SetUniform("BONE_MATRICES", jointMatrices.data(), static_cast<int>(jointMatrices.size()));
        } else {
            shader->SetUniform("USE_SKINNING", false);
        }
    }


    void SceneRenderer::SetupLightingUniforms(Shader* shader) {


        // Directional lights
        int directionalLightCount = static_cast<int>(directional_lights.size());
        shader->SetUniform("u_directionalLightCount", directionalLightCount);
        for (int i = 0; i < directionalLightCount; ++i) {
            std::string p = "u_directionalLights[" + std::to_string(i) + "].";
            shader->SetUniform(p + "direction", directional_lights[i].direction);
            shader->SetUniform(p + "color", directional_lights[i].color);
            shader->SetUniform(p + "intensity", directional_lights[i].intensity);
            shader->SetUniform(p + "castShadows", directional_lights[i].castShadows);
        }

        // Point lights
        int pointLightCount = static_cast<int>(point_lights.size());
        shader->SetUniform("u_pointLightCount", pointLightCount);
        for (int i = 0; i < pointLightCount; ++i) {
            std::string p = "u_pointLights[" + std::to_string(i) + "].";
            shader->SetUniform(p + "position", point_lights[i].position);
            shader->SetUniform(p + "color", point_lights[i].color);
            shader->SetUniform(p + "intensity", point_lights[i].intensity);
            shader->SetUniform(p + "range", point_lights[i].range);
            shader->SetUniform(p + "constant", point_lights[i].constant);
            shader->SetUniform(p + "linear", point_lights[i].linear);
            shader->SetUniform(p + "quadratic", point_lights[i].quadratic);
        }

        // Spot lights - now using pre-calculated cosine values
        int spotLightCount = static_cast<int>(spot_lights.size());
        shader->SetUniform("u_spotLightCount", spotLightCount);
        for (int i = 0; i < spotLightCount; ++i) {
            std::string p = "u_spotLights[" + std::to_string(i) + "].";
            shader->SetUniform(p + "position", spot_lights[i].position);
            shader->SetUniform(p + "direction", spot_lights[i].direction);
            shader->SetUniform(p + "color", spot_lights[i].color);
            shader->SetUniform(p + "intensity", spot_lights[i].intensity);
            shader->SetUniform(p + "range", spot_lights[i].range);
            // Use pre-calculated cosine values
            shader->SetUniform(p + "innerConeAngleCos", spot_lights[i].innerConeAngleCos);
            shader->SetUniform(p + "outerConeAngleCos", spot_lights[i].outerConeAngleCos);
            shader->SetUniform(p + "constant", spot_lights[i].constant);
            shader->SetUniform(p + "linear", spot_lights[i].linear);
            shader->SetUniform(p + "quadratic", spot_lights[i].quadratic);
        }
    }

    void SceneRenderer::SetupShadowUniforms(Shader* shader) {
        if (renderContext.shadowsEnabled && !directional_lights.empty()) {
            rendering_device->BindTexture(
                shader, "SHADOW_MAP", 6, rendering_device->GetDefaultShadowMapFramebuffer()->GetDepthAttachment().get());
            shader->SetUniform("LIGHT_SPACE_MATRIX", directional_lights[0].lightSpaceMatrix);
        }
    }

    void SceneRenderer::SetupIBLUniforms(Shader* shader, const DrawCommand& command) {


        const bool useIBL =
            command.material->UseImageBasedLighting() && world_environment_command.environmentComponent
            && world_environment_command.environmentComponent->GetTextureCubemap()
            && world_environment_command.environmentComponent->GetEnvironmentMode() == EWorldEnvironmentMode::WORLD_ENVIRONMENT_MODE_SKYBOX;

        if (!renderContext.iblEnabled || !useIBL) {
            shader->SetUniform("USE_IBL", false);
            auto cubemap = rendering_device->GetWhiteTextureCubemap().get();
            rendering_device->BindCubemap(shader, "IRRADIANCE_MAP", 7, cubemap);
            rendering_device->BindCubemap(shader, "PREFILTER_MAP", 8, cubemap);
            shader->SetUniform("AMBIENT_STRENGTH", 0.5f);
            return;
        }

        if (useIBL) {
            auto cubemap = world_environment_command.environmentComponent->GetTextureCubemap().get();

            rendering_device->BindCubemap(shader, "IRRADIANCE_MAP", 7, cubemap);
            rendering_device->BindCubemap(shader, "PREFILTER_MAP", 8, cubemap);

            shader->SetUniform("USE_IBL", true);
            shader->SetUniform("AMBIENT_STRENGTH", 0.3f);
        }
    }

    void SceneRenderer::RenderObject(const DrawCommand& command) {
        rendering_device->BindMaterial(command.material);
        auto shader = command.material->GetShader();

        SetupMaterialUniforms(command);
        SetupLightingUniforms(shader.get());
        SetupShadowUniforms(shader.get());
        SetupIBLUniforms(shader.get(), command);

        rendering_device->BindMesh(command.mesh);
        rendering_device->DrawMesh(command.mesh);
    }

    void SceneRenderer::CalculateLightSpaceMatrices() {
        if (!AreShadowsEnabled()) {
            return;
        }

        for (auto& directional_light : directional_lights) {
            if (!directional_light.castShadows) {
                continue;
            }

            const float orthoSize  = 30.0f;
            const float near_plane = 1.0f;
            const float far_plane  = 150.0f;

            glm::mat4 lightProjection = glm::ortho(-orthoSize, orthoSize, -orthoSize, orthoSize, near_plane, far_plane);

            glm::vec3 lightDir = glm::normalize(directional_light.direction);

            glm::vec3 cameraFront  = glm::normalize(-renderContext.camera.viewMatrix[2]);
            glm::vec3 shadowCenter = renderContext.camera.position + cameraFront * (orthoSize * 0.5f);

            glm::vec3 lightPos = shadowCenter - lightDir * (far_plane * 0.5f);

            glm::mat4 lightView = glm::lookAt(lightPos, shadowCenter, glm::vec3(0.0f, 1.0f, 0.0f));

            directional_light.lightSpaceMatrix = lightProjection * lightView;
        }
    }

    void SceneRenderer::ShadowPass() {
        if (!AreShadowsEnabled()) {
            return;
        }

        bool hasShadowCasters = false;
        for (const auto& light : directional_lights) {
            if (light.castShadows) {
                hasShadowCasters = true;
                break;
            }
        }

        if (!hasShadowCasters) {
            return;
        }

        rendering_device->SetDepthTest(true);
        rendering_device->SetDepthWrite(true);
        rendering_device->SetBlendMode(EBlendMode::BLEND_MODE_DISABLED);
        rendering_device->SetCullMode(ECullMode::CULL_MODE_FRONT);

        rendering_device->GetDefaultShadowMapFramebuffer()->Bind();
        rendering_device->ClearBuffer(EClearFlags::CLEAR_DEPTH);

        auto shadowShader = rendering_device->GetDefaultShadowMapShader();
        shadowShader->Bind();

        for (const auto& directional_light : directional_lights) {
            if (!directional_light.castShadows) {
                continue;
            }

            shadowShader->SetUniform("LIGHT_SPACE_MATRIX", directional_light.lightSpaceMatrix);

            for (const auto& command : command_queue) {
                if (!command.mesh || !command.material || command.material->IsTransparent()) {
                    continue;
                }

                shadowShader->SetUniform("MODEL_MATRIX", command.modelMatrix);

                if (command.skeletonAnimation && command.skeletonAnimation->GetSkeleton()) {
                    shadowShader->SetUniform("USE_SKINNING", true);
                    const auto& jointMatrices = command.skeletonAnimation->GetJointMatrices();
                    shadowShader->SetUniform("BONE_MATRICES", jointMatrices.data(), static_cast<int>(jointMatrices.size()));
                } else {
                    shadowShader->SetUniform("USE_SKINNING", false);
                }

                rendering_device->BindMesh(command.mesh);
                rendering_device->DrawMesh(command.mesh);
            }
            break;
        }

        rendering_device->GetDefaultShadowMapFramebuffer()->Unbind();
    }

    void SceneRenderer::GeometryOpaquePass(const std::vector<DrawCommand>& opaqueCommands) {
        rendering_device->SetDepthTest(true);
        rendering_device->SetDepthComparison(EComparisonFunc::COMPARISON_LESS);
        rendering_device->SetDepthWrite(true);
        rendering_device->SetBlendMode(EBlendMode::BLEND_MODE_DISABLED);
        rendering_device->SetCullMode(ECullMode::CULL_MODE_BACK);

        for (const auto& command : opaqueCommands) {
            RenderObject(command);
        }
    }

    void SceneRenderer::SkyboxPass() {
        if (!world_environment_command.environmentComponent
            || world_environment_command.environmentComponent->GetEnvironmentMode() != WORLD_ENVIRONMENT_MODE_SKYBOX) {
            return;
        }

        auto skyboxMesh    = world_environment_command.environmentComponent->GetSkyboxMesh();
        auto skyboxTexture = world_environment_command.environmentComponent->GetTextureCubemap();
        auto skyboxShader  = rendering_device->GetDefaultSkyboxShader();

        if (!skyboxShader || !skyboxMesh || !skyboxTexture) {
            return;
        }

        rendering_device->SetDepthComparison(EComparisonFunc::COMPARISON_LESS_EQUAL);
        rendering_device->SetDepthWrite(false);
        rendering_device->SetBlendMode(EBlendMode::BLEND_MODE_DISABLED);
        rendering_device->SetCullMode(ECullMode::CULL_MODE_DISABLED);

        skyboxShader->Bind();

        glm::mat4 viewNoTranslation = glm::mat4(glm::mat3(renderContext.camera.viewMatrix));
        skyboxShader->SetUniform("VIEW_MATRIX", viewNoTranslation);
        skyboxShader->SetUniform("PROJECTION_MATRIX", renderContext.camera.projectionMatrix);
        skyboxShader->SetUniform("EXPOSURE", world_environment_command.environmentComponent->GetExposure());

        rendering_device->BindCubemap(skyboxShader.get(), "SKYBOX", 0, skyboxTexture.get());

        rendering_device->BindMesh(skyboxMesh.get());
        rendering_device->DrawMesh(skyboxMesh.get());

        rendering_device->SetCullMode(ECullMode::CULL_MODE_BACK);
        rendering_device->SetDepthWrite(true);
        rendering_device->SetDepthComparison(EComparisonFunc::COMPARISON_LESS);
    }

    void SceneRenderer::GeometryTransparentPass(const std::vector<DrawCommand>& transparentCommands) {
        rendering_device->SetDepthTest(true);
        rendering_device->SetDepthComparison(EComparisonFunc::COMPARISON_LESS);
        rendering_device->SetDepthWrite(true);
        rendering_device->SetBlendMode(EBlendMode::BLEND_MODE_ALPHA);
        rendering_device->SetCullMode(ECullMode::CULL_MODE_BACK);

        for (const auto& command : transparentCommands) {
            RenderObject(command);
        }
    }

    void SceneRenderer::WorldCanvasPass() {
        if (world_canvas_commands.empty()) {
            return;
        }

        rendering_device->SetDepthTest(true);
        rendering_device->SetDepthWrite(false);
        rendering_device->SetBlendMode(EBlendMode::BLEND_MODE_ALPHA);

        auto shader_canvas = rendering_device->GetDefaultShaderCanvas();
        shader_canvas->Bind();

        for (const auto& command : world_canvas_commands) {
            if (!command.mesh || command.batches.empty()) {
                continue;
            }

            rendering_device->BindMesh(command.mesh);

            Uint32 offset = 0;
            for (const auto& batch : command.batches) {
                shader_canvas->SetUniform("TEXTURE",
                                          batch.texture != nullptr ? batch.texture : rendering_device->GetWhiteTexture2D().get());

                glm::mat4 model = command.modelMatrix * glm::scale(glm::mat4(1.0f), glm::vec3(command.scale));

                shader_canvas->SetUniform("MODEL_MATRIX", model);
                shader_canvas->SetUniform("VIEW_MATRIX", renderContext.camera.viewMatrix);
                shader_canvas->SetUniform("PROJECTION_MATRIX", renderContext.camera.projectionMatrix);
                shader_canvas->SetUniform("CAMERA_POSITION", renderContext.camera.position);
                shader_canvas->SetUniform("USE_BILLBOARDING", command.useBillboarding);

                command.mesh->DrawIndexed(offset, batch.indexCount);
                offset += batch.indexCount;
            }
        }

        rendering_device->SetDepthWrite(true);
        rendering_device->SetBlendMode(EBlendMode::BLEND_MODE_DISABLED);
    }

    void SceneRenderer::Sprite2DPass() {
        if (command_queue_2d.empty()) {
            return;
        }

        rendering_device->SetDepthTest(false);
        rendering_device->SetBlendMode(EBlendMode::BLEND_MODE_ALPHA);
        rendering_device->SetCullMode(ECullMode::CULL_MODE_DISABLED);

        const auto shader_2d = rendering_device->GetDefaultShader2D();
        shader_2d->Bind();

        quad->Bind();
        for (const auto& command : command_queue_2d) {
            shader_2d->SetUniform("MODEL_MATRIX", command.modelMatrix);
            shader_2d->SetUniform("VIEW_MATRIX", renderContext.camera.viewMatrix);
            shader_2d->SetUniform("PROJECTION_MATRIX", renderContext.camera.orthographicMatrix);
            shader_2d->SetUniform("TEXTURE_SIZE", command.size);
            shader_2d->SetUniform("TEXTURE_PIVOT", command.pivot);
            shader_2d->SetUniform("TEXTURE_UV_MIN", command.lowerLeftUV);
            shader_2d->SetUniform("TEXTURE_UV_MAX", command.upperRightUV);
            shader_2d->SetUniform("COLOR", command.color);
            shader_2d->SetUniform("TEXTURE", command.texture != nullptr ? command.texture : rendering_device->GetWhiteTexture2D().get());

            quad->Draw();
        }
        quad->Unbind();
    }

    void SceneRenderer::ScreenCanvasPass() {
        if (canvas_commands.empty()) {
            return;
        }

        rendering_device->SetDepthTest(false);
        rendering_device->SetBlendMode(EBlendMode::BLEND_MODE_ALPHA);
        rendering_device->SetCullMode(ECullMode::CULL_MODE_DISABLED);

        auto shader_canvas = rendering_device->GetDefaultShaderCanvas();
        shader_canvas->Bind();

        for (const auto& command : canvas_commands) {
            if (!command.mesh || command.batches.empty()) {
                continue;
            }

            rendering_device->BindMesh(command.mesh);

            Uint32 indexOffset = 0;
            for (const auto& batch : command.batches) {
                shader_canvas->SetUniform("TEXTURE",
                                          batch.texture != nullptr ? batch.texture : rendering_device->GetWhiteTexture2D().get());

                shader_canvas->SetUniform("USE_BILLBOARDING", false);
                shader_canvas->SetUniform("MODEL_MATRIX", glm::mat4(1.0f));
                shader_canvas->SetUniform("VIEW_MATRIX", glm::mat4(1.0f));
                shader_canvas->SetUniform("PROJECTION_MATRIX", renderContext.camera.orthographicMatrix);
                shader_canvas->SetUniform("CAMERA_POSITION", renderContext.camera.position);

                command.mesh->DrawIndexed(indexOffset, batch.indexCount);
                indexOffset += batch.indexCount;
            }

            command.mesh->Unbind();
        }

        rendering_device->SetDepthTest(true);
        rendering_device->SetBlendMode(EBlendMode::BLEND_MODE_DISABLED);
    }


    void SceneRenderer::SetOcclusionCullingEnabled(bool enabled) {
        renderContext.occlusionCullingEnabled = enabled;
    }

    bool SceneRenderer::IsOcclusionCullingEnabled() const {
        return renderContext.occlusionCullingEnabled;
    }

    bool SceneRenderer::Initialize(SDL_Window* pWindow, ERenderingDeviceType deviceType) {
        if (!create_renderer_internal(deviceType, &rendering_device)) {
            spdlog::error("Engine::Initialize Failed to create Rendering Device.");
            return false;
        }

        if (!rendering_device->Initialize(pWindow)) {
            spdlog::error("Engine::Initialize Failed to initialize Rendering Device.");
            return false;
        }

        quad = Mesh::CreateQuad();

        spdlog::info("SceneRenderer::Initialize Scene Renderer initialized successfully.");
        return true;
    }

    void SceneRenderer::Present() {
        rendering_device->SwapChain();
    }

    void SceneRenderer::BeginFrame(const glm::vec4& color) {
        if (world_environment_command.environmentComponent) {
            EWorldEnvironmentMode envMode = world_environment_command.environmentComponent->GetEnvironmentMode();

            if (envMode == EWorldEnvironmentMode::WORLD_ENVIRONMENT_MODE_CLEAR_COLOR
                || envMode == EWorldEnvironmentMode::WORLD_ENVIRONMENT_MODE_CUSTOM_COLOR) {
                rendering_device->ClearColor(world_environment_command.environmentComponent->GetClearColor());
            } else if (envMode == EWorldEnvironmentMode::WORLD_ENVIRONMENT_MODE_SKYBOX) {
                rendering_device->ClearColor(glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));
            }
        } else {
            rendering_device->ClearColor(color);
        }

        rendering_device->ClearBuffer(EClearFlags::CLEAR_COLOR | EClearFlags::CLEAR_DEPTH);
    }

    void SceneRenderer::EndFrame() {
        command_queue.clear();
        command_queue_2d.clear();
        world_canvas_commands.clear();
        canvas_commands.clear();
        directional_lights.clear();
        point_lights.clear();
        spot_lights.clear();
    }

    void SceneRenderer::SetShadowEnabled(bool enabled) {
        renderContext.shadowsEnabled = enabled;
    }

    bool SceneRenderer::AreShadowsEnabled() const {
        return renderContext.shadowsEnabled;
    }

    void SceneRenderer::SetImageBasedLightingEnabled(bool enabled) {
        renderContext.iblEnabled = enabled;
    }
    
    bool SceneRenderer::IsImageBasedLightingEnabled() const {
        return renderContext.iblEnabled;
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

    void SceneRenderer::Submit(const DirectionalLightCommand& command) {
        directional_lights.push_back(command);
    }

    void SceneRenderer::Submit(const PointLightCommand& command) {
        point_lights.push_back(command);
    }

    void SceneRenderer::Submit(const SpotLightCommand& command) {
        spot_lights.push_back(command);
    }

    void SceneRenderer::Submit(const WorldEnvironmentCommand& command) {
        world_environment_command = command;
    }

}; // namespace golias
