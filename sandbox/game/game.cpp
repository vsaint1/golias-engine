#include "game.h"

#include "scene/3d/camera_component.h"
#include "scene/3d/fp_controller_component.h"
#include "scene/3d/mesh_component.h"

bool SandboxApplication::Initialize() {

    scene = new golias::Scene();


    golias::Engine::GetInstance().SetScene(scene);


    std::string vertex_source = R"(
#version 330 core
layout(location = 0) in vec3 a_pos;
layout(location = 1) in vec3 a_color;
layout(location = 2) in vec2 a_texcoord;

out vec3 v_color;
out vec2 v_texcoord;

uniform mat4 MODEL_MATRIX;
uniform mat4 PROJECTION_MATRIX;
uniform mat4 VIEW_MATRIX;

void main() {
    v_color = a_color;
    v_texcoord = a_texcoord;
    gl_Position = PROJECTION_MATRIX * VIEW_MATRIX * MODEL_MATRIX * vec4(a_pos, 1.0);
}
)";

    std::string fragment_source = R"(
#version 330 core
in vec3 v_color;
in vec2 v_texcoord;

out vec4 COLOR;

void main() {
    COLOR = vec4(v_color, 1.0);
}
)";

    auto rd     = golias::Engine::GetInstance().GetRenderingDevice();
    auto shader = rd->CreateShaderFromSource(vertex_source, fragment_source);

    auto material = std::make_shared<golias::Material>();
    material->SetShader(shader);


    std::vector<float> box_vertices = {
        // pos              // color           // uv
        -0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, // 0
        0.5f,  -0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, // 1
        0.5f,  0.5f,  -0.5f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, // 2
        -0.5f, 0.5f,  -0.5f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, // 3

        -0.5f, -0.5f, 0.5f,  1.0f, 0.0f, 1.0f, 0.0f, 0.0f, // 4
        0.5f,  -0.5f, 0.5f,  0.0f, 1.0f, 1.0f, 1.0f, 0.0f, // 5
        0.5f,  0.5f,  0.5f,  1.0f, 1.0f, 1.0f, 1.0f, 1.0f, // 6
        -0.5f, 0.5f,  0.5f,  1.0f, 0.0f, 0.5f, 0.0f, 1.0f // 7
    };

    std::vector<Uint32> box_indices = {
        0, 1, 2, 2, 3, 0, // back face
        4, 5, 6, 6, 7, 4, // front face
        0, 4, 7, 7, 3, 0, // left face
        1, 5, 6, 6, 2, 1, // right face
        3, 2, 6, 6, 7, 3, // top face
        0, 1, 5, 5, 4, 0 // bottom face
    };


    golias::VertexLayout layout;
    layout.elements = {{0, 3, EDataType::FLOAT, false, 0},
                       {1, 3, EDataType::FLOAT, false, 3 * sizeof(float)},
                       {2, 2, EDataType::FLOAT, false, 6 * sizeof(float)}};

    layout.stride = 8 * sizeof(float);

    auto mesh = rd->CreateMeshFromData(layout, box_vertices, box_indices);




    auto camera = scene->CreateObject<golias::GameObject>("Camera");
    camera->AddComponent(new golias::CameraComponent());
    camera->AddComponent(new golias::FirstPersonControllerComponent());
    camera->SetPosition({0.0f, 0.0f, 5.0f});
    scene->SetMainCamera(camera);


    const int grid_size = 10;
    for  (int x = -grid_size / 2; x < grid_size / 2; ++x) {
        for (int y = -grid_size / 2; y < grid_size / 2; ++y) {
            auto obj = scene->CreateObject(std::string("Box_") + std::to_string(x) + "_" + std::to_string(y));
            obj->SetPosition({static_cast<float>(x), static_cast<float>(y), 0.0f});
            auto meshComp = new golias::MeshComponent();
            obj->AddComponent(new golias::MeshComponent(mesh,material));
        }
    }


    spdlog::info("GameApplication Initialized successfully.");
    return true;
}

void SandboxApplication::Update(float deltaTime) {
    auto& input = golias::Engine::GetInstance().GetInputManager();

    if (input.IsActionJustPressed("Jump")) {
        spdlog::info("Jump action just pressed!");
    }

    scene->Update(deltaTime);
}

void SandboxApplication::Destroy() {

    spdlog::info("GameApplication Destroy called.");
}
