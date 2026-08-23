#include "game.h"

#include "test_obj.h"


bool GameApplication::Initialize() {

    mScene = new Scene();
    CameraComponent* cameraComponent = new CameraComponent(45.0f, 800.0f / 600.0f, 0.1f, 100.0f);

    GameObject* cameraObject = mScene->CreateGameObject("Main Camera");
    cameraObject->AddComponent(cameraComponent);
    cameraObject->SetPosition(glm::vec3(0.0f, 0.0f, -5.0f));

    cameraObject->AddComponent(new PlayerControllerComponent());

    mScene->SetMainCamera(cameraObject);

    mScene->CreateGameObject<TestObject>("Test Object");

    Engine::GetInstance().SetScene(mScene);

    return true;
}

void GameApplication::Update(float deltaTime) {


    InputManager& inputManager = Engine::GetInstance().GetInputManager();

    if (inputManager.IsKeyPressed(KeyCode::Escape)) {
        GOLIAS_LOG_INFO("Escape key pressed. Closing the application.");
    }

    mScene->Update(deltaTime);
}

void GameApplication::Shutdown() {
   
}
