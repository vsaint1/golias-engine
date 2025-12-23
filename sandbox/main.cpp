#include "game/game.h"
#include  <SDL3/SDL_main.h>


int main(int argc, char* argv[]) {
    SandboxApplication* gameApp = new SandboxApplication();

    golias::Engine& engine = golias::Engine::GetInstance();
    engine.SetApplication(gameApp);

    golias::InputManager& inputManager = engine.GetInputManager();

    inputManager.BindAction("Jump", SDLK_SPACE);
    
    if (!engine.Initialize("Golias Engine - Sandbox", 1280, 720)) {
        return -1;
    }

    engine.Run();
    engine.Destroy();

    return 0;
}