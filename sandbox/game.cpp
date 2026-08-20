#include "game.h"

#include "test_obj.h"


bool GameApplication::Initialize() {

    mScene.CreateGameObject<TestObject>("Test Object");

    return true;
}

void GameApplication::Update(float deltaTime) {


    InputManager& inputManager = Engine::GetInstance().GetInputManager();

    if (inputManager.IsKeyPressed(KeyCode::Escape)) {
        GOLIAS_LOG_INFO("Escape key pressed. Closing the application.");
    }

    mScene.Update(deltaTime);
}

void GameApplication::Shutdown() {
}
