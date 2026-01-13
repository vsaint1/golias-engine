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

    GLenum SceneRenderer::ConvertDepthFunc(EComparisonFunc func) {
        switch (func) {
        case EComparisonFunc::COMPARISON_NEVER:
            return GL_NEVER;
        case EComparisonFunc::COMPARISON_LESS:
            return GL_LESS;
        case EComparisonFunc::COMPARISON_EQUAL:
            return GL_EQUAL;
        case EComparisonFunc::COMPARISON_LESS_EQUAL:
            return GL_LEQUAL;
        case EComparisonFunc::COMPARISON_GREATER:
            return GL_GREATER;
        case EComparisonFunc::COMPARISON_NOT_EQUAL:
            return GL_NOTEQUAL;
        case EComparisonFunc::COMPARISON_GREATER_EQUAL:
            return GL_GEQUAL;
        case EComparisonFunc::COMPARISON_ALWAYS:
            return GL_ALWAYS;
        default:
            return GL_LESS;
        }
    }

    void SceneRenderer::ApplyBlendMode(EBlendMode mode) {
        switch (mode) {
        case EBlendMode::BLEND_MODE_OPAQUE:
            glDisable(GL_BLEND);
            break;
        case EBlendMode::BLEND_MODE_ALPHA:
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            break;
        case EBlendMode::BLEND_MODE_ADDITIVE:
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE);
            break;
        case EBlendMode::BLEND_MODE_MULTIPLY:
            glEnable(GL_BLEND);
            glBlendFunc(GL_DST_COLOR, GL_ZERO);
            break;
        }
    }

    void SceneRenderer::ApplyMaterialState(Material* material) {

        if (!material) {
            return;
        }

        // Depth test
        if (material->IsDepthTestEnabled()) {
            glEnable(GL_DEPTH_TEST);
            glDepthFunc(ConvertDepthFunc(material->GetDepthFunc()));
        } else {
            glDisable(GL_DEPTH_TEST);
        }

        // Depth write
        glDepthMask(material->IsDepthWriteEnabled() ? GL_TRUE : GL_FALSE);

        // Blend mode
        ApplyBlendMode(material->GetBlendMode());

        // Cull mode
        switch (material->GetCullMode()) {
        case ECullMode::CULL_NONE:
            glDisable(GL_CULL_FACE);
            break;
        case ECullMode::CULL_FRONT:
            glEnable(GL_CULL_FACE);
            glCullFace(GL_FRONT);
            break;
        case ECullMode::CULL_BACK:
            glEnable(GL_CULL_FACE);
            glCullFace(GL_BACK);
            break;
        default:
            glEnable(GL_CULL_FACE);
            glCullFace(GL_BACK);
            break;
        }
    }

    void SceneRenderer::RenderObject(const DrawCommand& command, const CameraCommand& camera, bool shadowsEnabled) {
        rendering_device->BindMaterial(command.material);

        auto shader = command.material->GetShader();
        shader->SetUniform("MODEL_MATRIX", command.modelMatrix);
        shader->SetUniform("VIEW_MATRIX", camera.viewMatrix);
        shader->SetUniform("PROJECTION_MATRIX", camera.projectionMatrix);
        shader->SetUniform("CAMERA_POSITION", camera.position);

        // Set alpha clip threshold
        shader->SetUniform("ALPHA_CLIP_THRESHOLD", command.material->GetAlphaClipThreshold());

        if (world_environment_command.environmentComponent) {
            shader->SetUniform("u_tonemap", static_cast<int>(world_environment_command.environmentComponent->GetToneMappingMode()));
            shader->SetUniform("u_exposure", world_environment_command.environmentComponent->GetExposure());
        } else {
            shader->SetUniform("u_tonemap", static_cast<int>(EToneMappingMode::TONE_MAPPING_LINEAR));
            shader->SetUniform("u_exposure", 1.0f);
        }

        if (command.skeletonAnimation && command.skeletonAnimation->GetSkeleton()) {
            shader->SetUniform("USE_SKINNING", true);
            const auto& jointMatrices = command.skeletonAnimation->GetJointMatrices();
            shader->SetUniform("BONE_MATRICES", jointMatrices.data(), static_cast<int>(jointMatrices.size()));
        } else {
            shader->SetUniform("USE_SKINNING", false);
        }

        shader->SetUniform("u_specularStrength", 0.5f);
        shader->SetUniform("u_shininess", 32.0f);

        // Set directional lights
        int directionalLightCount = static_cast<int>(directional_lights.size());
        shader->SetUniform("u_directionalLightCount", directionalLightCount);
        for (int i = 0; i < directionalLightCount; ++i) {
            std::string p = "u_directionalLights[" + std::to_string(i) + "].";
            shader->SetUniform(p + "direction", directional_lights[i].direction);
            shader->SetUniform(p + "color", directional_lights[i].color);
            shader->SetUniform(p + "intensity", directional_lights[i].intensity);
            shader->SetUniform(p + "castShadows", directional_lights[i].castShadows);
        }

        // Set point lights
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

        int spotLightCount = static_cast<int>(spot_lights.size());
        shader->SetUniform("u_spotLightCount", spotLightCount);
        for (int i = 0; i < spotLightCount; ++i) {
            std::string p = "u_spotLights[" + std::to_string(i) + "].";
            shader->SetUniform(p + "position", spot_lights[i].position);
            shader->SetUniform(p + "direction", spot_lights[i].direction);
            shader->SetUniform(p + "color", spot_lights[i].color);
            shader->SetUniform(p + "intensity", spot_lights[i].intensity);
            shader->SetUniform(p + "range", spot_lights[i].range);
            shader->SetUniform(p + "innerConeAngle", glm::cos(glm::radians(spot_lights[i].innerConeAngle)));
            shader->SetUniform(p + "outerConeAngle", glm::cos(glm::radians(spot_lights[i].outerConeAngle)));
            shader->SetUniform(p + "constant", spot_lights[i].constant);
            shader->SetUniform(p + "linear", spot_lights[i].linear);
            shader->SetUniform(p + "quadratic", spot_lights[i].quadratic);
        }

        if (shadowsEnabled && !directional_lights.empty()) {
            glActiveTexture(GL_TEXTURE15);
            glBindTexture(GL_TEXTURE_2D, rendering_device->GetDefaultShadowMapFramebuffer()->GetDepthAttachmentHandle());
            shader->SetUniform("SHADOW_MAP", 15);
            shader->SetUniform("LIGHT_SPACE_MATRIX", directional_lights[0].lightSpaceMatrix);
        }

        bool useIBL =
            command.material->UseImageBasedLighting() && world_environment_command.environmentComponent
            && world_environment_command.environmentComponent->GetTextureCubemap()
            && world_environment_command.environmentComponent->GetEnvironmentMode() == EWorldEnvironmentMode::WORLD_ENVIRONMENT_MODE_SKYBOX;

        if (useIBL) {
            glActiveTexture(GL_TEXTURE13);
            glBindTexture(GL_TEXTURE_CUBE_MAP, world_environment_command.environmentComponent->GetTextureCubemap()->GetNativeHandle());
            shader->SetUniform("IRRADIANCE_MAP", 13);

            glActiveTexture(GL_TEXTURE14);
            glBindTexture(GL_TEXTURE_CUBE_MAP, world_environment_command.environmentComponent->GetTextureCubemap()->GetNativeHandle());
            shader->SetUniform("PREFILTER_MAP", 14);

            shader->SetUniform("USE_IBL", true);
            shader->SetUniform("AMBIENT_STRENGTH", 0.3f);
        } else {
            shader->SetUniform("USE_IBL", false);
            shader->SetUniform("AMBIENT_STRENGTH", 0.5f);

        }

        glActiveTexture(GL_TEXTURE0);

        rendering_device->BindMesh(command.mesh);
        rendering_device->DrawMesh(command.mesh);
    }

    void SceneRenderer::Draw(const CameraCommand& camera) {

        glFrontFace(GL_CCW);
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LESS);
        glDepthMask(GL_TRUE);

        // =========================================================
        // Calculate light space matrices for shadow casting
        // =========================================================
        bool anyShadowCaster = false;
        for (int i = 0; i < directional_lights.size(); ++i) {
            if (directional_lights[i].castShadows) {
                anyShadowCaster = true;

                float orthoSize  = 30.0f;
                float near_plane = -50.0f;
                float far_plane  = 100.0f;

                glm::mat4 lightProjection = glm::ortho(-orthoSize, orthoSize, -orthoSize, orthoSize, near_plane, far_plane);

                glm::vec3 lightDir = glm::normalize(-directional_lights[i].direction);

                glm::mat4 lightView = glm::lookAt(camera.position + lightDir * 50.0f, camera.position, glm::vec3(0.0f, 1.0f, 0.0f));

                directional_lights[i].lightSpaceMatrix = lightProjection * lightView;
                break;
            }
        }

        // =========================================================
        // SHADOW PASS (depth-only) - Only render opaque objects
        // =========================================================
        if (anyShadowCaster) {
            glEnable(GL_DEPTH_TEST);
            glDepthMask(GL_TRUE);
            glDisable(GL_BLEND);
            glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
            

            rendering_device->BeginShadowPass();

            auto shadowShader = rendering_device->GetDefaultShadowMapShader();
            shadowShader->Bind();

            for (size_t i = 0; i < directional_lights.size(); ++i) {
                if (!directional_lights[i].castShadows) {
                    continue;
                }

                shadowShader->SetUniform("LIGHT_SPACE_MATRIX", directional_lights[i].lightSpaceMatrix);

                for (const auto& command : command_queue) {
                    if (!command.mesh || !command.material) {
                        continue;
                    }


                    if (command.material->IsTransparent()) {
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

            rendering_device->EndShadowPass();

            
            glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
        }

        // =========================================================
        // Separate opaque and transparent objects
        // =========================================================
        std::vector<DrawCommand> opaqueCommands;
        std::vector<DrawCommand> transparentCommands;

        for (const auto& command : command_queue) {
            if (!command.mesh || !command.material) {
                continue;
            }

            if (command.material->IsTransparent()) {
                transparentCommands.push_back(command);
            } else {
                opaqueCommands.push_back(command);
            }
        }

        // =========================================================
        // OPAQUE PASS
        // =========================================================
        for (const auto& command : opaqueCommands) {
            ApplyMaterialState(command.material);
            RenderObject(command, camera, anyShadowCaster);
        }

        // =========================================================
        // SKYBOX PASS
        // =========================================================
        if (world_environment_command.environmentComponent
            && world_environment_command.environmentComponent->GetEnvironmentMode()
                   == EWorldEnvironmentMode::WORLD_ENVIRONMENT_MODE_SKYBOX) {

            auto skyboxMesh    = world_environment_command.environmentComponent->GetSkyboxMesh();
            auto skyboxTexture = world_environment_command.environmentComponent->GetTextureCubemap();
            auto skyboxShader  = rendering_device->GetDefaultSkyboxShader();

            if (skyboxShader && skyboxMesh && skyboxTexture) {
                glDepthFunc(GL_LEQUAL);
                glDepthMask(GL_FALSE);
                glDisable(GL_BLEND);
                glDisable(GL_CULL_FACE);

                skyboxShader->Bind();

                glm::mat4 viewNoTranslation = glm::mat4(glm::mat3(camera.viewMatrix));

                skyboxShader->SetUniform("VIEW_MATRIX", viewNoTranslation);
                skyboxShader->SetUniform("PROJECTION_MATRIX", camera.projectionMatrix);
                skyboxShader->SetUniform("EXPOSURE", world_environment_command.environmentComponent->GetExposure());

                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxTexture->GetNativeHandle());
                skyboxShader->SetUniform("SKYBOX", 0);

                rendering_device->BindMesh(skyboxMesh.get());
                rendering_device->DrawMesh(skyboxMesh.get());

                glEnable(GL_CULL_FACE);
                glCullFace(GL_BACK);
                glDepthMask(GL_TRUE);
                glDepthFunc(GL_LESS);
            }
        }

        // =========================================================
        // TRANSPARENT PASS - Back to front sorting
        // =========================================================
        if (!transparentCommands.empty()) {
            // Sort transparent objects back-to-front based on distance from camera
            std::sort(transparentCommands.begin(), transparentCommands.end(), [&camera](const DrawCommand& a, const DrawCommand& b) {
                // Extract position from model matrix (4th column)
                glm::vec3 posA = glm::vec3(a.modelMatrix[3]);
                glm::vec3 posB = glm::vec3(b.modelMatrix[3]);

                float distA = glm::distance(camera.position, posA);
                float distB = glm::distance(camera.position, posB);

                // Sort far to near (back to front)
                return distA > distB;
            });

            for (const auto& command : transparentCommands) {
                ApplyMaterialState(command.material);
                RenderObject(command, camera, anyShadowCaster);
            }
        }

        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LESS);
        glDepthMask(GL_TRUE);
        glDisable(GL_BLEND);
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);

        // =========================================================
        // WORLD CANVAS (billboards / decals)
        // =========================================================
        glEnable(GL_DEPTH_TEST);
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

            Uint32 offset = 0;
            for (const auto& batch : command.batches) {

                // shader_canvas->SetUniform("HAS_TEXTURE", batch.texture ? 1 : 0);
                shader_canvas->SetUniform("TEXTURE", batch.texture != nullptr ? batch.texture : rendering_device->GetWhiteTexture2D().get());

                glm::mat4 model = command.modelMatrix * glm::scale(glm::mat4(1.0f), glm::vec3(command.scale));

                shader_canvas->SetUniform("MODEL_MATRIX", model);
                shader_canvas->SetUniform("VIEW_MATRIX", camera.viewMatrix);
                shader_canvas->SetUniform("PROJECTION_MATRIX", camera.projectionMatrix);
                shader_canvas->SetUniform("CAMERA_POSITION", camera.position);
                shader_canvas->SetUniform("USE_BILLBOARDING", true);

                command.mesh->DrawIndexed(offset, batch.indexCount);
                offset += batch.indexCount;
            }
        }

        glDepthMask(GL_TRUE);
        glDisable(GL_BLEND);

        // =========================================================
        // MAIN 2D PASS
        // =========================================================
        glDisable(GL_DEPTH_TEST);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
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
            shader_2d->SetUniform("TEXTURE", command.texture != nullptr ? command.texture : rendering_device->GetWhiteTexture2D().get());

            quad->Draw();
        }
        quad->Unbind();

        // =========================================================
        // SCREEN CANVAS PASS
        // =========================================================
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

        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LESS);
        glDepthMask(GL_TRUE);
        glDisable(GL_BLEND);
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
        glFrontFace(GL_CCW); // Ensure consistent winding
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

    void SceneRenderer::BeginFrame(const glm::vec4& color) {
        if (world_environment_command.environmentComponent) {
            EWorldEnvironmentMode envMode = world_environment_command.environmentComponent->GetEnvironmentMode();

            if (envMode == EWorldEnvironmentMode::WORLD_ENVIRONMENT_MODE_CLEAR_COLOR) {
                rendering_device->Clear(world_environment_command.environmentComponent->GetClearColor());
            } else if (envMode == EWorldEnvironmentMode::WORLD_ENVIRONMENT_MODE_CUSTOM_COLOR) {
                rendering_device->Clear(world_environment_command.environmentComponent->GetClearColor());
            } else if (envMode == EWorldEnvironmentMode::WORLD_ENVIRONMENT_MODE_SKYBOX) {
                rendering_device->Clear(glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));
            }
        } else {
            rendering_device->Clear(color);
        }
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

}; // namespace golias
