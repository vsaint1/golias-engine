#include "test_obj.h"



TestObject::TestObject() {





    FileSystem& fileSystem = Engine::GetInstance().GetFileSystem();

    Ref<Material> material = Material::Load("materials/brick.gmat");

    std::vector<float> vertices = {
        // Front (+Z)
        0.5f,  0.5f,  0.5f,  1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
        -0.5f, 0.5f,  0.5f,  1.0f, 1.0f, 1.0f, 0.0f, 1.0f,
        -0.5f, -0.5f, 0.5f,  1.0f, 1.0f, 1.0f, 0.0f, 0.0f,
        0.5f,  -0.5f, 0.5f,  1.0f, 1.0f, 1.0f, 1.0f, 0.0f,

        // Back (-Z)
        -0.5f, 0.5f,  -0.5f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f,
        0.5f,  0.5f,  -0.5f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
        0.5f,  -0.5f, -0.5f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f,
        -0.5f, -0.5f, -0.5f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f,

        // Top (+Y)
        -0.5f, 0.5f,  0.5f,  1.0f, 1.0f, 1.0f, 0.0f, 0.0f,
        0.5f,  0.5f,  0.5f,  1.0f, 1.0f, 1.0f, 1.0f, 0.0f,
        0.5f,  0.5f,  -0.5f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
        -0.5f, 0.5f,  -0.5f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f,

        // Bottom (-Y)
        -0.5f, -0.5f, -0.5f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f,
        0.5f,  -0.5f, -0.5f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
        0.5f,  -0.5f, 0.5f,  1.0f, 1.0f, 1.0f, 1.0f, 0.0f,
        -0.5f, -0.5f, 0.5f,  1.0f, 1.0f, 1.0f, 0.0f, 0.0f,

        // Right (+X)
        0.5f,  0.5f,  -0.5f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f,
        0.5f,  0.5f,  0.5f,  1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
        0.5f,  -0.5f, 0.5f,  1.0f, 1.0f, 1.0f, 1.0f, 0.0f,
        0.5f,  -0.5f, -0.5f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f,

        // Left (-X)
        -0.5f, 0.5f,  0.5f,  1.0f, 0.0f, 1.0f, 0.0f, 1.0f,
        -0.5f, 0.5f,  -0.5f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
        -0.5f, -0.5f, -0.5f, 0.0f, 1.0f, 1.0f, 1.0f, 0.0f,
        -0.5f, -0.5f, 0.5f,  1.0f, 1.0f, 0.0f, 0.0f, 0.0f,
    };

    std::vector<uint32_t> indices = {
        0, 1, 2, 0, 2, 3,       // Front
        4, 5, 6, 4, 6, 7,       // Back
        8, 9, 10, 8, 10, 11,    // Top
        12, 13, 14, 12, 14, 15, // Bottom
        16, 17, 18, 16, 18, 19, // Right
        20, 21, 22, 20, 22, 23  // Left
    };


    VertexLayout layout;
    layout.Elements.push_back({0, 3, GL_FLOAT, 0});
    layout.Elements.push_back({1, 3, GL_FLOAT, 3 * sizeof(float)});
    layout.Elements.push_back({2, 2, GL_FLOAT, 6 * sizeof(float)});
    layout.Stride = 8 * sizeof(float);

    auto mesh = std::make_shared<Mesh>(layout, vertices, indices);

    StaticMeshComponent* meshComponent = new StaticMeshComponent(mesh, material);


    AddComponent(meshComponent);
}


void TestObject::Update(float deltaTime) {
    GameObject::Update(deltaTime);

    RotateLocal(glm::vec3(0.0f, 1.0f, 0.5f), 0.01f);
}
