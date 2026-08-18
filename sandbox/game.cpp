#include "game.h"
#include "core/engine.h"
#include <glfw/glfw3.h>


bool GameApplication::Initialize() {

    return true;
}

void GameApplication::Update(float deltaTime) {


    const auto& inputManager = Engine::GetInstance().GetInputManager();
    
    if(inputManager.IsKeyPressed(GLFW_KEY_ESCAPE)) {
        GOLIAS_LOG_INFO("Escape key pressed. Closing the application.");
    }

}

void GameApplication::Shutdown() {

}
