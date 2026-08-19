#include "game.h"


bool GameApplication::Initialize() {

    std::string vertexShaderSource = R"(
        #version 330 core
        layout(location = 0) in vec3 aPos;
        layout(location = 1) in vec3 aColor;

        out vec3 vColor;

        void main() {
            gl_Position = vec4(aPos, 1.0);
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

    mMaterial.SetShader(shader);

    std::vector<float> vertices = {
        0.5f,  0.5f,  0.0f, 1.0f, 0.0f, 0.0f, // Top Right
        -0.5f, 0.5f,  0.0f, 0.0f, 1.0f, 0.0f, // Top Left
        -0.5f, -0.5f, 0.0f, 0.0f, 0.0f, 1.0f, // Bottom Left
        0.5f,  -0.5f, 0.0f, 1.0f, 1.0f, 0.0f // Bottom Right

    };

    std::vector<uint32_t> indices = {0, 1, 2, 0, 2, 3};


    VertexLayout layout;
    layout.Elements.push_back({0, 3, GL_FLOAT, 0});
    layout.Elements.push_back({1, 3, GL_FLOAT, 3 * sizeof(float)});
    layout.Stride = 6 * sizeof(float);

    mMesh = std::make_shared<Mesh>(layout, vertices, indices);
    return true;
}

void GameApplication::Update(float deltaTime) {


    InputManager& inputManager = Engine::GetInstance().GetInputManager();

    if (inputManager.IsKeyPressed(KeyCode::Escape)) {
        GOLIAS_LOG_INFO("Escape key pressed. Closing the application.");
    }

    RenderCommand command;
    command.Mesh = mMesh.get();
    command.Material = &mMaterial;

    Engine::GetInstance().GetCommandQueue().Submit(command);
}

void GameApplication::Shutdown() {

    
}
