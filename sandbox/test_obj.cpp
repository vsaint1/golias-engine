#include "test_obj.h"


TestObject::TestObject() {

    std::string vertexShaderSource = R"(
        #version 330 core
        layout(location = 0) in vec3 aPos;
        layout(location = 1) in vec3 aColor;

        uniform mat4 uModel;
        uniform mat4 uView;
        uniform mat4 uProjection;

        out vec3 vColor;

        void main() {
            gl_Position = uProjection * uView * uModel * vec4(aPos, 1.0);
            vColor = aColor;
        }
    )";

    std::string fragmentShaderSource = R"(
        #version 330 core
        out vec4 FragColor;

        in vec3 vColor;

        void main() {
            FragColor = vec4(vColor, 1.0);
        }
    )";

    auto shader = Engine::GetInstance().GetGraphicsDevice().CreateShader(vertexShaderSource, fragmentShaderSource);

    auto material = std::make_shared<Material>();
    material->SetShader(shader);

    std::vector<float> vertices = {
        0.5f,  0.5f,  0.5f,  1.0f, 0.0f, 0.0f, // Top Right
        -0.5f, 0.5f,  0.5f,  0.0f, 1.0f, 0.0f, // Top Left
        -0.5f, -0.5f, 0.5f,  0.0f, 0.0f, 1.0f, // Bottom Left
        0.5f,  -0.5f, 0.5f,  1.0f, 1.0f, 0.0f, // Bottom Right

        0.5f,  0.5f,  -0.5f, 1.0f, 0.0f, 0.0f, // Top Right
        -0.5f, 0.5f,  -0.5f, 0.0f, 1.0f, 0.0f, // Top Left
        -0.5f, -0.5f, -0.5f, 0.0f, 0.0f, 1.0f, // Bottom Left
        0.5f,  -0.5f, -0.5f, 1.0f, 1.0f, 0.0f // Bottom Right

    };

    std::vector<uint32_t> indices = {
        0, 1, 2, 0, 2, 3, // Front face
        4, 5, 1, 4, 1, 0, // top face
        4, 0, 3, 4, 3, 7, // right face
        3, 2, 6, 3, 6, 7, // bottom face
        4, 5, 6, 4, 6, 7 // Back face
    };


    VertexLayout layout;
    layout.Elements.push_back({0, 3, GL_FLOAT, 0});
    layout.Elements.push_back({1, 3, GL_FLOAT, 3 * sizeof(float)});
    layout.Stride = 6 * sizeof(float);

    auto mesh = std::make_shared<Mesh>(layout, vertices, indices);

    StaticMeshComponent* meshComponent = new StaticMeshComponent(mesh, material);


    AddComponent(meshComponent);
}


void TestObject::Update(float deltaTime) {
    GameObject::Update(deltaTime);

    RotateLocal(glm::vec3(0.0f, 1.0f, 0.5f), 0.01f);
}
