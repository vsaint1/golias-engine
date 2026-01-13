#include "scene/3d/world_environment_component.h"

#include "core/engine.h"

namespace golias {

    void WorldEnvironmentComponent::SetSkyboxCubemap(const std::shared_ptr<TextureCubemap>& cubemap) {
        texture = cubemap;
    }

    std::shared_ptr<TextureCubemap> WorldEnvironmentComponent::GetTextureCubemap() const {
        return texture;
    }


    EToneMappingMode WorldEnvironmentComponent::GetToneMappingMode() const {
        return toneMappingMode;
    }

    void WorldEnvironmentComponent::SetToneMappingMode(EToneMappingMode mode) {
        toneMappingMode = mode;
    }


    void WorldEnvironmentComponent::Start() {
        std::vector<float> vertices = {
            -1.0f, 1.0f,  -1.0f, -1.0f, -1.0f, -1.0f, 1.0f,  -1.0f, -1.0f, 1.0f,  -1.0f, -1.0f, 1.0f,  1.0f,  -1.0f, -1.0f, 1.0f,  -1.0f,
            -1.0f, -1.0f, 1.0f,  -1.0f, -1.0f, -1.0f, -1.0f, 1.0f,  -1.0f, -1.0f, 1.0f,  -1.0f, -1.0f, 1.0f,  1.0f,  -1.0f, -1.0f, 1.0f,
            1.0f,  -1.0f, -1.0f, 1.0f,  -1.0f, 1.0f,  1.0f,  1.0f,  1.0f,  1.0f,  1.0f,  1.0f,  1.0f,  1.0f,  -1.0f, 1.0f,  -1.0f, -1.0f,
            -1.0f, -1.0f, 1.0f,  -1.0f, 1.0f,  1.0f,  1.0f,  1.0f,  1.0f,  1.0f,  1.0f,  1.0f,  1.0f,  -1.0f, 1.0f,  -1.0f, -1.0f, 1.0f,
            -1.0f, 1.0f,  -1.0f, 1.0f,  1.0f,  -1.0f, 1.0f,  1.0f,  1.0f,  1.0f,  1.0f,  1.0f,  -1.0f, 1.0f,  1.0f,  -1.0f, 1.0f,  -1.0f,
            -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, 1.0f,  1.0f,  -1.0f, -1.0f, 1.0f,  -1.0f, -1.0f, -1.0f, -1.0f, 1.0f,  1.0f,  -1.0f, 1.0f};

        std::vector<unsigned int> indices = {
            0,  1,  2,  3,  4,  5, // -Z
            6,  7,  8,  9,  10, 11, // -X
            12, 13, 14, 15, 16, 17, // +X
            18, 19, 20, 21, 22, 23, // +Z
            24, 25, 26, 27, 28, 29, // +Y
            30, 31, 32, 33, 34, 35 // -Y
        };

        VertexLayout layout;
        layout.elements = {
            {0, 3, EDataType::FLOAT, false, 0}  // position
        };

        auto rd     = Engine::GetInstance().GetSceneRenderer().GetRenderingDevice();
        skybox_mesh = rd->CreateMeshFromData(layout, vertices, indices);

        spdlog::info("WorldEnvironmentComponent::Start Created skybox Mesh");
    }

    void WorldEnvironmentComponent::Update(float deltaTime) {
      
        WorldEnvironmentCommand command;
        command.environmentComponent = this;
        Engine::GetInstance().GetSceneRenderer().Submit(command);
    }


    float WorldEnvironmentComponent::GetExposure() const {
        return exposure_;
    }

    void WorldEnvironmentComponent::SetExposure(float exp) {
        exposure_ = exp;
    }

    glm::vec4 WorldEnvironmentComponent::GetClearColor() const {
        return clearColor;
    }

    void WorldEnvironmentComponent::SetClearColor(const glm::vec4& color) {
        clearColor = color;
    }

    EWorldEnvironmentMode WorldEnvironmentComponent::GetEnvironmentMode() const {
        return environmentMode;
    }

    void WorldEnvironmentComponent::SetEnvironmentMode(EWorldEnvironmentMode mode) {
        environmentMode = mode;
    }

    std::shared_ptr<Mesh> WorldEnvironmentComponent::GetSkyboxMesh() const {
        return skybox_mesh;
    }


} // namespace golias
