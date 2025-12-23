#include "game.h"

bool SandboxApplication::Initialize() {

    std::string vertex_source = R"(
#version 330 core
layout(location = 0) in vec3 a_pos;
layout(location = 1) in vec3 a_color;
layout(location = 2) in vec2 a_texcoord;

out vec3 v_color;
out vec2 v_texcoord;

void main() {
    gl_Position = vec4(a_pos, 1.0);
    v_color = a_color;
    v_texcoord = a_texcoord;
}
)";

    std::string fragment_source = R"(
#version 330 core
in vec3 v_color;
in vec2 v_texcoord;

out vec4 frag_color;

void main() {
    frag_color = vec4(v_color, 1.0);
}
)";

    auto rd = golias::Engine::GetInstance().GetRenderingDevice();
    auto shader           = rd->CreateShaderFromSource(vertex_source, fragment_source);

    material.SetShader(shader);

    std::vector<float> vertices = {
        // positions        // colors         // texcoords
        -0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f,
         0.5f, -0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f,
         0.0f,  0.5f, 0.0f, 0.0f, 0.0f, 1.0f, 0.5f, 1.0f
    };

    std::vector<uint32_t> indices = {
        0, 1, 2
    };

    golias::VertexLayout layout;
    layout.elements = {
        {0, 3, EDataType::FLOAT, false, 0},
        {1, 3, EDataType::FLOAT, false, 3 * sizeof(float)},
        {2, 2, EDataType::FLOAT, false, 6 * sizeof(float)}
    };

    layout.stride = 8 * sizeof(float);

    mesh = rd->CreateMeshFromData(layout, vertices, indices);

    spdlog::info("GameApplication Initialized successfully.");
    return true;
}

void SandboxApplication::Update(float deltaTime) {
    auto& input = golias::Engine::GetInstance().GetInputManager();
   
    if (input.IsActionJustPressed("Jump")) {
        spdlog::info("Jump action just pressed!");
    }

    golias::DrawCommand command;
    command.mesh     = mesh.get();
    command.material = &material;

    auto& rendering_canvas = golias::Engine::GetInstance().GetRenderingCanvas();
    rendering_canvas.Submit(command);

}

void SandboxApplication::Destroy() {

    spdlog::info("GameApplication Destroy called.");
}
