#include "test_obj.h"

#include "scene/3d/mesh_component.h"

TestObject::TestObject() {


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

    // std::vector<float> vertices = {// positions        // colors         // texcoords
    //                                -0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.5f, -0.5f, 0.0f, 0.0f,
    //                                1.0f,  0.0f,  1.0f, 0.0f, 0.0f, 0.5f, 0.0f, 0.0f, 0.0f, 1.0f,  0.5f, 1.0f};

    // std::vector<uint32_t> indices = {0, 1, 2};

                                   std::vector<float> quad_vertices = {
        // positions        // colors         // texcoords
        -0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f,
         0.5f, -0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f,
         0.5f,  0.5f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f,
        -0.5f,  0.5f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f
    };

    std::vector<Uint32> quad_indices = {
        0, 1, 2,
        2, 3, 0
    };


    golias::VertexLayout layout;
    layout.elements = {{0, 3, EDataType::FLOAT, false, 0},
                       {1, 3, EDataType::FLOAT, false, 3 * sizeof(float)},
                       {2, 2, EDataType::FLOAT, false, 6 * sizeof(float)}};

    layout.stride = 8 * sizeof(float);

    auto mesh = rd->CreateMeshFromData(layout, quad_vertices, quad_indices);


    AddComponent(new golias::MeshComponent(mesh, material));
}

void TestObject::Update(float deltaTime) {
    golias::GameObject::Update(deltaTime);

    glm::vec3 pos = GetPosition();

    auto& input = golias::Engine::GetInstance().GetInputManager();

    if (input.IsKeyPressed(SDLK_W)) {
        pos.y += 1.0f * deltaTime;
    }

    if (input.IsKeyPressed(SDLK_S)) {
        pos.y -= 1.0f * deltaTime;
    }

    if (input.IsKeyPressed(SDLK_A)) {
        pos.x -= 1.0f * deltaTime;
    }

    if (input.IsKeyPressed(SDLK_D)) {
        pos.x += 1.0f * deltaTime;
    }

    SetPosition(pos);
}
