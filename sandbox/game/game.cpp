#include "game.h"

#include "scene/3d/camera_component.h"
#include "scene/3d/fp_controller_component.h"
#include "scene/3d/mesh_component.h"


bool SandboxApplication::Initialize() {

    scene = new golias::Scene();


    golias::Engine::GetInstance().SetScene(scene);

    auto fs = golias::Engine::GetInstance().GetFileSystem();


    std::string vertex_source = fs.LoadAssetFileText("shaders/vertex.glsl");

    std::string fragment_source = fs.LoadAssetFileText("shaders/fragment.glsl");


    auto rd      = golias::Engine::GetInstance().GetRenderingDevice();
    auto shader  = rd->CreateShaderFromSource(vertex_source, fragment_source);
    auto texture = rd->CreateTextureFromFile("textures/brick.png");

    auto material = std::make_shared<golias::Material>();
    material->SetShader(shader);
    material->SetParameter("TEXTURE", texture);


    std::vector<float> box_vertices = {
        // ----- Front -----
        -0.5f,
        -0.5f,
        0.5f,
        1,
        1,
        1,
        0,
        0,
        0.5f,
        -0.5f,
        0.5f,
        1,
        1,
        1,
        1,
        0,
        0.5f,
        0.5f,
        0.5f,
        1,
        1,
        1,
        1,
        1,
        -0.5f,
        0.5f,
        0.5f,
        1,
        1,
        1,
        0,
        1,

        // ----- Back -----
        -0.5f,
        -0.5f,
        -0.5f,
        1,
        1,
        1,
        1,
        0,
        0.5f,
        -0.5f,
        -0.5f,
        1,
        1,
        1,
        0,
        0,
        0.5f,
        0.5f,
        -0.5f,
        1,
        1,
        1,
        0,
        1,
        -0.5f,
        0.5f,
        -0.5f,
        1,
        1,
        1,
        1,
        1,

        // ----- Left -----
        -0.5f,
        -0.5f,
        -0.5f,
        1,
        1,
        1,
        0,
        0,
        -0.5f,
        -0.5f,
        0.5f,
        1,
        1,
        1,
        1,
        0,
        -0.5f,
        0.5f,
        0.5f,
        1,
        1,
        1,
        1,
        1,
        -0.5f,
        0.5f,
        -0.5f,
        1,
        1,
        1,
        0,
        1,

        // ----- Right -----
        0.5f,
        -0.5f,
        -0.5f,
        1,
        1,
        1,
        1,
        0,
        0.5f,
        -0.5f,
        0.5f,
        1,
        1,
        1,
        0,
        0,
        0.5f,
        0.5f,
        0.5f,
        1,
        1,
        1,
        0,
        1,
        0.5f,
        0.5f,
        -0.5f,
        1,
        1,
        1,
        1,
        1,

        // ----- Top -----
        -0.5f,
        0.5f,
        -0.5f,
        1,
        1,
        1,
        0,
        1,
        0.5f,
        0.5f,
        -0.5f,
        1,
        1,
        1,
        1,
        1,
        0.5f,
        0.5f,
        0.5f,
        1,
        1,
        1,
        1,
        0,
        -0.5f,
        0.5f,
        0.5f,
        1,
        1,
        1,
        0,
        0,

        // ----- Bottom -----
        -0.5f,
        -0.5f,
        -0.5f,
        1,
        1,
        1,
        0,
        0,
        0.5f,
        -0.5f,
        -0.5f,
        1,
        1,
        1,
        1,
        0,
        0.5f,
        -0.5f,
        0.5f,
        1,
        1,
        1,
        1,
        1,
        -0.5f,
        -0.5f,
        0.5f,
        1,
        1,
        1,
        0,
        1,
    };

    std::vector<uint32_t> box_indices = {
        0,  1,  2,  2,  3,  0, // front
        4,  5,  6,  6,  7,  4, // back
        8,  9,  10, 10, 11, 8, // left
        12, 13, 14, 14, 15, 12, // right
        16, 17, 18, 18, 19, 16, // top
        20, 21, 22, 22, 23, 20 // bottom
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


    const int grid_size = 2;
    for (int x = -grid_size / 2; x < grid_size / 2; ++x) {
        for (int y = -grid_size / 2; y < grid_size / 2; ++y) {
            auto obj = scene->CreateObject();
            obj->SetPosition({static_cast<float>(x), static_cast<float>(y), 0.0f});
            auto meshComp = new golias::MeshComponent();
            obj->AddComponent(new golias::MeshComponent(mesh, material));
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
