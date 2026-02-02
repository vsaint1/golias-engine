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
        UpdateUIScale();
        
        renderContext.camera = camera;

        CalculateLightSpaceMatrices();

        ShadowPass();

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
        
        BeginMainRenderPass();
        
        GeometryOpaquePass(opaqueCommands);
        SkyboxPass();
        GeometryTransparentPass(transparentCommands);
        WorldCanvasPass();
        Sprite2DPass();
        ScreenCanvasPass();
        
        rendering_device->EndRenderPass();
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
            int count = std::min<int>(jointMatrices.size(), 128);
            shader->SetUniform("BONE_MATRICES", jointMatrices.data(), count);
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
            
            
            // Bind all cascade shadow maps
            for (int i = 0; i < DirectionalLightCommand::NUM_CASCADES; ++i) {
                std::string uniformName = "SHADOW_MAP_CASCADE_" + std::to_string(i);
                rendering_device->BindTexture(
                    shader, uniformName, 6 + i, 
                    rendering_device->GetCascadeShadowMapFramebuffer(i)->GetDepthAttachment().get());
            }
            
            // Set cascade matrices
            for (int i = 0; i < DirectionalLightCommand::NUM_CASCADES; ++i) {
                std::string uniformName = "LIGHT_SPACE_MATRIX_CASCADE_" + std::to_string(i);
                shader->SetUniform(uniformName, directional_lights[0].lightSpaceMatrices[i]);
            }
            
            // Set cascade split distances as individual floats
            for (int i = 0; i < DirectionalLightCommand::NUM_CASCADES; ++i) {
                std::string uniformName = "CASCADE_SPLIT_" + std::to_string(i);
                shader->SetUniform(uniformName, directional_lights[0].cascadeSplits[i]);
            }

            shader->SetUniform("DEBUG_CASCADES", renderContext.debugCascades);
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

            const float camera_near = 0.1f;
            const float camera_far = 150.0f;
            
            // Calculate cascade split distances using practical split scheme
            // Blend between logarithmic and uniform distribution
            const float lambda = 0.75f; // Blending factor
            
            for (int i = 0; i < DirectionalLightCommand::NUM_CASCADES; ++i) {
                float p = static_cast<float>(i + 1) / DirectionalLightCommand::NUM_CASCADES;
                float log_split = camera_near * std::pow(camera_far / camera_near, p);
                float uniform_split = camera_near + (camera_far - camera_near) * p;
                
                directional_light.cascadeSplits[i] = lambda * log_split + (1.0f - lambda) * uniform_split;
            }
            
        

            glm::vec3 lightDir = glm::normalize(directional_light.direction);
            
            float lastSplitDist = camera_near;
            for (int i = 0; i < DirectionalLightCommand::NUM_CASCADES; ++i) {
                float splitDist = directional_light.cascadeSplits[i];
                
                glm::vec3 frustumCorners[8];
                CalculateFrustumCorners(renderContext.camera.projectionMatrix,
                                       renderContext.camera.viewMatrix,
                                       lastSplitDist,
                                       splitDist,
                                       frustumCorners);
                
                // Calculate frustum center
                glm::vec3 frustumCenter = glm::vec3(0.0f);
                for (int j = 0; j < 8; ++j) {
                    frustumCenter += frustumCorners[j];
                }
                frustumCenter /= 8.0f;
                
                // Calculate radius (bounding sphere of frustum)
                float radius = 0.0f;
                for (int j = 0; j < 8; ++j) {
                    float distance = glm::length(frustumCorners[j] - frustumCenter);
                    radius = std::max(radius, distance);
                }
                
                // Round radius to reduce shimmering
                radius = std::ceil(radius * 16.0f) / 16.0f;
                
                // Calculate light view matrix
                glm::vec3 lightPos = frustumCenter - lightDir * radius;
                glm::mat4 lightView = glm::lookAt(lightPos, frustumCenter, glm::vec3(0.0f, 1.0f, 0.0f));
                
                // Calculate orthographic projection
                glm::mat4 lightProjection = glm::ortho(-radius, radius, -radius, radius, 0.0f, radius * 2.0f);
                
                // Stabilize shadow maps by snapping to texel grid
                glm::mat4 shadowMatrix = lightProjection * lightView;
                glm::vec4 shadowOrigin = shadowMatrix * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
                const float texelSize = (radius * 2.0f) / 2048.0f; // Assuming 2048 resolution
                shadowOrigin *= (1.0f / texelSize);
                shadowOrigin = glm::floor(shadowOrigin);
                shadowOrigin *= texelSize;
                
                glm::vec4 roundOffset = shadowOrigin - (shadowMatrix * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));
                lightProjection[3][0] += roundOffset.x;
                lightProjection[3][1] += roundOffset.y;
                
                directional_light.lightSpaceMatrices[i] = lightProjection * lightView;
                
                lastSplitDist = splitDist;
            }
        }
    }

    void SceneRenderer::CalculateFrustumCorners(const glm::mat4& proj, const glm::mat4& view, 
                                                float nearPlane, float farPlane, 
                                                glm::vec3 frustumCorners[8]) {
        glm::mat4 invViewProj = glm::inverse(proj * view);
        
        // For perspective projection: ndc_z = (far + near) / (far - near) + (2 * far * near) / ((far - near) * view_z)
        
        // Get camera near and far from projection matrix
        float cam_near = proj[3][2] / (proj[2][2] - 1.0f);
        float cam_far = proj[3][2] / (proj[2][2] + 1.0f);
        
        // Convert view-space depths to NDC Z
        float ndcNear, ndcFar;
        
        if (proj[3][3] == 0.0f) {
            // Perspective projection
            ndcNear = (cam_far + cam_near) / (cam_far - cam_near) + (2.0f * cam_far * cam_near) / ((cam_far - cam_near) * -nearPlane);
            ndcFar = (cam_far + cam_near) / (cam_far - cam_near) + (2.0f * cam_far * cam_near) / ((cam_far - cam_near) * -farPlane);
        } else {
            // Orthographic projection
            ndcNear = (2.0f * nearPlane - cam_far - cam_near) / (cam_far - cam_near);
            ndcFar = (2.0f * farPlane - cam_far - cam_near) / (cam_far - cam_near);
        }
        
        int index = 0;
        for (int z = 0; z < 2; ++z) {
            for (int y = 0; y < 2; ++y) {
                for (int x = 0; x < 2; ++x) {
                    glm::vec4 corner = invViewProj * glm::vec4(
                        2.0f * x - 1.0f,
                        2.0f * y - 1.0f,
                        z == 0 ? ndcNear : ndcFar,
                        1.0f
                    );
                    frustumCorners[index++] = glm::vec3(corner) / corner.w;
                }
            }
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

        rendering_device->ApplyPipelineState(pipeline_shadow);

        auto shadowShader = rendering_device->GetDefaultShadowMapShader();
        rendering_device->BindShader(shadowShader.get());

        for (int cascadeIndex = 0; cascadeIndex < DirectionalLightCommand::NUM_CASCADES; ++cascadeIndex) {
            for (const auto& directional_light : directional_lights) {
                if (!directional_light.castShadows) {
                    continue;
                }

                auto cascadeFBO = rendering_device->GetCascadeShadowMapFramebuffer(cascadeIndex);
                
                RenderPassBeginInfo rpInfo;
                rpInfo.framebuffer = cascadeFBO.get();
                rpInfo.colorLoadOp = ELoadOp::DONT_CARE;
                rpInfo.colorStoreOp = EStoreOp::DONT_CARE;
                rpInfo.depthLoadOp = ELoadOp::CLEAR;
                rpInfo.depthStoreOp = EStoreOp::STORE;
                rpInfo.clearValues.push_back(ClearValue::DepthStencil(1.0f, 0));

                const auto& spec = rpInfo.framebuffer->GetSpecification();
                rpInfo.viewport.x = 0;
                rpInfo.viewport.y = 0;
                rpInfo.viewport.width = spec.width;
                rpInfo.viewport.height = spec.height;
                rpInfo.viewport.min_depth = 0.0f;
                rpInfo.viewport.max_depth = 1.0f;
                rpInfo.scissor.x = 0;
                rpInfo.scissor.y = 0;
                rpInfo.scissor.width = spec.width;
                rpInfo.scissor.height = spec.height;

                rendering_device->BeginRenderPass(rpInfo);

                // Set light space matrix for this cascade
                shadowShader->SetUniform("LIGHT_SPACE_MATRIX", directional_light.lightSpaceMatrices[cascadeIndex]);

                // Render all shadow casters
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

                rendering_device->EndRenderPass();
                
                break;
            }
        }
    }

    void SceneRenderer::BeginMainRenderPass() {
        RenderPassBeginInfo rpInfo;
        rpInfo.framebuffer = nullptr; // nullptr means default framebuffer (screen)
        rpInfo.colorLoadOp = ELoadOp::CLEAR;
        rpInfo.colorStoreOp = EStoreOp::STORE;
        rpInfo.depthLoadOp = ELoadOp::CLEAR;
        rpInfo.depthStoreOp = EStoreOp::STORE;
        
        if (world_environment_command.environmentComponent) {
            EWorldEnvironmentMode envMode = world_environment_command.environmentComponent->GetEnvironmentMode();
            if (envMode == EWorldEnvironmentMode::WORLD_ENVIRONMENT_MODE_CLEAR_COLOR ||
                envMode == EWorldEnvironmentMode::WORLD_ENVIRONMENT_MODE_CUSTOM_COLOR) {
                glm::vec4 clearColor = world_environment_command.environmentComponent->GetClearColor();
                rpInfo.clearValues.push_back(ClearValue::Color(clearColor));
            } else {
                rpInfo.clearValues.push_back(ClearValue::Color(0.0f, 0.0f, 0.0f, 1.0f));
            }
        } else {
            rpInfo.clearValues.push_back(ClearValue::Color(renderContext.clearColor));
        }
        rpInfo.clearValues.push_back(ClearValue::DepthStencil(1.0f, 0));
        
        int windowWidth, windowHeight;
        SDL_GetWindowSize(rendering_device->GetWindow(), &windowWidth, &windowHeight);
        
        
        rpInfo.viewport.x = 0;
        rpInfo.viewport.y = 0;
        rpInfo.viewport.width = windowWidth;
        rpInfo.viewport.height = windowHeight;
        rpInfo.viewport.min_depth = 0.0f;
        rpInfo.viewport.max_depth = 1.0f;
        
        rpInfo.scissor.x = 0;
        rpInfo.scissor.y = 0;
        rpInfo.scissor.width = windowWidth;
        rpInfo.scissor.height = windowHeight;
        
        rendering_device->BeginRenderPass(rpInfo);
    }


    void SceneRenderer::EndMainRenderPass() {
    }

    void SceneRenderer::GeometryOpaquePass(const std::vector<DrawCommand>& opaqueCommands) {

        rendering_device->ApplyPipelineState(pipeline_opaque);

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

        rendering_device->ApplyPipelineState(pipeline_skybox);

        rendering_device->BindShader(skyboxShader.get());

        glm::mat4 viewNoTranslation = glm::mat4(glm::mat3(renderContext.camera.viewMatrix));
        skyboxShader->SetUniform("VIEW_MATRIX", viewNoTranslation);
        skyboxShader->SetUniform("PROJECTION_MATRIX", renderContext.camera.projectionMatrix);
        skyboxShader->SetUniform("EXPOSURE", world_environment_command.environmentComponent->GetExposure());

        rendering_device->BindCubemap(skyboxShader.get(), "SKYBOX", 0, skyboxTexture.get());

        rendering_device->BindMesh(skyboxMesh.get());
        rendering_device->DrawMesh(skyboxMesh.get());
    }

    void SceneRenderer::GeometryTransparentPass(const std::vector<DrawCommand>& transparentCommands) {
        rendering_device->ApplyPipelineState(pipeline_transparent);

        for (const auto& command : transparentCommands) {
            RenderObject(command);
        }
    }

    void SceneRenderer::WorldCanvasPass() {
        if (world_canvas_commands.empty()) {
            return;
        }

        rendering_device->ApplyPipelineState(pipeline_canvas);

        auto shader_canvas = rendering_device->GetDefaultShaderCanvas();
        rendering_device->BindShader(shader_canvas.get());

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
    }

    void SceneRenderer::Sprite2DPass() {
        if (command_queue_2d.empty()) {
            return;
        }

        rendering_device->ApplyPipelineState(pipeline_2d);

        const auto shader_2d = rendering_device->GetDefaultShader2D();
        rendering_device->BindShader(shader_2d.get());

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

        rendering_device->ApplyPipelineState(pipeline_canvas);

        auto shader_canvas = rendering_device->GetDefaultShaderCanvas();
        rendering_device->BindShader(shader_canvas.get());

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

        InitializePipelines();

        spdlog::info("SceneRenderer::Initialize Scene Renderer initialized successfully.");
        return true;
    }

    void SceneRenderer::InitializePipelines() {
        pipeline_opaque = CreateOpaquePipeline();
        pipeline_transparent = CreateTransparentPipeline();
        pipeline_shadow = CreateShadowPipeline();
        pipeline_skybox = CreateSkyboxPipeline();
        pipeline_canvas = CreateCanvasPipeline();
        pipeline_2d = Create2DPipeline();
    }

    PipelineState SceneRenderer::CreateOpaquePipeline() {
        PipelineState state;
        
        // Rasterizer
        state.rasterizer.cullMode = ECullMode::CULL_MODE_BACK;
        state.rasterizer.polygonMode = EPolygonMode::FILL;
        state.rasterizer.frontFaceCCW = true;
        
        // Depth-stencil
        state.depthStencil.depthTestEnable = true;
        state.depthStencil.depthWriteEnable = true;
        state.depthStencil.depthFunc = EComparisonFunc::COMPARISON_LESS;
        
        // Blend (disabled for opaque)
        state.blend.attachments[0].blendEnable = false;
        
        state.topology = EPrimitiveTopology::TRIANGLES;
        
        return state;
    }

    PipelineState SceneRenderer::CreateTransparentPipeline() {
        PipelineState state;
        
        // Rasterizer
        state.rasterizer.cullMode = ECullMode::CULL_MODE_BACK;
        state.rasterizer.polygonMode = EPolygonMode::FILL;
        state.rasterizer.frontFaceCCW = true;
        
        // Depth-stencil (read but don't write)
        state.depthStencil.depthTestEnable = true;
        state.depthStencil.depthWriteEnable = false;
        state.depthStencil.depthFunc = EComparisonFunc::COMPARISON_LESS;
        
        // Blend (alpha blending)
        state.blend.attachments[0].blendEnable = true;
        state.blend.attachments[0].srcColorBlend = EBlendFactor::BLEND_SRC_ALPHA;
        state.blend.attachments[0].dstColorBlend = EBlendFactor::BLEND_INV_SRC_ALPHA;
        state.blend.attachments[0].colorBlendOp = EBlendOp::BLEND_OP_ADD;
        state.blend.attachments[0].srcAlphaBlend = EBlendFactor::BLEND_ONE;
        state.blend.attachments[0].dstAlphaBlend = EBlendFactor::BLEND_INV_SRC_ALPHA;
        state.blend.attachments[0].alphaBlendOp = EBlendOp::BLEND_OP_ADD;
        
        state.topology = EPrimitiveTopology::TRIANGLES;
        
        return state;
    }

    PipelineState SceneRenderer::CreateShadowPipeline() {
        PipelineState state;
        
        state.rasterizer.cullMode = ECullMode::CULL_MODE_BACK;
        state.rasterizer.polygonMode = EPolygonMode::FILL;
        state.rasterizer.frontFaceCCW = true;
        state.rasterizer.depthBiasEnable = true;
        state.rasterizer.depthBiasConstant = 1.25f;
        state.rasterizer.depthBiasSlopeFactor = 1.75f;
        
        // Depth-stencil
        state.depthStencil.depthTestEnable = true;
        state.depthStencil.depthWriteEnable = true;
        state.depthStencil.depthFunc = EComparisonFunc::COMPARISON_LESS;
        
        // Blend (disabled)
        state.blend.attachments[0].blendEnable = false;
        
        state.topology = EPrimitiveTopology::TRIANGLES;
        
        return state;
    }

    PipelineState SceneRenderer::CreateSkyboxPipeline() {
        PipelineState state;
        
        // Rasterizer (no culling for skybox)
        state.rasterizer.cullMode = ECullMode::CULL_MODE_DISABLED;
        state.rasterizer.polygonMode = EPolygonMode::FILL;
        state.rasterizer.frontFaceCCW = true;
        
        // Depth-stencil (equal test, no write)
        state.depthStencil.depthTestEnable = true;
        state.depthStencil.depthWriteEnable = false;
        state.depthStencil.depthFunc = EComparisonFunc::COMPARISON_LESS_EQUAL;
        
        // Blend (disabled)
        state.blend.attachments[0].blendEnable = false;
        
        state.topology = EPrimitiveTopology::TRIANGLES;
        
        return state;
    }

    PipelineState SceneRenderer::CreateCanvasPipeline() {
        PipelineState state;
        
        // Rasterizer (no culling)
        state.rasterizer.cullMode = ECullMode::CULL_MODE_DISABLED;
        state.rasterizer.polygonMode = EPolygonMode::FILL;
        state.rasterizer.frontFaceCCW = true;
        
        // Depth-stencil (test but don't write for world canvas)
        state.depthStencil.depthTestEnable = true;
        state.depthStencil.depthWriteEnable = false;
        state.depthStencil.depthFunc = EComparisonFunc::COMPARISON_LESS;
        
        // Blend (alpha blending)
        state.blend.attachments[0].blendEnable = true;
        state.blend.attachments[0].srcColorBlend = EBlendFactor::BLEND_SRC_ALPHA;
        state.blend.attachments[0].dstColorBlend = EBlendFactor::BLEND_INV_SRC_ALPHA;
        state.blend.attachments[0].colorBlendOp = EBlendOp::BLEND_OP_ADD;
        state.blend.attachments[0].srcAlphaBlend = EBlendFactor::BLEND_ONE;
        state.blend.attachments[0].dstAlphaBlend = EBlendFactor::BLEND_INV_SRC_ALPHA;
        state.blend.attachments[0].alphaBlendOp = EBlendOp::BLEND_OP_ADD;
        
        state.topology = EPrimitiveTopology::TRIANGLES;
        
        return state;
    }

    PipelineState SceneRenderer::Create2DPipeline() {
        PipelineState state;
        
        // Rasterizer (no culling)
        state.rasterizer.cullMode = ECullMode::CULL_MODE_DISABLED;
        state.rasterizer.polygonMode = EPolygonMode::FILL;
        state.rasterizer.frontFaceCCW = true;
        
        // Depth-stencil (disabled for 2D)
        state.depthStencil.depthTestEnable = false;
        state.depthStencil.depthWriteEnable = false;
        
        // Blend (alpha blending)
        state.blend.attachments[0].blendEnable = true;
        state.blend.attachments[0].srcColorBlend = EBlendFactor::BLEND_SRC_ALPHA;
        state.blend.attachments[0].dstColorBlend = EBlendFactor::BLEND_INV_SRC_ALPHA;
        state.blend.attachments[0].colorBlendOp = EBlendOp::BLEND_OP_ADD;
        state.blend.attachments[0].srcAlphaBlend = EBlendFactor::BLEND_ONE;
        state.blend.attachments[0].dstAlphaBlend = EBlendFactor::BLEND_INV_SRC_ALPHA;
        state.blend.attachments[0].alphaBlendOp = EBlendOp::BLEND_OP_ADD;
        
        state.topology = EPrimitiveTopology::TRIANGLES;
        
        return state;
    }

    void SceneRenderer::Present() {
        rendering_device->SwapChain();
    }

    void SceneRenderer::BeginFrame(const glm::vec4& color) {
        // Store the clear color in render context for use in BeginMainRenderPass
        renderContext.clearColor = color;
        
        // Note: Actual clearing happens in BeginMainRenderPass now,
        // which is called at the start of Draw() after shadow pass
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
    
    void SceneRenderer::SetScreenScaleMode(EScreenScaleMode mode) {
        renderContext.canvasScaling.mode = mode;
    }
    
    EScreenScaleMode SceneRenderer::GetScreenScaleMode() const {
        return renderContext.canvasScaling.mode;
    }
    
    void SceneRenderer::SetReferenceResolution(const glm::vec2& resolution) {
        renderContext.canvasScaling.referenceResolution = resolution;
    }
    
    glm::vec2 SceneRenderer::GetReferenceResolution() const {
        return renderContext.canvasScaling.referenceResolution;
    }
    
    float SceneRenderer::GetUIScale() const {
        return renderContext.canvasScaling.scale;
    }
    
    void SceneRenderer::UpdateUIScale() {
        const auto& viewport = rendering_device->GetViewport();
        
        if (renderContext.canvasScaling.mode == EScreenScaleMode::VIEWPORT) {
            float scaleX = static_cast<float>(viewport.width) / renderContext.canvasScaling.referenceResolution.x;
            float scaleY = static_cast<float>(viewport.height) / renderContext.canvasScaling.referenceResolution.y;
            renderContext.canvasScaling.scale = glm::min(scaleX, scaleY);
        } else {
            renderContext.canvasScaling.scale = 1.0f;
        }
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
